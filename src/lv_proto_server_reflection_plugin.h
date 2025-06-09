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
    class LVProtoServerReflectionPlugin {
    public:
        LVProtoServerReflectionPlugin();

        void AddFileDescriptorProto(const std::string& serializedProto);
        void AddService(const std::string& serviceName);

        grpc::Service* GetService();

    private:
        std::shared_ptr<grpc_labview::LVProtoServerReflectionService> reflection_service_;
    };
}