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

typedef struct {
    int32_t cnt; /* number of bytes that follow */
    char str[1]; /* cnt bytes */
} LStr, * LStrPtr, ** LStrHandle;

typedef struct {
	int32_t cnt; /* number of bytes that follow */
    int32_t padding;
	int8_t str[1]; /* cnt bytes */
} LV1DArray, * LV1DArrayPtr, ** LV1DArrayHandle;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void InitCallbacks();
void SetLVString(LStrHandle* lvString, std::string str);
std::string GetLVString(LStrHandle lvString);
int LVNumericArrayResize(int32_t typeCode, int32_t numDims, void* handle, size_t size);
int LVPostLVUserEvent(LVUserEventRef ref, void *data);