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

Download the repo and update submodules, this will pull the gRPC components and all dependencies

```
> git clone https://github.com/ni/labview-grpc-query-server.git labview-grpc-query-server
> cd labview-grpc-query-server
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
> cmake .
> cmake --build . --Config Release
```

## Building on Linux

Download the repo and update submodules, this will pull the gRPC components and all dependencies

```
> git clone https://github.com/ni/labview-grpc-query-server.git labview-grpc-query-server
> cd labview-grpc-query-server
> git submodule update --init --recursive
```
Build

```
> cmake .
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
> git clone https://github.com/ni/labview-grpc-query-server.git labview-grpc-query-server
> cd labview-grpc-query-server
> git submodule update --init --recursive
```

Build

```
> cmake .
> make
```

## Creating a LabVIEW Server

Copy the QueryServer_Template.vi to a new name of your choosing.  RPC methods are handled by LabVIEW user events.
The template consumes all of the events and processes them in the event case.  You can choose to handle the events in
any manner of your choosing.

## Using the LabVIEW Client API

Coming Soon

## Example

There is an example server ExampleQueryServer.vi and a corresponding C++ client (example_client).
When you build the C++ libraries the example client will also be built.

## SSL/TLS Support

You can enable SSL/TLS support on the server by passing in a path to a certificate and private key for the server.

You can generate the certificate with openssl using the following script.

```
mypass="password123"

echo Generate server key:
openssl genrsa -passout pass:$mypass -des3 -out server.key 4096

echo Generate server signing request:
openssl req -passin pass:$mypass -new -key server.key -out server.csr -subj  "/C=US/ST=TX/L=Austin/O=NI/OU=labview/CN=localhost"

echo Self-sign server certificate:
openssl x509 -req -passin pass:$mypass -days 365 -in server.csr -signkey server.key -set_serial 01 -out server.crt

echo Remove passphrase from server key:
openssl rsa -passin pass:$mypass -in server.key -out server.key

rm server.csr
```

Clients then must connect using the server certificate that was generated (server.cer) otherwise the connection will fail.

If you do not passing in a certificate then the server will use insecure gRPC.