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

namespace grpc_labview 
{
    class gRPCid;
    extern PointerManager<gRPCid> gClientTokenManager;

    //---------------------------------------------------------------------
    // LabVIEW gRPC definitions
    //---------------------------------------------------------------------
    class gRPCid
    {
    public:

        /// NOTE: It is important that the CastTo method never be made virtual and never access any member variables.
        /// This code is expected to work: ((gRPCid*)nullptr)->CastTo<gRPCid>();
        template <typename T>
        std::shared_ptr<T> CastTo()
        {
            return gClientTokenManager.TryCastTo<T>(this);
        }

        virtual ~gRPCid() { }
    protected:
        gRPCid() { }
    };

    int AlignClusterOffset(int clusterOffset, int alignmentRequirement);

    //---------------------------------------------------------------------
    // LabVIEW definitions
    //---------------------------------------------------------------------
    typedef int32_t MagicCookie;
    typedef MagicCookie LVRefNum;
    typedef MagicCookie LVUserEventRef;
    typedef int32_t(*CleanupProcPtr)(gRPCid* id);

    struct LStr {
        int32_t cnt; /* number of bytes that follow */
        char str[1]; /* cnt bytes */
    };

    using LStrPtr = LStr*;
    using LStrHandle =  LStr**;

    struct LV1DArray {
        int32_t cnt; /* number of bytes that follow */
        int8_t rawBytes[1]; /* cnt bytes */

        template<typename T>
        T* bytes()
        {
            static_assert(!std::is_class<T>::value, "T must not be a struct/class type.");
            return (T*)(bytes(0, sizeof(T)));
        }

        template<typename T>
        T* bytes(int byteOffset)
        {
            static_assert(!std::is_class<T>::value, "T must not be a struct/class type.");
            return (T*)(bytes(byteOffset, sizeof(T)));
        }

        void* bytes(int byteOffset, int byteAlignment)
        {
            return (void*)(rawBytes + AlignClusterOffset(4, byteAlignment) - 4 + byteOffset);
        }
    };

    using LV1DArrayPtr = LV1DArray*;
    using LV1DArrayHandle = LV1DArray**;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    #ifdef _PS_4
    #pragma pack (push, 1)
    #endif
    struct AnyCluster
    {    
        LStrHandle TypeUrl;
        LV1DArrayHandle Bytes;
    };
    #ifdef _PS_4
    #pragma pack (pop)
    #endif

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void InitCallbacks();
    void SetLVString(LStrHandle* lvString, std::string str);
    std::string GetLVString(LStrHandle lvString);
    int NumericArrayResize(int32_t typeCode, int32_t numDims, void* handle, size_t size);
    int PostUserEvent(LVUserEventRef ref, void *data);
    int SignalOccurrence(MagicCookie occurrence);
    int32_t RegisterCleanupProc(CleanupProcPtr cleanUpProc, grpc_labview::gRPCid* id);
    int32_t DeregisterCleanupProc(CleanupProcPtr cleanUpProc, grpc_labview::gRPCid* id);
}
