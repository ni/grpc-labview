//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
//#include <src/proto/grpc/reflection/v1alpha/reflection.grpc.pb.h>
#include <src/proto/grpc/reflection/v1alpha/reflection.grpc.pb.h> // TODO: Fix this path
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <future>
#include <grpcpp/impl/server_initializer.h>
#include <proto_parser.cc>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerInitializer;
using grpc::reflection::v1alpha::ServerReflectionRequest;
using grpc::reflection::v1alpha::ServerReflectionResponse;

namespace grpc_labview
{

    class LVProtoServerReflectionService final : public grpc::reflection::v1alpha::ServerReflection::Service // TODO: Check if we need to derive this from reflection::v1alpha::ServerReflection::Service (::grpc::Service)
    {
    public:
        LVProtoServerReflectionService(): descriptor_pool_(grpc::protobuf::DescriptorPool::generated_pool()), services_(new std::vector<std::string>()) {}

        // Add the full names of registered services
        void SetServiceList(const std::vector<std::string>* snames) {
            for (int i = 0; i < snames->size(); ++i)
                services_->push_back(snames->at(i));
        }

        void AddService(const std::string serviceName) {
            services_->push_back(serviceName);
        }

        void FindFile(const std::string pathName) {
            if (descriptor_pool_ != nullptr)
                descriptor_pool_->FindFileByName(pathName);
        }

        // implementation of ServerReflectionInfo(stream ServerReflectionRequest) rpc
        // in ServerReflection service
        Status ServerReflectionInfo(
            ServerContext* context,
            grpc::ServerReaderWriter<grpc::reflection::v1alpha::ServerReflectionResponse,
            grpc::reflection::v1alpha::ServerReflectionRequest>* stream)
            override {
            
            ServerReflectionRequest request;
            ServerReflectionResponse response;
            Status status;
            while (stream->Read(&request)) {
                switch (request.message_request_case()) {
                case ServerReflectionRequest::MessageRequestCase::kFileByFilename:
                    status = GetFileByName(context, request.file_by_filename(), &response);
                    break;
                case ServerReflectionRequest::MessageRequestCase::kFileContainingSymbol:
                    status = GetFileContainingSymbol(
                        context, request.file_containing_symbol(), &response);
                    break;
                case ServerReflectionRequest::MessageRequestCase::kFileContainingExtension:
                    status = GetFileContainingExtension(
                        context, &request.file_containing_extension(), &response);
                    break;
                    case ServerReflectionRequest::MessageRequestCase::
                    kAllExtensionNumbersOfType:
                        status = GetAllExtensionNumbers(
                            context, request.all_extension_numbers_of_type(),
                            response.mutable_all_extension_numbers_response());
                        break;
                case ServerReflectionRequest::MessageRequestCase::kListServices:
                        status = ListService(context, response.mutable_list_services_response());
                        break;
                    default:
                        status = Status(grpc::StatusCode::UNIMPLEMENTED, "");
                }

                if (!status.ok()) {
                    FillErrorResponse(status, response.mutable_error_response());
                }
                response.set_valid_host(request.host());
                response.set_allocated_original_request(new ServerReflectionRequest(request));
                stream->Write(response);
            }

            return Status::OK;

        } // TODO

        void ImportFile(std::string path, std::string searchPath) {            
            // protoParser.Import(path, searchPath);
            /*auto descriptor = protoParser.m_FileDescriptor;
            std::unique_ptr<const FileDescriptor*> fileDescriptor(descriptor->pool()->);
            if (descriptor != nullptr)
                custom_descriptor_poolMap[path] = ;
            AddService(path);*/
        }

        void AddFileDescriptorProto(const std::string& serializedProtoStr) { 
            // reflection_service_.get()->AddFileDescriptorProto(serializedProto); 
            FileDescriptorProto proto;
            proto.ParseFromString(serializedProtoStr);
            other_pool.BuildFile(proto);
        }


    private:
        Status ListService(ServerContext* context,
            grpc::reflection::v1alpha::ListServiceResponse* response){ 
            
            if (services_ == nullptr) {
                return Status(grpc::StatusCode::NOT_FOUND, "Services not found.");
            }
            for (const auto& value : *services_) {
                grpc::reflection::v1alpha::ServiceResponse* service_response = response->add_service();
                service_response->set_name(value);
            }

            //auto serviceCount = protoParser.m_FileDescriptor->service_count();
            //for (int i = 0; i < serviceCount; ++i) {
            //    auto serviceDescriptor = protoParser.m_FileDescriptor->service(i);
            //    auto service_response = response->add_service();
            //    service_response->set_name(serviceDescriptor->full_name());
            //}

            return Status::OK; 
        }

