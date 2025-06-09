//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <lv_proto_server_reflection_plugin.h>
#include <grpcpp/impl/server_initializer.h>
#include <grpcpp/server_builder.h>
#include <lv_interop.h>
#include <grpc_server.h>
//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerInitializer;
using grpc::ServerBuilder;

namespace grpc_labview
{
    LVProtoServerReflectionPlugin::LVProtoServerReflectionPlugin() 
        : reflection_service_(std::make_shared<grpc_labview::LVProtoServerReflectionService>()) {}

    void LVProtoServerReflectionPlugin::AddFileDescriptorProto(const std::string& serializedProto)
    {
        reflection_service_->AddFileDescriptorProto(serializedProto);
    }

    void LVProtoServerReflectionPlugin::AddService(const std::string& serviceName)
    {
        reflection_service_->AddService(serviceName);
    }

    grpc::Service* LVProtoServerReflectionPlugin::GetService()
    {
        return reflection_service_.get();
    }
}
