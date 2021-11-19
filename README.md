# gRPC Support for LabVIEW

This repo contains necessary C++ code and support VIs to implement a gRPC server in LabVIEW.

Open `examples/query_server/Query Server.lvproj` for examples on creating a query server. `/examples/query_server/Protos/query_server.proto` defines a simple query service example that can be used for a variety of purposes.

You can either use the service as defined to implement a generic server via gPRC or use the implementation
as a pattern to implement a gRPC service of your design.

The project supports Windows, Linux, and Linux RT.

## Minimum Compatible LabVIEW Version

LabVIEW source is saved with __LabVIEW 2019__.

## Note: This project is not yet complete
* Not all .proto data types are supported
* The VI generated has not yet been implemneted - VIs need to be implemented by hand to match the .proto file
* Extensive testing is not complete
* The names of the various generated methods, events, and VIs are subject to change until the 1.0 release
* This project is not supported by NI Technical Support

## Creating a LabVIEW gRPC Server

Step-by-step examples can be found in the [Creating a Server](docs/ServerCreation.md) guide

### Examples

Example servers are located in the examples foldes in the releases.
* QueryServer - Example server which implements a Query / Invoke / Event API

The examples include a python client that can be used to communicate with the example servers.

## Using the LabVIEW Client API

Coming Soon

## SSL/TLS Support

The connection to the server can be secured using TLS (Server provided certificates) or Multual TLS (Client and Server provided certificates).
See [Certificates](docs/Certificates.md) for more information.

## Building the server binaries
To build the binaries for the scripting tool or the gRPC server see [Building](docs/Building.md) for instructions.

## Contributing
Contributions to grpc-labview are welcome from all. See [Contributing](CONTRIBUTING.md) for instructions.

