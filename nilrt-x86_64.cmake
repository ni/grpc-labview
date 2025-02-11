#----------------------------------------------------------------------
# CMake toolchain file for cross-compiling for NI Linux Real-Time
#----------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(_GRPC_DEVICE_NILRT_LEGACY_TOOLCHAIN TRUE)

#----------------------------------------------------------------------
# Path variables for toolchains
#----------------------------------------------------------------------
find_program(COMPILER_PATH x86_64-nilrt-linux-gcc)
get_filename_component(toolchain_path ${COMPILER_PATH}/../../../../.. REALPATH DIRECTORY)
set(include_path core2-64-nilrt-linux/usr/include/c++/10.0)

#----------------------------------------------------------------------
# Compilers
#----------------------------------------------------------------------
set(CMAKE_C_COMPILER x86_64-nilrt-linux-gcc)
set(CMAKE_CXX_COMPILER x86_64-nilrt-linux-g++)

#----------------------------------------------------------------------
# Default compiler flags
#----------------------------------------------------------------------
set(CMAKE_SYSROOT ${toolchain_path}/core2-64-nilrt-linux)
set(CMAKE_<LANG>_STANDARD_INCLUDE_DIRECTORIES ${toolchain_path}/${include_path} ${toolchain_path}/${include_path}/x86_64-nilrt-linux)
set(CMAKE_<LANG>_FLAGS "-Wall -fmessage-length=0")
set(CMAKE_<LANG>_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_<LANG>_FLAGS_RELEASE "-O3")

#----------------------------------------------------------------------
# Define proper search behavior for cross compilation
#----------------------------------------------------------------------
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)