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
typedef int (*PostLVUserEvent_T)(LVUserEventRef ref, void *data);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static NumericArrayResize_T NumericArrayResize = nullptr;
static PostLVUserEvent_T PostLVUserEvent = nullptr;

#ifdef _WIN32

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void InitCallbacks()
{
    if (NumericArrayResize != nullptr)
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
    NumericArrayResize = (NumericArrayResize_T)GetProcAddress(lvModule, "NumericArrayResize");
    PostLVUserEvent = (PostLVUserEvent_T)GetProcAddress(lvModule, "PostLVUserEvent");
}

#else

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void InitCallbacks()
{
    if (NumericArrayResize != nullptr)
    {
        return;
    }

    auto lvModule = dlopen(nullptr, RTLD_LAZY);
    if (lvModule != nullptr)
    {
        NumericArrayResize = (NumericArrayResize_T)dlsym(lvModule, "NumericArrayResize");
        PostLVUserEvent = (PostLVUserEvent_T)dlsym(lvModule, "PostLVUserEvent");
    }
    if (NumericArrayResize == nullptr)
    {
        std::cout << "Loading LabVIEW Runtime engine!" << std::endl;
        lvModule = dlopen("liblvrt.so", RTLD_NOW);
        NumericArrayResize = (NumericArrayResize_T)dlsym(lvModule, "NumericArrayResize");
        PostLVUserEvent = (PostLVUserEvent_T)dlsym(lvModule, "PostLVUserEvent");
    }
}

#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int LVNumericArrayResize(int32_t typeCode, int32_t numDims, void* handle, size_t size)
{    
    return NumericArrayResize(typeCode, numDims, handle, size);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int LVPostLVUserEvent(LVUserEventRef ref, void *data)
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
