# Building the Scripting and Server binaries

This document covers building the binaries for various operation systems.
You only need to build the binaries if you wish to contribute to the C++ source or if you need binaries for a OS that is not supported by default.

## Build Status
![Linux Build](https://github.com/ni/grpc-labview/workflows/Linux%20Build/badge.svg)
![Windows x64 build](https://github.com/ni/grpc-labview/workflows/Windows%20x64%20build/badge.svg)
![Windows x86 build](https://github.com/ni/grpc-labview/workflows/Windows%20x86%20build/badge.svg)

## Building on Windows

### Prerequisites
To prepare for cmake + Microsoft Visual C++ compiler build
- Install LabVIEW 2019
- Install Visual Studio 2015, 2017, or 2019 (Visual C++ compiler will be used).
- Install [Git](https://git-scm.com/).
- Install [CMake](https://cmake.org/download/).


### Building 64-bit

**Launch "x64 Native Tools Command Prompt for Visual Studio"**

Download the repo and update submodules, this will pull the gRPC components and all dependencies

```
> git clone https://github.com/ni/grpc-labview.git grpc-labview
> cd grpc-labview
> git submodule update --init --recursive
```

Build Debug
```
> mkdir build
> cd build
> cmake ..
> cmake --build .
```

Build Release
```
> mkdir build
> cd build
> cmake ..
> cmake --build . --config Release
```
### Building 32-bit

**Launch "x86 Native Tools Command Prompt for Visual Studio"**

Download the repo and update submodules, this will pull the gRPC components and all dependencies

```
> git clone https://github.com/ni/grpc-labview.git grpc-labview
> cd grpc-labview
> git submodule update --init --recursive
```

Build Debug
```
> mkdir build
> cd build
> cmake -A Win32 ..
> cmake --build .
```

Build Release
```
> mkdir build
> cd build
> cmake -A Win32 ..
> cmake --build . --config Release
```

## Building on Linux

Download the repo and update submodules, this will pull the gRPC components and all dependencies

```
> git clone https://github.com/ni/grpc-labview.git grpc-labview
> cd grpc-labview
> git submodule update --init --recursive
```

Build Debug

```
> mkdir -p cmake/build
> cd cmake/build
> cmake ../..
> make
```

Build Release

```
> mkdir -p cmake/build
> cd cmake/build
> cmake -DCMAKE_BUILD_TYPE=Release ../..
> make
```

## Building on Linux RT

Install required packages not installed by default

```
> opkg update
> opkg install git
> opkg install git-perltools
> opkg install cmake
> opkg install g++
> opkg install g++-symlinks
```

Download the repo and update submodules, this will pull the gRPC components and all dependencies

```
> git clone https://github.com/ni/grpc-labview.git grpc-labview
> cd grpc-labview
> git submodule update --init --recursive
```

Build Debug

```
> mkdir -p cmake/build
> cd cmake/build
> cmake ../..
> make
```

Build Release

```
> mkdir -p cmake/build
> cd cmake/build
> cmake -DCMAKE_BUILD_TYPE=Release ../..
> make
```

