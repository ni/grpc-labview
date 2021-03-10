#
# Example communication with example query server over gRPC
#
# Copyright 2021 National Instruments
# Licensed under the MIT license
#
# This example uses the "betterproto" protocol buffers / gRPC library
#   betterproto produces a more idiomatic version of the gRPC API
#   for more information see: https://github.com/danielgtaylor/python-betterproto
#
# Getting Started:
#
# Install the gRPC tools for Python
#     > pip install grpcio-tools
#   if you are using anaconda
#     > conda install grpcio-tools
#
# Install the betterproto tools
#   > pip install "betterproto[compiler]"
#
# Generate the python API from the gRPC definition (.ptoto) files
#   > python -m grpc_tools.protoc -I.. --python_betterproto_out=. --grpc_python_out=. ../query_server.proto
#
# Update the server address to match the location of the query server server address
#

import queryserver
import asyncio
from grpclib.client import Channel

async def talk_to_server():

    # Create the communcation channel for the remote host (in this case we are connecting to a local server)
    # and create a connection to the niScope service
    channel = Channel(host="localhost", port=50051)
    queryServer = queryserver.QueryServerStub(channel)

    uptime = await queryServer.query(query="Uptime")
    print("Server Uptime: " + uptime.message)

    reader = queryServer.register(event_name="Heartbeat")
    count = 0
    async for event in reader:
        print ("Server Event: " + event.event_data)
        count += 1
        if count == 5:
            break

    await queryServer.invoke(command = "Reset")

    print("Done")
    channel.close()

loop = asyncio.get_event_loop()
asyncio.run(talk_to_server())
