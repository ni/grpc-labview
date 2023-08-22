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
    LVProtoServerReflectionService::LVProtoServerReflectionService() :
        descriptor_pool_(grpc::protobuf::DescriptorPool::generated_pool()), services_(new std::vector<std::string>()) {
             other_pool_services_info_ptr = std::make_unique<OtherPoolServiceInfo>();
    }

    // Add the full names of registered services
    void LVProtoServerReflectionService::SetServiceList(const std::vector<std::string>* snames) {
        for (int i = 0; i < snames->size(); ++i)
            services_->push_back(snames->at(i));
    }

    void LVProtoServerReflectionService::AddService(const std::string serviceName) {
        services_->push_back(serviceName);
    }

    // Implementation of ServerReflectionInfo(stream ServerReflectionRequest) rpc
    // in ServerReflection service

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


    void LVProtoServerReflectionService::AddFileDescriptorProto(const std::string& serializedProtoStr) {
        FileDescriptorProto proto;
        proto.ParseFromString(serializedProtoStr);
        other_pool_services_info_ptr->other_pool_file_descriptor = other_pool.BuildFile(proto);       
        AddOtherPoolServices();
    }

    void LVProtoServerReflectionService::AddOtherPoolServices()
    {
        if (other_pool_services_info_ptr->other_pool_file_descriptor != nullptr)
        {
            int numServices = other_pool_services_info_ptr->other_pool_file_descriptor->service_count();
            for (int i = 0; i < numServices; ++i)
            {
                const google::protobuf::ServiceDescriptor* serviceDescriptor = other_pool_services_info_ptr->other_pool_file_descriptor->service(i);
                other_pool_services_info_ptr->other_pool_services_.push_back(serviceDescriptor->full_name());
            }
        }
    }


    Status LVProtoServerReflectionService::ListService(ServerContext* context,
        grpc::reflection::v1alpha::ListServiceResponse* response) {

        if (services_ == nullptr) {
            return Status(grpc::StatusCode::NOT_FOUND, "Services not found.");
        }
        for (const auto& value : *services_) {
            grpc::reflection::v1alpha::ServiceResponse* service_response = response->add_service();
            service_response->set_name(value);
        }
        for (const auto value : other_pool_services_info_ptr->other_pool_services_) {
            grpc::reflection::v1alpha::ServiceResponse* service_response = response->add_service();
            service_response->set_name(value);
        }

        return Status::OK;
    }

    Status LVProtoServerReflectionService::GetFileByName(ServerContext* context, const std::string& file_name,
        grpc::reflection::v1alpha::ServerReflectionResponse* response) {

        if (descriptor_pool_ == nullptr) {
            return Status::CANCELLED;
        }

        const grpc::protobuf::FileDescriptor* file_desc =
            descriptor_pool_->FindFileByName(file_name);
        if (file_desc == nullptr) {
            // check in other pools
            file_desc = other_pool.FindFileByName(file_name);
        }

        if (file_desc == nullptr) // we couldn't find it anywhere
            return Status(grpc::StatusCode::NOT_FOUND, "File not found.");

        std::unordered_set<std::string> seen_files;
        FillFileDescriptorResponse(file_desc, response, &seen_files);
        return Status::OK;
    }

    Status LVProtoServerReflectionService::GetFileContainingSymbol(
        ServerContext* context, const std::string& symbol,
        grpc::reflection::v1alpha::ServerReflectionResponse* response) {

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

    Status LVProtoServerReflectionService::GetFileContainingExtension(
        ServerContext* context,
        const grpc::reflection::v1alpha::ExtensionRequest* request,
        grpc::reflection::v1alpha::ServerReflectionResponse* response) {
        if (descriptor_pool_ == nullptr) {
            return Status::CANCELLED;
        }

        const grpc::protobuf::Descriptor* desc =
            descriptor_pool_->FindMessageTypeByName(request->containing_type());
        if (desc == nullptr) {
            desc = other_pool.FindMessageTypeByName(request->containing_type());
        }
        if (desc == nullptr) {
            return Status(grpc::StatusCode::NOT_FOUND, "Type not found.");
        }

        const grpc::protobuf::FieldDescriptor* field_desc = descriptor_pool_->FindExtensionByNumber(desc, request->extension_number());
        if (field_desc == nullptr) {
            field_desc = other_pool.FindExtensionByNumber(desc, request->extension_number());
        }
        if (field_desc == nullptr) {
            return Status(grpc::StatusCode::NOT_FOUND, "Extension not found.");
        }
        std::unordered_set<std::string> seen_files;
        FillFileDescriptorResponse(field_desc->file(), response, &seen_files);
        return Status::OK;
    }

    Status LVProtoServerReflectionService::GetAllExtensionNumbers(
        ServerContext* context, const std::string& type,
        grpc::reflection::v1alpha::ExtensionNumberResponse* response) {
        if (descriptor_pool_ == nullptr) {
            return Status::CANCELLED;
        }

        const grpc::protobuf::Descriptor* desc =
            descriptor_pool_->FindMessageTypeByName(type);
        if (desc == nullptr) 
            desc = other_pool.FindMessageTypeByName(type);

        if (desc == nullptr)
            return Status(grpc::StatusCode::NOT_FOUND, "Type not found.");        

        std::vector<const grpc::protobuf::FieldDescriptor*> extensions;
        descriptor_pool_->FindAllExtensions(desc, &extensions);
        if (extensions.empty())
            other_pool.FindAllExtensions(desc, &extensions);
        for (const auto& value : extensions) {
            response->add_extension_number(value->number());
        }
        response->set_base_type_name(type);
        return Status::OK;
    }

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

    void LVProtoServerReflectionService::FillErrorResponse(const Status& status,
        grpc::reflection::v1alpha::ErrorResponse* error_response) {
        error_response->set_error_code(status.error_code());
        error_response->set_error_message(status.error_message());
    }
}