#
# Example communication with LabVIEW Querey Server over gRPC
#
# Copyright 2021 National Instruments
# Licensed under the MIT license
#
# Getting Started:
#
# Install the gRPC tools for Python
#     > pip install grpcio-tools
#   if you are using anaconda
#     > conda install grpcio-tools
#
# Generate the python API from the gRPC definition (.ptoto) files
#   > python -m grpc_tools.protoc -I.. --python_out=. --grpc_python_out=. ../query_server.proto
#
# Update the server address in this file to match where the LabVIEW server is running
#

import grpc
import query_server_pb2 as queryTypes
import query_server_pb2_grpc as gRPCQueryServer

# This is the location (ipaddress or machine name):(port) of the niDevice server
serverAddress = "localhost:50051"

# Create the communcation channel for the remote host (in this case we are connecting to a local server)
# and create a connection to the niScope service
channel = grpc.insecure_channel(serverAddress)
queryServer = gRPCQueryServer.QueryServerStub(channel)

# Get the server uptime
uptime = queryServer.Query(queryTypes.QueryRequest(
    query = "Uptime"
    ))

print("Server Uptime: " + uptime.message)

reader = queryServer.Register(queryTypes.RegistrationRequest(
    eventName = "Heartbeat"
    ))

for x in range(0, 5):
    event = reader.next()
    print ("Server Event: " + event.eventData)

queryServer.Invoke(queryTypes.InvokeRequest(
    command = "Reset"
))

print("Done")