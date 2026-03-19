//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_interop.h>
#include <grpc_server.h>
#include <lv_message.h>
#include <cluster_copier.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::internal;

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::shared_ptr<CallData> CallData::Create(
        std::shared_ptr<LabVIEWgRPCServer> server,
        grpc::AsyncGenericService* service,
        grpc::ServerCompletionQueue* cq)
    {
        auto callData = std::shared_ptr<CallData>(new CallData(server, service, cq));

        auto finishedTag = new CallFinishedTag(callData);
        callData->_callFinishedTag = finishedTag;
        callData->_ctx.AsyncNotifyWhenDone(finishedTag);

        // Start the state machine which waits for a new call to arrive.
        auto tag = new CompletionQueueTag(callData);
        callData->_service->RequestCall(
            &callData->_ctx,
            &callData->_stream,
            callData->_cq,
            callData->_cq,
            tag);
        return callData;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    CallData::CallData(std::shared_ptr<LabVIEWgRPCServer> server, grpc::AsyncGenericService *service, grpc::ServerCompletionQueue *cq) :
        _server(server),
        _service(service),
        _cq(cq),
        _stream(&_ctx),
        _status(CallStatus::WaitingForConnection),
        _callStatus(grpc::Status::OK)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::SetCallStatusError(std::string errorMessage)
    {
        _callStatus = grpc::Status(grpc::StatusCode::INTERNAL, errorMessage);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::SetCallStatusError(grpc::StatusCode statusCode, std::string errorMessage)
    {
        _callStatus = grpc::Status(statusCode, errorMessage);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    grpc::StatusCode CallData::GetCallStatusCode()
    {
        return _callStatus.error_code();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool CallData::IsCancelled()
    {
        return _ctx.IsCancelled();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool CallData::IsActive()
    {
        return _status != CallStatus::Finished && _status != CallStatus::Finishing && !IsCancelled();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool CallData::ReadNext(int8_t* cluster)
    {
        // GenericServerAsyncReaderWriter only supports one read at a time. Also,
        // trying to read concurrently from the same request message would produce
        // undefined results.
        std::lock_guard<std::mutex> lock(_readMutex);
        if (!IsActive())
        {
            return false;
        }

        std::unique_ptr<ReadNextTag> tag(new ReadNextTag(shared_from_this()));
        _stream.Read(&_rb, tag.get());
        if (!tag->Wait())
        {
            return false;
        }
        _request->ParseFromByteBuffer(_rb);
        grpc_labview::ClusterDataCopier::CopyToCluster(*_request, cluster);
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool CallData::Write(int8_t* cluster)
    {
        // GenericServerAsyncReaderWriter only supports one write at a time. Also,
        // trying to write concurrently to the same response message would produce
        // undefined results.
        std::lock_guard<std::mutex> lock(_writeMutex);
        if (!IsActive())
        {
            return false;
        }

        grpc_labview::ClusterDataCopier::CopyFromCluster(*_response, cluster);
        auto wb = _response->SerializeToByteBuffer();
        std::unique_ptr<WriteNextTag> tag(new WriteNextTag(shared_from_this()));
        _stream.Write(*wb, tag.get());
        if (!tag->Wait())
        {
            return false;
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::Proceed(bool ok)
    {
        std::lock_guard<std::mutex> lock(_stateMutex);

        if (!ok && _status != CallStatus::Finished)
        {
            if (_status == CallStatus::WaitingForConnection)
            {
                // Ugh. When using the grpc async APIs, you are required to call AsyncNotifyWhenDone if you want to call IsCancelled
                // on the ServerContext. However, the tag registered with AsyncNotifyWhenDone is only notified if a RPC call actually
                // starts, and you must call AsyncNotifyWhenDone before the call starts or tag will not be notified either. Generally,
                // it is acceptable to just leak this one tag on server shutdown. However, because we maintain a shared pointer to the
                // server, we will end up leaking everything if we don't clean up this tag. As a work around, we delete the tag here.
                // This leaves a dangling tag pointer in the completion queue, but it never does anything with the tag. It only delivers
                // the tag from the Next call which we know will never be triggered so this should be safe.
                delete _callFinishedTag;
                _callFinishedTag = nullptr;
            }
            _status = CallStatus::Finishing;
        }

        if (_status == CallStatus::WaitingForConnection)
        {
            // Spawn a new CallData instance to serve new clients while we process
            // the one for this CallData. The new call object is kept alive and
            // referenced by the completion queue until a connection from a new
            // client is established for the call.
            CallData::Create(_server, _service, _cq);

            auto& name = _ctx.method();
            LVEventData eventData;
            if ((_server->HasRegisteredServerMethod(name) && _server->FindEventData(name, eventData)) || _server->HasGenericMethodEvent())
            {
                auto requestMetadata = _server->FindMetadata(eventData.requestMetadataName);
                auto responseMetadata = _server->FindMetadata(eventData.responseMetadataName);
                _request = std::make_shared<LVMessage>(requestMetadata);
                _response = std::make_shared<LVMessage>(responseMetadata);

                gPointerManager.RegisterPointer(shared_from_this());
                grpc_labview::RegisterCleanupProc(grpc_labview::CloseServerEventCleanupProc, this);
                _server->SendEvent(name, static_cast<gRPCid*>(this));
                _status = CallStatus::Connected;
            }
            else
            {
                _status = CallStatus::Finishing;
                _stream.Finish(grpc::Status(grpc::StatusCode::UNIMPLEMENTED, ""), new CompletionQueueTag(shared_from_this()));
            }
        }
        else if (_status == CallStatus::Connected)
        {
            // Once connected, we expect reads of request messages to be triggered by calls to ReadNext via API calls from
            // the user's implementation of the RPC in the server. We do not expect any events to be received for this RPC
            // in this state.
            assert(false);
        }
        else if (_status == CallStatus::Finishing)
        {
            _status = CallStatus::Finished;
        }
        else
        {
            assert(_status == CallStatus::Finished);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::FinishFromLabVIEW()
    {
        std::lock_guard<std::mutex> lock(_stateMutex);

        // Helps guard against LV code completing a grpc call after shutdown
        // has been called on the server or LV completing the call multiple times.
        if (_status == CallStatus::Finishing || _status == CallStatus::Finished)
        {
            return;
        }

        _status = CallStatus::Finishing;
        _stream.Finish(_callStatus, new CompletionQueueTag(shared_from_this()));
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::FinishFromCompletionQueue()
    {
        std::lock_guard<std::mutex> lock(_stateMutex);

        _callFinishedTag = nullptr;

        // The call was completed normally from LV code.
        if (_status == CallStatus::Finishing || _status == CallStatus::Finished)
        {
            return;
        }

        // If FinishFromCompletionQueue is called and we are not already finishing because the user completed
        // the call from LV, then it means either the server is shutting down or the call was cancelled. In either
        // case there is no point in calling Finish on the stream so just mark the call as finished.
        _status = CallStatus::Finished;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    CallFinishedTag::CallFinishedTag(std::shared_ptr<CallData> callData)
    {
        _callData = callData;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallFinishedTag::Proceed(bool ok)
    {
        _callData->FinishFromCompletionQueue();
        delete this;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    ReadNextTag::ReadNextTag(std::shared_ptr<CallData> callData) :
        _readCompleteSemaphore(0),
        _success(false),
        _callData(callData)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ReadNextTag::Proceed(bool ok)
    {
        _success = ok;
        _readCompleteSemaphore.notify();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool ReadNextTag::Wait()
    {
        _readCompleteSemaphore.wait();
        return _success;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    WriteNextTag::WriteNextTag(std::shared_ptr<CallData> callData) :
        _writeCompleteSemaphore(0),
        _success(false),
        _callData(callData)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void WriteNextTag::Proceed(bool ok)
    {
        _success = ok;
        _writeCompleteSemaphore.notify();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool WriteNextTag::Wait()
    {
        _writeCompleteSemaphore.wait();
        return _success;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    EventData::EventData(ServerContext *_context) :
        _completed(false)
    {
        context = _context;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void EventData::WaitForComplete()
    {
        std::unique_lock<std::mutex> lck(lockMutex);
        while (!_completed) lock.wait(lck);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void EventData::NotifyComplete()
    {
        std::unique_lock<std::mutex> lck(lockMutex);
        _completed = true;
        lock.notify_all();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    ServerStartEventData::ServerStartEventData()
        : EventData(nullptr)
    {
        serverStartStatus = 0;
    }
}