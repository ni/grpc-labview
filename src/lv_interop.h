//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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
    #define LIBRARY_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace grpc_labview
{
    class gRPCid;
    extern PointerManager<gRPCid> gPointerManager;

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
            return gPointerManager.TryCastTo<T>(this);
        }

        virtual ~gRPCid() { }
    protected:
        gRPCid() { }
    };

    int AlignClusterOffset(int clusterOffset, int alignmentRequirement);

    // Provides type code for use with NumericArrayResize function for various sizes of data types.
    int32_t GetTypeCodeForSize(int byteSize);

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
    using LStrHandle = LStr**;

    struct LV1DArray {
        int32_t cnt; /* number of T elements that follow */
        int8_t rawBytes[1]; /* (cnt * sizeof(T)) bytes */

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

    struct LV2DArray {
        int32_t dimensionSizes[2]; /* number of T elements for each dimension */
        int8_t rawBytes[1]; /* (firstDimensionSize * secondDimensionSize * sizeof(T)) bytes */

        // The start of 2D array data is always aligned and does not require padding.
        template<typename T>
        T* bytes()
        {
            static_assert(!std::is_class<T>::value, "T must not be a struct/class type.");
            static_assert(sizeof(T) <= 8, "Need to revisit logic if we ever have element size larger than 8 bytes.");
            return (T*)(rawBytes);
        }
    };

    using LV2DArrayPtr = LV2DArray*;
    using LV2DArrayHandle = LV2DArray**;

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
    void SetLVRTModulePath(std::string modulePath);
    void InitCallbacks();
    void SetLVString(LStrHandle* lvString, std::string str);
    std::string GetLVString(LStrHandle lvString);
    int NumericArrayResize(int32_t typeCode, int32_t numDims, void* handle, size_t size);
    int PostUserEvent(LVUserEventRef ref, void* data);
    unsigned char** DSNewHandle(size_t n);
    int DSSetHandleSize(void* h, size_t n);
    long DSDisposeHandle(void* h);
    int SignalOccurrence(MagicCookie occurrence);
    int32_t RegisterCleanupProc(CleanupProcPtr cleanUpProc, grpc_labview::gRPCid* id);
    int32_t DeregisterCleanupProc(CleanupProcPtr cleanUpProc, grpc_labview::gRPCid* id);
}