        Status GetFileByName(ServerContext* context, const std::string& file_name,
            grpc::reflection::v1alpha::ServerReflectionResponse* response){ 
            
            if (descriptor_pool_ == nullptr) {
                return Status::CANCELLED;
            }

            const grpc::protobuf::FileDescriptor* file_desc =
                descriptor_pool_->FindFileByName(file_name);
            if (file_desc == nullptr) {
                // check in other pools
                /*for (auto it = custom_descriptor_poolMap.begin(); it != custom_descriptor_poolMap.end(); ++it) {
                    const grpc::protobuf::FileDescriptor* descriptor = it->second;
                    file_desc = descriptor->pool()->FindFileByName(file_name);
                    if (file_desc != nullptr)
                        break;
                }*/
                file_desc = other_pool.FindFileByName(file_name);
            }

            if (file_desc == nullptr) // we couldn't find it anywhere
                return Status(grpc::StatusCode::NOT_FOUND, "File not found.");

            std::unordered_set<std::string> seen_files;
            FillFileDescriptorResponse(file_desc, response, &seen_files);
            return Status::OK;
        }

        Status GetFileContainingSymbol(
            ServerContext* context, const std::string& symbol,
            grpc::reflection::v1alpha::ServerReflectionResponse* response){ 

            if (descriptor_pool_ == nullptr) {
                return Status::CANCELLED;
            }

            const grpc::protobuf::FileDescriptor* file_desc =
                descriptor_pool_->FindFileContainingSymbol(symbol);

            if (file_desc == nullptr) {
                file_desc = other_pool.FindFileContainingSymbol(symbol);
            }

            if (file_desc == nullptr) {
                return Status(grpc::StatusCode::NOT_FOUND, "Symbol not found.");
            }
            std::unordered_set<std::string> seen_files;
            FillFileDescriptorResponse(file_desc, response, &seen_files);
            return Status::OK;

        }

        Status GetFileContainingExtension(
            ServerContext* context,
            const grpc::reflection::v1alpha::ExtensionRequest* request,
            grpc::reflection::v1alpha::ServerReflectionResponse* response) {
            if (descriptor_pool_ == nullptr) {
                return Status::CANCELLED;
            }

            const grpc::protobuf::Descriptor* desc =
                descriptor_pool_->FindMessageTypeByName(request->containing_type());
            /*if (desc == nullptr) {
                desc = protoParser.m_FileDescriptor->pool()->FindMessageTypeByName(request->containing_type());
            }*/
            if (desc == nullptr) {
                return Status(grpc::StatusCode::NOT_FOUND, "Type not found.");
            }

            const grpc::protobuf::FieldDescriptor* field_desc = descriptor_pool_->FindExtensionByNumber(desc, request->extension_number());
            /*if (field_desc == nullptr) {
                field_desc = protoParser.m_FileDescriptor->pool()->FindExtensionByNumber(desc, request->extension_number());
            }*/
            if (field_desc == nullptr) {
                return Status(grpc::StatusCode::NOT_FOUND, "Extension not found.");
            }
            std::unordered_set<std::string> seen_files;
            FillFileDescriptorResponse(field_desc->file(), response, &seen_files);
            return Status::OK;
        }

        Status GetAllExtensionNumbers(
            ServerContext* context, const std::string& type,
            grpc::reflection::v1alpha::ExtensionNumberResponse* response) {
            if (descriptor_pool_ == nullptr) {
                return Status::CANCELLED;
            }

            const grpc::protobuf::Descriptor* desc =
                descriptor_pool_->FindMessageTypeByName(type);
           /* if (desc == nullptr) {
                desc = protoParser.m_FileDescriptor->pool()->FindMessageTypeByName(type);
            }*/
            if (desc == nullptr) {
                return Status(grpc::StatusCode::NOT_FOUND, "Type not found.");
            }

            std::vector<const grpc::protobuf::FieldDescriptor*> extensions;
            descriptor_pool_->FindAllExtensions(desc, &extensions);
            /*if (extensions.empty())
            {
                protoParser.m_FileDescriptor->pool()->FindAllExtensions(desc, &extensions);
            }*/
            for (const auto& value : extensions) {
                response->add_extension_number(value->number());
            }
            response->set_base_type_name(type);
            return Status::OK;
        }

        void FillFileDescriptorResponse(
            const grpc::protobuf::FileDescriptor* file_desc,
            grpc::reflection::v1alpha::ServerReflectionResponse* response,
            std::unordered_set<std::string>* seen_files) {
            if (seen_files->find(file_desc->name()) != seen_files->end()) {
                return;
            }
            seen_files->insert(file_desc->name());

            grpc::protobuf::FileDescriptorProto file_desc_proto;
            std::string data;
            file_desc->CopyTo(&file_desc_proto);
            file_desc_proto.SerializeToString(&data);
            response->mutable_file_descriptor_response()->add_file_descriptor_proto(data);

            for (int i = 0; i < file_desc->dependency_count(); ++i) {
                FillFileDescriptorResponse(file_desc->dependency(i), response, seen_files);
            }
        }

