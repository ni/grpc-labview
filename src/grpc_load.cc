#ifndef _WIN32
#include <dlfcn.h>
typedef void* LibHandle;
#else
#include <wtypes.h>
typedef HMODULE LibHandle;
#endif

LibHandle LockgRPCLibraryIntoProcessMem()
{
#if _WIN32
    auto dllHandle = LoadLibrary("labview_grpc_server.dll");
#else
    auto dllHandle = dlopen("liblabview_grpc_server.so", RTLD_LAZY);
#endif
    return dllHandle;
}

namespace grpc_labview
{
    LibHandle gSelfLibHandle = LockgRPCLibraryIntoProcessMem();
}