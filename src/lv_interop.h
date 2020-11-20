//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <string>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef _WIN32
    #define LIBRARY_EXPORT extern "C" __declspec(dllexport)
#else
    #define LIBRARY_EXPORT extern "C"
#endif

//---------------------------------------------------------------------
// LabVIEW definitions
//---------------------------------------------------------------------
typedef int32_t MagicCookie;
typedef MagicCookie LVRefNum;
typedef MagicCookie LVUserEventRef;

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
#ifndef _PS_4
        if (sizeof(T) < 8)
        {
            return (T*)rawBytes;
        }
        return (T*)(rawBytes + 4); // 8-byte aligned data
#else
        return (T*)rawBytes;
#endif
    }

    template<typename T>
    T* bytes(int byteOffset)
    {
#ifndef _PS_4
        if (sizeof(T) < 8)
        {
            return (T*)(rawBytes + byteOffset);
        }
        return (T*)(rawBytes + 4 + byteOffset); // 8-byte aligned data
#else
        return (T*)(rawBytes + byteOffset);
#endif
    }
};

using LV1DArrayPtr = LV1DArray*;
using LV1DArrayHandle = LV1DArray**;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void InitCallbacks();
void SetLVString(LStrHandle* lvString, std::string str);
std::string GetLVString(LStrHandle lvString);
int LVNumericArrayResize(int32_t typeCode, int32_t numDims, void* handle, size_t size);
int LVPostLVUserEvent(LVUserEventRef ref, void *data);