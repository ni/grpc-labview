#include <string>
#include <algorithm>
#include <filesystem>

#include "path_support.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#include <dlfcn.h>
#include <libgen.h>
#endif


namespace grpc_labview
{
    //---------------------------------------------------------------------
    // Returns a string of the path containing this shared library
    //---------------------------------------------------------------------
    std::string GetFolderContainingDLL() {
        std::filesystem::path fullPath;

#ifdef _WIN32
        HMODULE hModule = nullptr;
        if (GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&GetFolderContainingDLL), &hModule))
        {
            char path[MAX_PATH];
            if (GetModuleFileNameA(hModule, path, MAX_PATH)) {
                fullPath = std::filesystem::path(path).parent_path();
            }
        }

#else
        Dl_info dl_info;
        if (dladdr(reinterpret_cast<void*>(&GetFolderContainingDLL), &dl_info) && dl_info.dli_fname) {
            char realPath[PATH_MAX];
            if (realpath(dl_info.dli_fname, realPath)) {
                fullPath = std::filesystem::path(realPath).parent_path();
            }
        }
#endif

        return fullPath.string();
    }
}
