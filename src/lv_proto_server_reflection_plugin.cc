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
    LVProtoServerReflectionPlugin::LVProtoServerReflectionPlugin() : reflection_service_(new grpc_labview::LVProtoServerReflectionService()) {
    }

    std::string LVProtoServerReflectionPlugin::name() { 
        return "LVProtoServerReflectionPlugin";
    }

    void LVProtoServerReflectionPlugin::InitServer(ServerInitializer* si) {
        si->RegisterService(reflection_service_);
    }

    void LVProtoServerReflectionPlugin::Finish(ServerInitializer* si) {
        reflection_service_->SetServiceList(si->GetServiceList());
    }

    void LVProtoServerReflectionPlugin::ChangeArguments(const ::std::string& name, void* value) {
        // TODO
    }

    bool LVProtoServerReflectionPlugin::has_async_methods() const {
        return false; // TODO
    }
    bool LVProtoServerReflectionPlugin::has_sync_methods() const {
        return true; // TODO
    }

    void LVProtoServerReflectionPlugin::AddService(std::string serviceName) {
        reflection_service_.get()->AddService(serviceName);
    }

    void LVProtoServerReflectionPlugin::AddFileDescriptorProto(const std::string& serializedProto) {
        reflection_service_.get()->AddFileDescriptorProto(serializedProto);
    }

    std::unique_ptr< ::grpc::ServerBuilderPlugin> CreateLVProtoReflection() {

        LVProtoServerReflectionPlugin* reflectionPluginInstance = new LVProtoServerReflectionPlugin();
        reflectionPluginInstance->AddFileDescriptorProto(grpc_labview::ProtoDescriptorString::getInstance()->getDescriptor());
        return std::unique_ptr< ::grpc::ServerBuilderPlugin>(reflectionPluginInstance);
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
        grpc_labview::ProtoDescriptorString::getInstance()->setDescriptor(serializedDescriptorStr);
    }
}
