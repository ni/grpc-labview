# Example gRPC Service for LabVIEW

This repo contains an example / pattern to use to implement a gRPC server / client for LabVIEW.
query_server.proto defines a simple query service that can be used for a variety of purposes.  

You can either use the service as defined to implement a generic server via gPRC or use the implementation
as a pattern to implement a gRPC service of your design.

the project support Windows and Linux for both the client and server.

## Building on Windows

### Prerequisites
To prepare for cmake + Microsoft Visual C++ compiler build
- Install Visual Studio 2015, 2017, or 2019 (Visual C++ compiler will be used).
- Install [Git](https://git-scm.com/).
- Install gRPC for C++
- Install [CMake](https://cmake.org/download/).
- Install LabVIEW 2019
- (Optional) Install [Ninja](https://ninja-build.org/) (`choco install ninja`)


### Building
- Launch "x64 Native Tools Command Prompt for Visual Studio"

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
> cmake .
> cmake --build . --Config Release
```

## Building on Linux
```
> mkdir build
> cd build
> cmake ..
> make
```

## Building on Linux RT

Coming soon

## Creating a LabVIEW Server

Copy the QueryServer_Template.vi to a new name of your choosing.  RPC methods are handled by LabVIEW user events.
The template consumes all of the events and processes them in the event case.  You can choose to handle the events in
any manner of your choosing.

## Using the LabVIEW Client API

## Example

There is an example server ExampleQueryServer.vi and a corresponding C++ client (example_client).
When you build the C++ libraries the example client will also be built.
