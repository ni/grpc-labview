//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_interop.h>
#include <grpc_server.h>
#include <lv_message.h>
#include <cluster_copier.h>

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

        auto doneTag = new CallDoneNotificationTag(callData, server);
        callData->_callDoneNotificationTag = doneTag;
        callData->_ctx.AsyncNotifyWhenDone(doneTag);

        // Start the state machine which waits for a new call to arrive.
        auto tag = CompletionQueueTag::ForRequestCall(callData);
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
        _callState(CallState::WaitingForConnection),
        _callDoneNotificationTag(nullptr),
        _cancelled(false),
        _finishedFromLabVIEW(false),
        _callStatus(grpc::Status::OK)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::SetCallStatusError(std::string errorMessage)
    {
        SetCallStatusError(grpc::StatusCode::INTERNAL, errorMessage);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::SetCallStatusError(grpc::StatusCode statusCode, std::string errorMessage)
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _callStatus = grpc::Status(statusCode, errorMessage);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    grpc::StatusCode CallData::GetCallStatusCode()
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        return _callStatus.error_code();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool CallData::IsCancelled()
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        return _cancelled;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool CallData::ReadNext(int8_t* cluster)
    {
        // GenericServerAsyncReaderWriter only supports one read at a time. Also,
        // trying to read concurrently from the same request message would produce
        // undefined results.
        std::lock_guard<std::mutex> readLock(_readMutex);
        std::unique_ptr<ReadNextTag> tag(new ReadNextTag(shared_from_this()));
        {
            std::lock_guard<std::mutex> stateLock(_stateMutex);
            if (_callState != CallState::Connected)
            {
                return false;
            }
            _stream.Read(&_rb, tag.get());
        }

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
        std::lock_guard<std::mutex> writeLock(_outboundOperationMutex);
        std::unique_ptr<WriteNextTag> tag;
        std::unique_ptr<grpc::ByteBuffer> wb;
        {
            std::lock_guard<std::mutex> stateLock(_stateMutex);
            if (_callState != CallState::Connected)
            {
                return false;
            }

            grpc_labview::ClusterDataCopier::CopyFromCluster(*_response, cluster);
            wb = _response->SerializeToByteBuffer();
            tag = std::make_unique<WriteNextTag>(shared_from_this());
            _stream.Write(*wb, tag.get());
        }

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

        if (!ok)
        {
            if (_callState == CallState::WaitingForConnection)
            {
                // AsyncNotifyWhenDone is not delivered when a pending RequestCall never starts. Keep its
                // weak notification tag alive until the completion queue has been fully drained.
                auto server = _server.lock();
                if (server != nullptr)
                {
                    server->RetainUndeliveredCallDoneNotificationTag(_callDoneNotificationTag);
                }
                else
                {
                    delete _callDoneNotificationTag;
                }
                _callDoneNotificationTag = nullptr;
            }
            _callState = CallState::Finished;
            return;
        }

        if (_callDoneNotificationTag != nullptr)
        {
            _callDoneNotificationTag->SetCallAccepted();
        }

        auto server = _server.lock();
        if (server == nullptr)
        {
            FinishWithStatusLocked(grpc::Status(grpc::StatusCode::UNAVAILABLE, "Server is no longer available"));
            return;
        }

        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The new call object is kept alive and
        // referenced by the completion queue until a connection from a new
        // client is established for the call.
        CallData::Create(server, _service, _cq);

        if (_callState == CallState::Finished)
        {
            return;
        }

        auto& name = _ctx.method();
        LVEventData eventData;
        if (server->FindEventData(name, eventData))
        {
            auto requestMetadata = server->FindMetadata(eventData.requestMetadataName);
            auto responseMetadata = server->FindMetadata(eventData.responseMetadataName);
            _request = std::make_shared<LVMessage>(requestMetadata);
            _response = std::make_shared<LVMessage>(responseMetadata);

            grpc_labview::gPointerManager.RegisterPointer(shared_from_this());
            grpc_labview::RegisterCleanupProc(grpc_labview::CloseServerEventCleanupProc, this);
            _callState = CallState::Connected;
            if (server->SendEvent(name, static_cast<gRPCid*>(this)) != 0)
            {
                grpc_labview::DeregisterCleanupProc(grpc_labview::CloseServerEventCleanupProc, this);
                grpc_labview::gPointerManager.UnregisterPointer(this);
                FinishWithStatusLocked(grpc::Status(grpc::StatusCode::INTERNAL, "Failed to send LV event for RPC"));
            }
        }
        else
        {
            FinishWithStatusLocked(grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "RPC is not implemented or no metadata was registered"));
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::FinishWithStatusLocked(grpc::Status status)
    {
        _callState = CallState::Finishing;
        _stream.Finish(status, CompletionQueueTag::ForFinish(shared_from_this()));
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::FinishFromLabVIEW()
    {
        std::lock_guard<std::mutex> writeLock(_outboundOperationMutex);
        std::lock_guard<std::mutex> stateLock(_stateMutex);

        if (_finishedFromLabVIEW)
        {
            return;
        }

        _finishedFromLabVIEW = true;
        grpc_labview::DeregisterCleanupProc(grpc_labview::CloseServerEventCleanupProc, this);
        grpc_labview::gPointerManager.UnregisterPointer(this);

        // Helps guard against LV code completing a grpc call after shutdown
        // has been called on the server or LV completing the call multiple times.
        if (_callState == CallState::Finishing || _callState == CallState::Finished)
        {
            return;
        }

        FinishWithStatusLocked(_callStatus);
        return;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::FinishOperationCompleted(bool ok)
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _callState = CallState::Finished;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::OnCallDone()
    {
        auto cancelled = _ctx.IsCancelled();
        std::lock_guard<std::mutex> lock(_stateMutex);

        _callDoneNotificationTag = nullptr;
        _cancelled = cancelled;

        // The call was completed normally from LV code.
        if (_callState == CallState::Finishing || _callState == CallState::Finished)
        {
            return;
        }

        // If OnCallDone is called and we are not already finishing because the user completed the call
        // from LV, then it means either the server is shutting down or the call was cancelled. In either
        // case there is no point in calling Finish on the stream so just mark the call as finished.
        _callState = CallState::Finished;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    CallDoneNotificationTag::CallDoneNotificationTag(std::shared_ptr<CallData> callData, std::shared_ptr<LabVIEWgRPCServer> server) :
        _callData(callData),
        _server(server)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallDoneNotificationTag::SetCallAccepted()
    {
        _acceptedCallData = _callData.lock();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallDoneNotificationTag::Proceed(bool ok)
    {
        auto server = _server.lock();
        if (server != nullptr)
        {
            server->ReleaseUndeliveredCallDoneNotificationTag(this);
        }

        auto callData = _acceptedCallData;
        if (callData == nullptr)
        {
            callData = _callData.lock();
        }
        if (callData != nullptr)
        {
            callData->OnCallDone();
        }
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