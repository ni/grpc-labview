from concurrent import futures

import grpc
import sibling_oneof_pb2 
import sibling_oneof_pb2_grpc

class GreeterService(sibling_oneof_pb2_grpc.GreeterServiceServicer):
    def GetFeature(self, request, context):
        response = sibling_oneof_pb2.res(res_latitude=request.req_latitude+1)
        if request.HasField('req_id'):
            response.res_id = request.req_id + 1
        if request.HasField('req_name'):
            response.res_name = request.req_name + "_response"
        response.message_a.data_a1 = request.message_a.data_a1 + "_response"
        return response
	  
def server():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=2))
    sibling_oneof_pb2_grpc.add_GreeterServiceServicer_to_server(GreeterService(), server)
    server.add_insecure_port('[::]:50051')
    print("gRPC starting")
    server.start()
    server.wait_for_termination()

if __name__ == "__main__":
    server()