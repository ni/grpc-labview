//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <future>
#include <grpcpp/impl/server_initializer.h>
#include "lv_proto_server_reflection_plugin.h"

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerInitializer;

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LabVIEWgRPCServer::LabVIEWgRPCServer() :
        _shutdown(false),
        _genericMethodEvent(0)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LabVIEWgRPCServer::RegisterEvent(std::string name, LVUserEventRef item, std::string requestMetadata, std::string responseMetadata)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        LVEventData data = { item, requestMetadata, responseMetadata };
        _registeredServerMethods.insert(std::pair<std::string, LVEventData>(name, data));
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LabVIEWgRPCServer::RegisterGenericMethodEvent(LVUserEventRef item)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _genericMethodEvent = item;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LabVIEWgRPCServer::SendEvent(std::string name, gRPCid* data)
    {
        if (HasGenericMethodEvent())
        {
            OccurServerEvent(_genericMethodEvent, data, name);
        }
        else
        {
            auto eventData = _registeredServerMethods.find(name);
            if (eventData != _registeredServerMethods.end())
            {
                OccurServerEvent(eventData->second.event, data);
            }
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LabVIEWgRPCServer::FindEventData(std::string name, LVEventData& data)
    {
        auto eventData = _registeredServerMethods.find(name);
        if (eventData != _registeredServerMethods.end())
        {
            data = eventData->second;
            return true;
        }
        return false;
    }

    bool LabVIEWgRPCServer::HasRegisteredServerMethod(std::string methodName)
    {
        return _registeredServerMethods.find(methodName) != _registeredServerMethods.end();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LabVIEWgRPCServer::HasGenericMethodEvent()
    {
        return _genericMethodEvent != 0;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int LabVIEWgRPCServer::ListeningPort()
    {
        return _listeningPort;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int LabVIEWgRPCServer::Run(std::string address, std::string serverCertificatePath, std::string serverKeyPath)
    {
        FinalizeMetadata();

        auto serverStarted = new ServerStartEventData;
        _runThread = std::make_unique<std::thread>(StaticRunServer, this, address, serverCertificatePath, serverKeyPath, serverStarted);
        serverStarted->WaitForComplete();
        auto result = serverStarted->serverStartStatus;
        delete serverStarted;
        if (result == -1)
        {
            // If we weren't able to start the gRPC server then the _runThread has nothing to do.
            // So do an immediate join on the thread.
            if (_runThread->joinable())
            {
                _runThread->join();
            }
        }
        return result;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::string read_keycert(const std::string &filename)
    {
        std::string data;
        std::ifstream file(filename.c_str(), std::ios::in);
        if (file.is_open())
        {
            std::stringstream ss;
            ss << file.rdbuf();
            file.close();
            data = ss.str();
        }
        return data;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LabVIEWgRPCServer::HandleRpcs(grpc::ServerCompletionQueue *cq)
    {
        // Spawn a new CallData instance to serve new clients.
        new CallData(this, _rpcService.get(), cq);
        void *tag; // uniquely identifies a request.
        bool ok;
        while (true)
        {
            if (_shutdown)
            {
                break;
            }

            // Block waiting to read the next event from the completion queue. The
            // event is uniquely identified by its tag, which in this case is the
            // memory address of a CallData instance.
            cq->Next(&tag, &ok);
            static_cast<CallDataBase*>(tag)->Proceed(ok);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LabVIEWgRPCServer::StaticRunServer(
        LabVIEWgRPCServer* server,
        std::string address,
        std::string serverCertificatePath,
        std::string serverKeyPath,
        ServerStartEventData *serverStarted)
    {
        server->RunServer(address, serverCertificatePath, serverKeyPath, serverStarted);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LabVIEWgRPCServer::RunServer(
        std::string address,
        std::string serverCertificatePath,
        std::string serverKeyPath,
        ServerStartEventData *serverStarted)
    {
        std::string server_address;
        if (address.length() != 0)
        {
            server_address = address;
        }
        if (server_address.length() == 0)
        {
            server_address = "0.0.0.0:50051";
        }

        grpc::EnableDefaultHealthCheckService(true);
        InitLVProtoReflectionServerBuilderPlugin();
        // grpc::reflection::InitProtoReflectionServerBuilderPlugin();
        ServerBuilder builder;

        std::shared_ptr<grpc::ServerCredentials> creds;
        if (serverCertificatePath.length() > 1)
        {
            std::string servercert = read_keycert(serverCertificatePath);
            std::string serverkey = read_keycert(serverKeyPath);

            grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp;
            pkcp.private_key = serverkey;
            pkcp.cert_chain = servercert;

            grpc::SslServerCredentialsOptions ssl_opts;
            ssl_opts.pem_root_certs = "";
            ssl_opts.pem_key_cert_pairs.push_back(pkcp);

            creds = grpc::SslServerCredentials(ssl_opts);
        }
        else
        {
            creds = grpc::InsecureServerCredentials();
        }
        builder.AddListeningPort(server_address, creds, &_listeningPort);
        builder.SetMaxSendMessageSize(-1);
        builder.SetMaxReceiveMessageSize(-1);

        _rpcService = std::unique_ptr<grpc::AsyncGenericService>(new grpc::AsyncGenericService());
        builder.RegisterAsyncGenericService(_rpcService.get());
        _cq = builder.AddCompletionQueue();

        _server = builder.BuildAndStart();
        if (_server != nullptr)
        {
            std::cout << "Server listening on " << server_address << std::endl;
            serverStarted->NotifyComplete();

            HandleRpcs(_cq.get());
            _server->Wait();
        }
        else
        {
            serverStarted->serverStartStatus = -1;
            serverStarted->NotifyComplete();
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LabVIEWgRPCServer::StopServer()
    {
        _shutdown = true;
        if (_server != nullptr)
        {
            // We need shutdown passing a deadline so that any RPC calls in progress are terminated as well.
            _server->Shutdown(std::chrono::system_clock::now());
            _server->Wait();

            // Always shutdown the completion queue after the server.
            if (_cq != nullptr)
            {
                _cq->Shutdown();
            }

            if (_runThread->joinable())
            {
                _runThread->join();
            }

            // Drain the complete queue before deleting the server.
            // Otherwise, server might fail on the assertion that the completion queue must be empty.
            if (_cq != nullptr)
            {
                void *tag;
                bool ok;
                while (_cq->Next(&tag, &ok)) {}
            }

            _server = nullptr;
        }
        grpc_labview::ProtoDescriptorString::getInstance()->deleteInstance();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

    // Initialize the static members
    ProtoDescriptorString* ProtoDescriptorString::m_instance = nullptr;
    std::mutex ProtoDescriptorString::m_mutex;

    // Return the static class instance. Thread safe.
    ProtoDescriptorString* ProtoDescriptorString::getInstance() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new ProtoDescriptorString();
        }
        return m_instance;
    }

    // Get the descriptor string
    std::string ProtoDescriptorString::getDescriptor() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_descriptor;
    }

    // Set the descriptor string
    void ProtoDescriptorString::setDescriptor(std::string str) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_refcount++;
        m_descriptor = str;
    }

    // Delete the instaance based on the refcount
    void ProtoDescriptorString::deleteInstance() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!--m_refcount) {
            delete m_instance;
            m_instance = nullptr;
        }
    }
}
