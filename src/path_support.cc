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
    // Returns the directory path containing the currently loaded shared library
    //---------------------------------------------------------------------
    std::filesystem::path GetFolderContainingDLL() {
        std::filesystem::path fullPath;

#ifdef _WIN32
        HMODULE hModule = nullptr;
        if (GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&GetFolderContainingDLL), &hModule))
        {
            // Fetch the path of the DLL, increasing the buffer size in case the path is a long path
            std::string path(MAX_PATH, '\0');
            DWORD result = GetModuleFileNameA(hModule, path.data(), path.size());
            while (result >= path.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                path.resize(path.size() * 2);
                result = GetModuleFileNameA(hModule, path.data(), path.size());
            }
            path.resize(result); // remove padding at end
            if (result) {
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

        return fullPath;
    }
}
