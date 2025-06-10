//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include "lv_proto_server_reflection_service.h"

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerInitializer;
using grpc::reflection::v1alpha::ServerReflectionRequest;
using grpc::reflection::v1alpha::ServerReflectionResponse;
using grpc::protobuf::FileDescriptorProto;

namespace grpc_labview
{
    //---------------------------------------------------------------------
    // During construction of the LVProtoServerReflectionService, generated_pool() is used to register all built-in
    // gRPC messages into the descriptor pool.  
    //---------------------------------------------------------------------
    LVProtoServerReflectionService::LVProtoServerReflectionService() :
        grpc_descriptor_pool_(grpc::protobuf::DescriptorPool::generated_pool()), services_(new std::vector<std::string>()) {
       
        // Add the reflection service name manually to the published service list.  The actual methods 
        // for the reflection service are given are registered by `grpc::protobuf::DescriptorPool::generated_pool()` 
        // used during construction, but the actual service name needs to be added as they are tracked
        // separately
        AddService("grpc.reflection.v1alpha.ServerReflection");
    }

    //---------------------------------------------------------------------
    // Adds a service name to the list of services to be published via gRPC reflection.
    // 
    // As registering a service name only does not provide any description of the gRPC functions, messages,
    // or structures used by that gRPC service, it is assumed that this function is only used to register
    // built-in gRPC services (such as the reflection service itself).  Built-in gRPC messages and structures
    // as provided by the `grpc_descriptor_pool`, populated using the `generated_pool()` in the constructor.
    //---------------------------------------------------------------------
    void LVProtoServerReflectionService::AddService(const std::string serviceName) {
        services_->push_back(serviceName);
    }

    //---------------------------------------------------------------------
    // Adds a serialized proto descriptor string to the pool of known gRPC published methods which are
    // published via gRPC reflection.  
    // 
    // When calling AddFileDescriptorProtoString, any gRPC services found in the descriptor string
    // are automatically added to the list of registered services.
    //---------------------------------------------------------------------
    void LVProtoServerReflectionService::AddFileDescriptorProtoString(const std::string& serializedProtoStr) {
        // Parse the serialized proto string into a FileDescriptorProto, then query how many
        // services are present in that proto file.  Add those services to the services_ list
        FileDescriptorProto proto;
        if (!proto.ParseFromString(serializedProtoStr)) {
            return;
        }
        const auto* proto_file_descriptor = lv_descriptor_pool_.BuildFile(proto);

        if (proto_file_descriptor != nullptr)
        {
            int numServices = proto_file_descriptor->service_count();
            for (int i = 0; i < numServices; ++i)
            {
                const google::protobuf::ServiceDescriptor* serviceDescriptor = proto_file_descriptor->service(i);
                services_->push_back(serviceDescriptor->full_name());
            }
        }
    }

    //---------------------------------------------------------------------
    // Implementation of ServerReflectionInfo(stream ServerReflectionRequest) rpc
    // in ServerReflection service
    //---------------------------------------------------------------------
    Status LVProtoServerReflectionService::ServerReflectionInfo(
        ServerContext* context,
        grpc::ServerReaderWriter<grpc::reflection::v1alpha::ServerReflectionResponse,
        grpc::reflection::v1alpha::ServerReflectionRequest>* stream) {

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
            case ServerReflectionRequest::MessageRequestCase::kAllExtensionNumbersOfType:
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

    }
    
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    Status LVProtoServerReflectionService::ListService(ServerContext* context,
        grpc::reflection::v1alpha::ListServiceResponse* response) {

        if (services_ == nullptr) {
            return Status(grpc::StatusCode::NOT_FOUND, "Services not found.");
        }
        for (const auto& value : *services_) {
            grpc::reflection::v1alpha::ServiceResponse* service_response = response->add_service();
            service_response->set_name(value);
        }
        
        return Status::OK;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    Status LVProtoServerReflectionService::GetFileByName(ServerContext* context, const std::string& file_name,
        grpc::reflection::v1alpha::ServerReflectionResponse* response) {

        if (grpc_descriptor_pool_ == nullptr) {
            return Status::CANCELLED;
        }

        const grpc::protobuf::FileDescriptor* file_desc =
            grpc_descriptor_pool_->FindFileByName(file_name);
        if (file_desc == nullptr) {
            // check in the lv descriptor pool
            file_desc = lv_descriptor_pool_.FindFileByName(file_name);
        }

        if (file_desc == nullptr) // we couldn't find it anywhere
            return Status(grpc::StatusCode::NOT_FOUND, "File not found.");

        std::unordered_set<std::string> seen_files;
        FillFileDescriptorResponse(file_desc, response, &seen_files);
        return Status::OK;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    Status LVProtoServerReflectionService::GetFileContainingSymbol(
        ServerContext* context, const std::string& symbol,
        grpc::reflection::v1alpha::ServerReflectionResponse* response) {

        if (grpc_descriptor_pool_ == nullptr) {
            return Status::CANCELLED;
        }

        const grpc::protobuf::FileDescriptor* file_desc =
            grpc_descriptor_pool_->FindFileContainingSymbol(symbol);

        if (file_desc == nullptr) {
            file_desc = lv_descriptor_pool_.FindFileContainingSymbol(symbol);
        }

        if (file_desc == nullptr) {
            return Status(grpc::StatusCode::NOT_FOUND, "Symbol not found.");
        }
        std::unordered_set<std::string> seen_files;
        FillFileDescriptorResponse(file_desc, response, &seen_files);
        return Status::OK;

    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    Status LVProtoServerReflectionService::GetFileContainingExtension(
        ServerContext* context,
        const grpc::reflection::v1alpha::ExtensionRequest* request,
        grpc::reflection::v1alpha::ServerReflectionResponse* response) {
        if (grpc_descriptor_pool_ == nullptr) {
            return Status::CANCELLED;
        }

        const grpc::protobuf::Descriptor* desc =
            grpc_descriptor_pool_->FindMessageTypeByName(request->containing_type());
        if (desc == nullptr) {
            desc = lv_descriptor_pool_.FindMessageTypeByName(request->containing_type());
        }
        if (desc == nullptr) {
            return Status(grpc::StatusCode::NOT_FOUND, "Type not found.");
        }

        const grpc::protobuf::FieldDescriptor* field_desc = grpc_descriptor_pool_->FindExtensionByNumber(desc, request->extension_number());
        if (field_desc == nullptr) {
            field_desc = lv_descriptor_pool_.FindExtensionByNumber(desc, request->extension_number());
        }
        if (field_desc == nullptr) {
            return Status(grpc::StatusCode::NOT_FOUND, "Extension not found.");
        }
        std::unordered_set<std::string> seen_files;
        FillFileDescriptorResponse(field_desc->file(), response, &seen_files);
        return Status::OK;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    Status LVProtoServerReflectionService::GetAllExtensionNumbers(
        ServerContext* context, const std::string& type,
        grpc::reflection::v1alpha::ExtensionNumberResponse* response) {
        if (grpc_descriptor_pool_ == nullptr) {
            return Status::CANCELLED;
        }

        const grpc::protobuf::Descriptor* desc =
            grpc_descriptor_pool_->FindMessageTypeByName(type);
        if (desc == nullptr) 
            desc = lv_descriptor_pool_.FindMessageTypeByName(type);

        if (desc == nullptr)
            return Status(grpc::StatusCode::NOT_FOUND, "Type not found.");        

        std::vector<const grpc::protobuf::FieldDescriptor*> extensions;
        grpc_descriptor_pool_->FindAllExtensions(desc, &extensions);
        if (extensions.empty())
            lv_descriptor_pool_.FindAllExtensions(desc, &extensions);
        for (const auto& value : extensions) {
            response->add_extension_number(value->number());
        }
        response->set_base_type_name(type);
        return Status::OK;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVProtoServerReflectionService::FillFileDescriptorResponse(
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

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVProtoServerReflectionService::FillErrorResponse(const Status& status,
        grpc::reflection::v1alpha::ErrorResponse* error_response) {
        error_response->set_error_code(status.error_code());
        error_response->set_error_message(status.error_message());
    }
}