# Quick Start

This guide gets you started with gRPC in LabVIEW with a simple working example.

## Prerequisites

* LabVIEW 2019 or higher
* VIPM 2020 or higher

## gRPC

To install gRPC:

1. Download the latest package from [Releases](https://github.com/ni/grpc-labview/releases).
    1. Download and Unzip `grpc-labview.zip` which contains the released packages.
2. Install the following gRPC package,
    1. `ni_lib_labview_grpc_library-x.x.x.x.vip`

## gRPC Server & Client tools:

LabVIEW gRPC tools include scripting tool to generate the server and client code from '.proto' service definitions. For the first part of our quick-start example, we have already generated the server and client code from [helloworld.proto](https://github.com/grpc/grpc/blob/v1.42.0/examples/protos/helloworld.proto), but you will need the tools for the rest of our quick start, as well as for your own project.

To install gRPC tools:

Note: Continue after 'installing gRPC'

1. Install the following packages,
    1. `ni_lib_labview_grpc_servicer-x.x.x.x.vip`
    2. `ni_lib_grpc_server_and_client_template[2]-x.x.x.x.vip`

## Download and Run the example

To download the example:

You will need a local copy of the example code to work through this quick start. Download the example code from our GitHub repository (the following command clones the entire repository, but you just need the examples for this quick start and other tutorials):

```
*# clone the repository to get the example code:*
$ git clone https://github.com/ni/grpc-labview.git
*# Navigate to the "hello, world" LabVIEW example using file explorer:*
../grpc-labview/examples/helloworld/
```

To run the server:

