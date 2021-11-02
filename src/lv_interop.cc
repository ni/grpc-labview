//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <lv_interop.h>
#include <iostream>
#include <cstring>
#include <memory>

#ifndef _WIN32
#include <dlfcn.h>
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
typedef int (*NumericArrayResize_T)(int32_t, int32_t, void* handle, size_t size);
typedef int (*PostLVUserEvent_T)(grpc_labview::LVUserEventRef ref, void *data);
typedef int (*Occur_T)(grpc_labview::MagicCookie occurrence);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static NumericArrayResize_T NumericArrayResizeImp = nullptr;
static PostLVUserEvent_T PostLVUserEvent = nullptr;
static Occur_T Occur = nullptr;

namespace grpc_labview
{

#ifdef _WIN32

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void InitCallbacks()
    {
        if (NumericArrayResizeImp != nullptr)
        {
            return;
        }

        auto lvModule = GetModuleHandle("LabVIEW.exe");
        if (lvModule == nullptr)
        {
            lvModule = GetModuleHandle("lvffrt.dll");
        }
        if (lvModule == nullptr)
        {
            lvModule = GetModuleHandle("lvrt.dll");
        }
        NumericArrayResizeImp = (NumericArrayResize_T)GetProcAddress(lvModule, "NumericArrayResize");
        PostLVUserEvent = (PostLVUserEvent_T)GetProcAddress(lvModule, "PostLVUserEvent");
        Occur = (Occur_T)GetProcAddress(lvModule, "Occur");
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

        auto lvModule = dlopen(nullptr, RTLD_LAZY);
        if (lvModule != nullptr)
        {
            NumericArrayResizeImp = (NumericArrayResize_T)dlsym(lvModule, "NumericArrayResize");
            PostLVUserEvent = (PostLVUserEvent_T)dlsym(lvModule, "PostLVUserEvent");
            Occur = (Occur_T)dlsym(lvModule, "Occur");
        }
        if (NumericArrayResize == nullptr)
        {
            std::cout << "Loading LabVIEW Runtime engine!" << std::endl;
            lvModule = dlopen("liblvrt.so", RTLD_NOW);
            NumericArrayResizeImp = (NumericArrayResize_T)dlsym(lvModule, "NumericArrayResize");
            PostLVUserEvent = (PostLVUserEvent_T)dlsym(lvModule, "PostLVUserEvent");
            Occur = (Occur_T)dlsym(lvModule, "Occur");
        }
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
    int PostUserEvent(LVUserEventRef ref, void *data)
    {
        return PostLVUserEvent(ref, data);    
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
}
