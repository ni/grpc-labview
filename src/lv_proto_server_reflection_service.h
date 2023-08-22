//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <grpcpp/impl/server_initializer.h>
#include <src/proto/grpc/reflection/v1alpha/reflection.grpc.pb.h>

using grpc::ServerContext;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Status;
using grpc::ServerInitializer;
using grpc::reflection::v1alpha::ServerReflectionRequest;
using grpc::reflection::v1alpha::ServerReflectionResponse;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef _WIN32
    #define LIBRARY_EXPORT extern "C" __declspec(dllexport)
#else
    #define LIBRARY_EXPORT extern "C"
#endif

namespace grpc_labview 
{
    class LVProtoServerReflectionService final : public grpc::reflection::v1alpha::ServerReflection::Service
    {
    public:
        LVProtoServerReflectionService();
        
        Status ServerReflectionInfo(
            ServerContext* context,
            grpc::ServerReaderWriter<grpc::reflection::v1alpha::ServerReflectionResponse,
            grpc::reflection::v1alpha::ServerReflectionRequest>* stream)
            override;

        void SetServiceList(const std::vector<std::string>* snames);
        void AddService(const std::string serviceName);
        void AddFileDescriptorProto(const std::string& serializedProtoStr);
        void AddOtherPoolServices();

    private:
        Status ListService(ServerContext* context, grpc::reflection::v1alpha::ListServiceResponse* response);
        Status GetFileByName(ServerContext* context, const std::string& file_name, grpc::reflection::v1alpha::ServerReflectionResponse* response);
        Status GetFileContainingSymbol(ServerContext* context, const std::string& symbol, grpc::reflection::v1alpha::ServerReflectionResponse* response);

        Status GetFileContainingExtension(ServerContext* context,
            const grpc::reflection::v1alpha::ExtensionRequest* request,
            grpc::reflection::v1alpha::ServerReflectionResponse* response);

        Status GetAllExtensionNumbers(
            ServerContext* context, const std::string& type,
            grpc::reflection::v1alpha::ExtensionNumberResponse* response);

        void FillFileDescriptorResponse(
            const grpc::protobuf::FileDescriptor* file_desc,
            grpc::reflection::v1alpha::ServerReflectionResponse* response,
            std::unordered_set<std::string>* seen_files);

        void FillErrorResponse(const Status& status,
            grpc::reflection::v1alpha::ErrorResponse* error_response);

        const grpc::protobuf::DescriptorPool* descriptor_pool_;
        grpc::protobuf::DescriptorPool other_pool;
        std::vector<std::string>* services_;

        struct OtherPoolServiceInfo
        {
            const grpc::protobuf::FileDescriptor* other_pool_file_descriptor;
            std::vector<std::string> other_pool_services_;
        };

        std::unique_ptr<OtherPoolServiceInfo> other_pool_services_info_ptr;
    };
}
