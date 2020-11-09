//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <lv_interop.h>
#include <memory>

#ifndef _WIN32
#include <dlfcn.h>
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace std;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
typedef int (*NumericArrayResize_T)(int32_t, int32_t, void* handle, size_t size);
typedef int (*PostLVUserEvent_T)(LVUserEventRef ref, void *data);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static NumericArrayResize_T NumericArrayResize;
static PostLVUserEvent_T PostLVUserEvent;

#ifdef _WIN32

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void InitCallbacks()
{
    if (NumericArrayResize != NULL)
    {
        return;
    }

    auto lvModule = GetModuleHandle("LabVIEW.exe");
    if (lvModule == NULL)
    {
        lvModule = GetModuleHandle("lvffrt.dll");
    }
    if (lvModule == NULL)
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
    if (NumericArrayResize != NULL)
    {
        return;
    }

    auto lvModule = dlopen("labview", RTLD_NOLOAD);
    if (lvModule == NULL)
    {
        lvModule = dlopen("liblvrt.so", RTLD_NOW);
    }
    NumericArrayResize = (NumericArrayResize_T)dlsym(lvModule, "NumericArrayResize");
    PostLVUserEvent = (PostLVUserEvent_T)dlsym(lvModule, "PostLVUserEvent");
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
void SetLVString(LStrHandle* lvString, string str)
{
    auto length = str.length();    
    auto error = NumericArrayResize(0x01, 1, lvString, length);
    memcpy((**lvString)->str, str.c_str(), length);
    (**lvString)->cnt = (int)length;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
string GetLVString(LStrHandle lvString)
{    
    if (lvString == NULL || *lvString == NULL)
    {
        return string();
    }

    auto count = (*lvString)->cnt;
    auto chars = (*lvString)->str;

    string result(chars, count);
    return result;
}
