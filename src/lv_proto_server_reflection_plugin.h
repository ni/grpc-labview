//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <string>
#include <memory>
#include <pointer_manager.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef _WIN32
    #define LIBRARY_EXPORT extern "C" __declspec(dllexport)
#else
    #define LIBRARY_EXPORT extern "C"
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include "lv_proto_server_reflection_service.h"
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <src/proto/grpc/reflection/v1alpha/reflection.grpc.pb.h>
#include <grpcpp/impl/server_initializer.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::ServerContext;
using grpc::ServerInitializer;

namespace grpc_labview 
{
    class LVProtoServerReflectionPlugin : public ::grpc::ServerBuilderPlugin {
    public:
        LVProtoServerReflectionPlugin();
        ::std::string name() override;
        void InitServer(ServerInitializer* si) override;
        void Finish(ServerInitializer* si) override;
        void ChangeArguments(const ::std::string& name, void* value) override;
        bool has_async_methods() const override;
        bool has_sync_methods() const override;
        void AddService(std::string serviceName);
        void AddFileDescriptorProto(const std::string& serializedProto);

    private:
        std::shared_ptr<grpc_labview::LVProtoServerReflectionService> reflection_service_;
    };

    std::unique_ptr<::grpc::ServerBuilderPlugin> CreateLVProtoReflection();
    void InitLVProtoReflectionServerBuilderPlugin();

}
