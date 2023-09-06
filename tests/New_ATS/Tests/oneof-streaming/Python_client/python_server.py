import grpc
from concurrent import futures
import time
import oneof_streaming_pb2
import oneof_streaming_pb2_grpc

class GreeterService(oneof_streaming_pb2_grpc.GreeterServiceServicer):
    def UnaryMethod(self, request, context):
        age = request.age
        message = request.request_oneof.message
        request_id = request.request_oneof.request_id
        new_age = age + 1
        result = f"{message}_response" if message else None
        response = oneof_streaming_pb2.Response(new_age=new_age, result=result, response_id=request_id)
        return response
    
    # def UnaryMethod(self, request, context):
    #     return super().UnaryMethod(request, context)

    # def ServerStreamingMethod(self, request, context):
    #     age = request.age
    #     message = request.request_oneof.message

    #     for _ in range(age):
    #         new_age = age + 1
    #         result = f"{message}_response" if message else None
    #         response_id = request_id+1 if request_id else None
    #         request_id = request.request_oneof.request_id
    #         response = oneof_streaming_pb2.Response(new_age=new_age, result=result, response_id=response_id)
    #         yield response  # Corrected here

    def ServerStreamingMethod(self, request, context):
        return super().ServerStreamingMethod(request, context)

    def ClientStreamingMethod(self, request_iterator, context):
        return super().ClientStreamingMethod(request_iterator, context)
    
    def BidirectionalStreamingMethod(self, request_iterator, context):
        return super().BidirectionalStreamingMethod(request_iterator, context)

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=2))
    oneof_streaming_pb2_grpc.add_GreeterServiceServicer_to_server(GreeterService(), server)
    server.add_insecure_port('[::]:50051')
    print("Server started on port 50051")
    server.start()
    server.wait_for_termination()

if __name__ == '__main__':
    serve()
