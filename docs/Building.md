# Build Instructions

## Building the Scripting and Server binaries

This section covers building the binaries for various operation systems.
You only need to build the binaries if you wish to contribute to the C++ source or if you need binaries for a OS that is not supported by default.

### Build Status
![Linux Build](https://github.com/ni/grpc-labview/workflows/Linux%20Build/badge.svg)
![Windows x64 build](https://github.com/ni/grpc-labview/workflows/Windows%20x64%20build/badge.svg)
![Windows x86 build](https://github.com/ni/grpc-labview/workflows/Windows%20x86%20build/badge.svg)

### Building on Windows

#### Prerequisites
To prepare for cmake + Microsoft Visual C++ compiler build
- Install Visual Studio 2019 (Visual C++ compiler will be used).
- Install [Git](https://git-scm.com/).
- Install [CMake](https://cmake.org/download/).
- Install [Python 3.7 or Higher](https://www.python.org/downloads/).

#### Building 64-bit

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
#### Building 32-bit

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

### Building on Linux

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
> cmake -DCMAKE_BUILD_TYPE=Debug ../..
> make
```

Build Release

```
> mkdir -p cmake/build
> cd cmake/build
> cmake -DCMAKE_BUILD_TYPE=Release ../..
> make
```

### Building on Linux RT

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
> cmake -DCMAKE_BUILD_TYPE=Debug ../..
> make
```

Build Release

```
> mkdir -p cmake/build
> cd cmake/build
> cmake -DCMAKE_BUILD_TYPE=Release ../..
> make
```

## Building the VIP files for LV Support, LV Servicer and Client and Server templates

This section covers building the VIP files from the labview source so that you can install them for a supported version of LabVIEW (2019 or Higher).

You would need to build the VIP files if you make changes to the LabVIEW pieces of the grpc-labview repository.

** Building of VIP files using the instructions below is only supported on Windows **

### Prerequisites

- The codebase is maintained in LabVIEW 2019 for backwards compatibility
   - To contribute, Install LabVIEW 2019
   - Otherwise any LabVIEW 2019 or newer is sufficient to build
   - Both bitnesses are supported.  64-bit is modern and recommended. 32-bit is the default to support the CI system.
- Install LabVIEW CLI
- Install JKI VI Package Manager. You would need Community or Pro edition to build VIP files.
- Install VI Package Manager API for LabVIEW. (Download [location](https://www.ni.com/en-in/support/downloads/tools-network/download.vi-package-manager-api.html#374501))
- Install Python 3.7 or Higher

** Apart from these you would need the Prerequisites for building the binaries as well if you use the --buildcpp option described below. **

### Building VIP files

Download the repo. Run the command below to build the VIP files.

```
> python build-it\build.py --target <BUILD_TARGET> [--pathToBinaries <PATH_TO_PREBUILT_BINARIES>] [--buildcpp] [--labview-version <VERSION>] [--labview-bits <32|64>] [--labview-port <PORT>]
```

The accepted vaues for BUILD_TARGET are:
- Win32
- Win64
- All

If you choose to build for Win32 or Win64 target you can specify the *--buildcpp* option to indicate that you want to build the C++ binaries first and then use them to build VIP files. If you don't specify this option it is assumed that the required binaries are already built accoriding to the steps described above.

Optional parameters:
- *--labview-version*: Specify the LabVIEW version by year using either 2 or 4 digits (e.g., 19, 2019, 2023). Default is "2019".
- *--labview-bits*: Specify either 64 or 32 as the "bitness" of the LabVIEW to use for the building. Default is "32".
- *--labview-port*: Specify the LabVIEW port number. Default is "Auto".  When this is "Auto", the port number will be read from the matching LAbVIEW's .INI file.  Specify a value to override.

If you choose to build "All" target then you need to specify a folder where we can find the pre built binaries for all the supported targets using the *--pathToBinaries** option. The --buildcpp option would be ignored in this mode even if specified.  We expect the folder specified to have the following structure.

```
- <TOP_LEVEL_FOLDER>
    - LabVIEW gRPC Server
        - Libraries
            - Win32
            - Win64
            - Linux
            - LinuxRT
    - LabVIEW gRPC Generator
        - Libraries
            - Win32
            - Win64
            - Linux
```
