//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <lv_interop.h>
#include <iostream>
#include <cstring>
#include <memory>
#include <grpcpp/grpcpp.h>
#include <feature_toggles.h>

#ifndef _WIN32
#include <dlfcn.h>
#endif

// Value passed to the RTSetCleanupProc to remove the clean up proc for the VI.
static int kCleanOnRemove = 0;
// Value passed to the RTSetCleanUpProc to register a cleanup proc for a VI which
// gets called when the VI state changes to Idle.
static int kCleanOnIdle = 2;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
typedef int (*NumericArrayResize_T)(int32_t, int32_t, void* handle, size_t size);
typedef int (*PostLVUserEvent_T)(grpc_labview::LVUserEventRef ref, void* data);
typedef int (*Occur_T)(grpc_labview::MagicCookie occurrence);
typedef int32_t(*RTSetCleanupProc_T)(grpc_labview::CleanupProcPtr cleanUpProc, grpc_labview::gRPCid* id, int32_t mode);
typedef unsigned char** (*DSNewHandlePtr_T)(size_t);
typedef int (*DSSetHandleSize_T)(void* h, size_t);
typedef long (*DSDisposeHandle_T)(void* h);


//---------------------------------------------------------------------
//---------------------------------------------------------------------
static NumericArrayResize_T NumericArrayResizeImp = nullptr;
static PostLVUserEvent_T PostLVUserEvent = nullptr;
static Occur_T Occur = nullptr;
static RTSetCleanupProc_T RTSetCleanupProc = nullptr;

static std::string ModulePath = "";
static DSNewHandlePtr_T DSNewHandleImpl = nullptr;
static DSSetHandleSize_T DSSetHandleSizeImpl = nullptr;
static DSDisposeHandle_T DSDisposeHandleImpl = nullptr;

namespace grpc_labview
{
    grpc_labview::PointerManager<grpc_labview::gRPCid> gPointerManager;

    //---------------------------------------------------------------------
    // Allows for definition of the LVRT DLL path to be used for callback functions
    // This function should be called prior to calling InitCallbacks()
    //---------------------------------------------------------------------
    void SetLVRTModulePath(std::string modulePath)
    {
        ModulePath = modulePath;
    }

#ifdef _WIN32

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void InitCallbacks()
    {
        // Instantiating the feature toggles singleton that will read the feature configuration file
        // FeatureConfig::getInstance().readConfigFromFile("feature_config.ini");

        if (NumericArrayResizeImp != nullptr)
        {
            return;
        }

        HMODULE lvModule;

        if (ModulePath != "")
        {
            lvModule = GetModuleHandle(ModulePath.c_str());
        }
        else
        {
            lvModule = GetModuleHandle("LabVIEW.exe");
            if (lvModule == nullptr)
            {
                lvModule = GetModuleHandle("lvffrt.dll");
            }
            if (lvModule == nullptr)
            {
                lvModule = GetModuleHandle("lvrt.dll");
            }
        }
        NumericArrayResizeImp = (NumericArrayResize_T)GetProcAddress(lvModule, "NumericArrayResize");
        PostLVUserEvent = (PostLVUserEvent_T)GetProcAddress(lvModule, "PostLVUserEvent");
        Occur = (Occur_T)GetProcAddress(lvModule, "Occur");
        RTSetCleanupProc = (RTSetCleanupProc_T)GetProcAddress(lvModule, "RTSetCleanupProc");
        DSNewHandleImpl = (DSNewHandlePtr_T)GetProcAddress(lvModule, "DSNewHandle");
        DSSetHandleSizeImpl = (DSSetHandleSize_T)GetProcAddress(lvModule, "DSSetHandleSize");
        DSDisposeHandleImpl = (DSDisposeHandle_T)GetProcAddress(lvModule, "DSDisposeHandle");
    }

#else

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void InitCallbacks()
    {
        if (NumericArrayResizeImp != nullptr)
        {
            return;
        }

        NumericArrayResizeImp = (NumericArrayResize_T)dlsym(RTLD_DEFAULT, "NumericArrayResize");
        PostLVUserEvent = (PostLVUserEvent_T)dlsym(RTLD_DEFAULT, "PostLVUserEvent");
        Occur = (Occur_T)dlsym(RTLD_DEFAULT, "Occur");
        RTSetCleanupProc = (RTSetCleanupProc_T)dlsym(RTLD_DEFAULT, "RTSetCleanupProc");
        DSNewHandleImpl = (DSNewHandlePtr_T)dlsym(RTLD_DEFAULT, "DSNewHandle");
        DSSetHandleSizeImpl = (DSSetHandleSize_T)dlsym(RTLD_DEFAULT, "DSSetHandleSize");
        DSDisposeHandleImpl = (DSDisposeHandle_T)dlsym(RTLD_DEFAULT, "DSDisposeHandle");
    }

#endif

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int SignalOccurrence(MagicCookie occurrence)
    {
        return Occur(occurrence);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int NumericArrayResize(int32_t typeCode, int32_t numDims, void* handle, size_t size)
    {
        return NumericArrayResizeImp(typeCode, numDims, handle, size);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int PostUserEvent(LVUserEventRef ref, void* data)
    {
        return PostLVUserEvent(ref, data);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    unsigned char** DSNewHandle(size_t n)
    {
        return DSNewHandleImpl(n);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int DSSetHandleSize(void* h, size_t n)
    {
        return DSSetHandleSizeImpl(h, n);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    long DSDisposeHandle(void* h)
    {
        return DSDisposeHandleImpl(h);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void SetLVString(LStrHandle* lvString, std::string str)
    {
        auto length = str.length();
        auto error = NumericArrayResize(0x01, 1, lvString, length);
        memcpy((**lvString)->str, str.c_str(), length);
        (**lvString)->cnt = (int)length;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::string GetLVString(LStrHandle lvString)
    {
        if (lvString == nullptr || *lvString == nullptr)
        {
            return std::string();
        }

        auto count = (*lvString)->cnt;
        auto chars = (*lvString)->str;

        std::string result(chars, count);
        return result;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int32_t RegisterCleanupProc(CleanupProcPtr cleanUpProc, gRPCid* id)
    {
        return RTSetCleanupProc(cleanUpProc, id, kCleanOnIdle);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int32_t DeregisterCleanupProc(CleanupProcPtr cleanUpProc, gRPCid* id)
    {
        return RTSetCleanupProc(cleanUpProc, id, kCleanOnRemove);
    }

    int AlignClusterOffset(int clusterOffset, int alignmentRequirement)
    {
#ifndef _PS_4
        int remainder = abs(clusterOffset) % alignmentRequirement;
        if (remainder == 0)
        {
            return clusterOffset;
        }
        return clusterOffset + alignmentRequirement - remainder;
#else
        return clusterOffset;
#endif
    }
  
    int32_t GetTypeCodeForSize(int byteSize)
    {
        switch (byteSize)
        {
        case 1:
            return 0x5; // uB
        case 2:
            return 0x6; // uW
        case 4:
            return 0x7; // uL
        case 8:
        default:
            return 0x8; // uQ
        }
    }
}
