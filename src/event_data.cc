//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::internal;

namespace grpc_labview
{  
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    CallData::CallData(LabVIEWgRPCServer* server, grpc::AsyncGenericService *service, grpc::ServerCompletionQueue *cq) :
        _server(server), 
        _service(service),
        _cq(cq),
        _stream(&_ctx),
        _status(CallStatus::Create),
        _writeSemaphore(0),
        _cancelled(false),
        _requestDataReady(false),
        _callStatus(grpc::Status::OK)
    {
        Proceed(true);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::shared_ptr<MessageMetadata> CallData::FindMetadata(const std::string& name)
    {
        return _server->FindMetadata(name);    
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    CallFinishedData::CallFinishedData(CallData* callData)
    {
        _call = callData;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallFinishedData::Proceed(bool ok)
    {
        _call->CallFinished();
        delete this;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool CallData::Write()
    {
        if (IsCancelled())
        {
            return false;
        }
        auto wb = _response->SerializeToByteBuffer();
        grpc::WriteOptions options;
        _status = CallStatus::Writing;
        _stream.Write(*wb, this);
        _writeSemaphore.wait();
        if (IsCancelled())
        {
            return false;
        }
        return true;
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
    void CallData::CallFinished()
    {
        _cancelled = _ctx.IsCancelled();      
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::Finish()
    {
        if (_status == CallStatus::PendingFinish)
        {
            _status = CallStatus::Finish;
            Proceed(false);
        }
        else
        {
            _status = CallStatus::Finish;
            _stream.Finish(_callStatus, this);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool CallData::IsCancelled()
    {
        return _cancelled;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool CallData::ReadNext()
    {
        if (_requestDataReady)
        {
            return true;
        }
        if (IsCancelled())
        {
            return false;
        }
        auto tag = new ReadNextTag(this);
        _stream.Read(&_rb, tag);
        if (!tag->Wait())
        {
            return false;
        }
        _request->ParseFromByteBuffer(_rb);
        _requestDataReady = true;
        if (IsCancelled())
        {
            return false;
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::ReadComplete()
    {
        _requestDataReady = false;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CallData::Proceed(bool ok)
    {
        if (!ok)
        {
            if (_status != CallStatus::Finish)
            {
                _status = CallStatus::PendingFinish;
            }
        }
        if (_status == CallStatus::Create)
        {
            // As part of the initial CREATE state, we *request* that the system
            // start processing SayHello requests. In this request, "this" acts are
            // the tag uniquely identifying the request (so that different CallData
            // instances can serve different requests concurrently), in this case
            // the memory address of this CallData instance.
            _service->RequestCall(&_ctx, &_stream, _cq, _cq, this);
            _ctx.AsyncNotifyWhenDone(new CallFinishedData(this));
            _status = CallStatus::Read;
        }
        else if (_status == CallStatus::Read)
        {
            // Spawn a new CallData instance to serve new clients while we process
            // the one for this CallData. The instance will deallocate itself as
            // part of its FINISH state.
            new CallData(_server, _service, _cq);

            _stream.Read(&_rb, this);
            _status = CallStatus::Process;
        }
        else if (_status == CallStatus::Process)
        {
            auto name = _ctx.method();

            LVEventData eventData;
            if (_server->FindEventData(name, eventData) || _server->HasGenericMethodEvent())
            {
                auto requestMetadata = _server->FindMetadata(eventData.requestMetadataName);
                auto responseMetadata = _server->FindMetadata(eventData.responseMetadataName);
                _request = std::make_shared<LVMessage>(requestMetadata);
                _response = std::make_shared<LVMessage>(responseMetadata);
                _request->ParseFromByteBuffer(_rb);
                _requestDataReady = true;

                _methodData = std::make_shared<GenericMethodData>(this, &_ctx, _request, _response);
                gPointerManager.RegisterPointer(_methodData);
                _server->SendEvent(name, static_cast<gRPCid*>(_methodData.get()));
            }
            else
            {
                _stream.Finish(grpc::Status::CANCELLED, this);
            }       
        }
        else if (_status == CallStatus::Writing)
        {
            _writeSemaphore.notify();
        }
        else if (_status == CallStatus::PendingFinish)
        {        
        }
        else
        {
            assert(_status == CallStatus::Finish);
            delete this;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    ReadNextTag::ReadNextTag(CallData* callData) :
        _readCompleteSemaphore(0),
        _success(false)
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
    GenericMethodData::GenericMethodData(CallData* call, ServerContext *context, std::shared_ptr<LVMessage> request, std::shared_ptr<LVMessage> response)
        : EventData(context)
    {
        _call = call;
        _request = request;
        _response = response;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::shared_ptr<MessageMetadata> GenericMethodData::FindMetadata(const std::string& name)
    {
        if (_call != nullptr)
        {
            return _call->FindMetadata(name);
        }
        return nullptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    ServerStartEventData::ServerStartEventData()
        : EventData(nullptr)
    {
        serverStartStatus = 0;
    }
}