from concurrent import futures

import grpc
import helloworld_pb2
import helloworld_pb2_grpc

class Greeter(helloworld_pb2_grpc.GreeterServiceServicer):
    # Sends a greeting by appending 'Hello ' prefix and "!!!" suffix to the name field of HelloRequest message
    def SayHello(self, request, context):
        print("Got request " + str(request))
        return helloworld_pb2.HelloReply(message=f'Hello {request.name}!!!')
	  
def server():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=2))
    helloworld_pb2_grpc.add_GreeterServiceServicer_to_server(Greeter(), server)
    server.add_insecure_port('[::]:50051')
    print("gRPC starting")
    server.start()
    server.wait_for_termination()

if __name__ == "__main__":
    server()