        void FillErrorResponse(const Status& status,
            grpc::reflection::v1alpha::ErrorResponse* error_response) {}

        const grpc::protobuf::DescriptorPool* descriptor_pool_;
        grpc::protobuf::DescriptorPool other_pool;
        // std::map<const std::string, std::unique_ptr<const grpc::protobuf::FileDescriptor*>> custom_descriptor_poolMap;
        std::vector<std::unique_ptr<const grpc::protobuf::FileDescriptor*>> fileDescriptorVec;
        std::vector<std::string>* services_;
        // LVProtoParser protoParser;
    };

    class LVProtoServerReflectionPlugin : public ::grpc::ServerBuilderPlugin {
    public:
        ::std::string name() override { return "LVProtoServerReflectionPlugin"; }
        void InitServer(ServerInitializer* si) override { si->RegisterService(reflection_service_); }
        void Finish(ServerInitializer* si) override { reflection_service_->SetServiceList(si->GetServiceList()); }
        void ChangeArguments(const ::std::string& name, void* value) override {}
        bool has_async_methods() const override { return false; }
        bool has_sync_methods() const override { return true; }

        void AddService(std::string serviceName) { reflection_service_.get()->AddService(serviceName); }

        void FindFile(std::string path) { reflection_service_.get()->FindFile(path); }

        void ImportFile(std::string path, std::string searchPath) { reflection_service_.get()->ImportFile(path, searchPath); }

        void AddFileDescriptorProto(const std::string& serializedProto) { reflection_service_.get()->AddFileDescriptorProto(serializedProto); }

        // TODO: Remove file descriptor

        static LVProtoServerReflectionPlugin* GetInstance() {
            if (m_instance == nullptr)
            {
                m_instance = new LVProtoServerReflectionPlugin();
            }
            return m_instance;
        }

        void DeleteInstance() { m_instance = nullptr;}

    private:
        LVProtoServerReflectionPlugin() : reflection_service_(new grpc_labview::LVProtoServerReflectionService()) {}

        // LVProtoServerReflectionPlugin(); // hide constructors
        static LVProtoServerReflectionPlugin* m_instance;
        std::shared_ptr<grpc_labview::LVProtoServerReflectionService> reflection_service_;
    };

    LVProtoServerReflectionPlugin* LVProtoServerReflectionPlugin::m_instance = nullptr;


    static std::unique_ptr< ::grpc::ServerBuilderPlugin> CreateLVProtoReflection() {
        return std::unique_ptr< ::grpc::ServerBuilderPlugin>(
            LVProtoServerReflectionPlugin::GetInstance());
    }

    void InitLVProtoReflectionServerBuilderPlugin() {
        static struct Initialize {
            Initialize() {
                ::grpc::ServerBuilder::InternalAddPluginFactory(&CreateLVProtoReflection);
            }
        } initializer;
    }

    LIBRARY_EXPORT void DeserializeReflectionInfo(grpc_labview::LStrHandle serializedFileDescriptor)
    {
        std::string serializedDescriptorStr = grpc_labview::GetLVString(serializedFileDescriptor);
        LVProtoServerReflectionPlugin::GetInstance()->AddFileDescriptorProto(serializedDescriptorStr);
    }

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
        LVProtoServerReflectionPlugin::GetInstance()->FindFile("helloworld.proto");
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
            _runThread->join();
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
            // Block waiting to read the next event from the completion queue. The
            // event is uniquely identified by its tag, which in this case is the
            // memory address of a CallData instance.
            cq->Next(&tag, &ok);
            static_cast<CallDataBase*>(tag)->Proceed(ok);
            if (_shutdown)
            {
                break;
            }
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
        LVProtoServerReflectionPlugin::GetInstance()->ImportFile("helloworld.proto", "C:\\Workdesk\\lv-grpc-examples\\helloworld");

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

        // auto _reflectionService = std::unique_ptr<grpc_labview::LVProtoServerReflectionService>(new grpc_labview::LVProtoServerReflectionService());
        // builder.RegisterService(_reflectionService.get());

        auto cq = builder.AddCompletionQueue();

        _server = builder.BuildAndStart();
        if (_server != nullptr)
        {
            std::cout << "Server listening on " << server_address << std::endl;
            serverStarted->NotifyComplete();

            HandleRpcs(cq.get());
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
            _runThread->join();
            _server = nullptr;
        }
        LVProtoServerReflectionPlugin::GetInstance()->DeleteInstance();
    }
}
