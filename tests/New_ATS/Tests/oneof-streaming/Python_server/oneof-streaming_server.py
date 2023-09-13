import grpc
from concurrent import futures
import time
import oneof_streaming_pb2
import oneof_streaming_pb2_grpc

class GreeterService(oneof_streaming_pb2_grpc.GreeterServiceServicer):

    # 'UnaryMethod' rpc performs the following operations on the 'Request' message fields:
    # 1) Increments age by 1 and saves in new_age of 'Response' message.
    # 2) Increments request_id of request_oneof by 1 and saves in response_oneof's response_id field of 'Response' message. 
    # 3) Appends '_response' to the message field of request_oneof and saves in result field of response_oneof of 'Response' message.
    def UnaryMethod(self, request, context):
        age = request.age
        message = request_id = None
        if request.HasField("message"):
            message = request.message
        if request.HasField("request_id"):
            request_id = request.request_id
        new_age = age + 1
        result = f"{message}_response" if message else None
        response = oneof_streaming_pb2.Response(new_age=new_age, result=result, response_id=request_id)
        return response
    

    # 'ServerStreamingMethod' rpc performs the following operations on the 'Request' message fields and streams 'age' number of identical 'Response' messages:
    # 1) Increments age by 1 and saves in new_age of 'Response' message.
    # 2) Increments request_id of request_oneof by 1 and saves in response_oneof's response_id field of 'Response' message. 
    # 3) Appends '_response' to the message field of request_oneof and saves in result field of response_oneof of 'Response' message.
    def ServerStreamingMethod(self, request, context):
        age = request.age
        message = request_id = None
        if request.HasField("message"):
            message = request.message
        if request.HasField("request_id"):
            request_id = request.request_id

        for _ in range(age):
            new_age = age + 1
            result = f"{message}_response" if message else None
            response_id = request_id+1 if request_id else None
            response = oneof_streaming_pb2.Response(new_age=new_age, result=result, response_id=response_id)
            yield response 


    # 'ClientStreamingMethod' rpc performs the following operations on the 'Request' message fields returns one single 'Response' message:
    # 1) Sums up the values of age field in all 'Request' messages and saves the sum in new_age of 'Response' message.
    # 2) Increments request_id of request_oneof of the last 'Request' message by 1 and saves in response_oneof's response_id field of 'Response' message. 
    # 3) Appends '_response' to the message field of request_oneof of the last 'Request' message and saves in result field of response_oneof of 'Response' message.
    def ClientStreamingMethod(self, request_iterator, context):
        age_sum = 0
        last_message = None

        for request in request_iterator:
            age_sum += request.age
            last_message = request
        
        new_age=age_sum
        message = request_id = None
        if last_message.HasField("message"):
            message = last_message.message
        if last_message.HasField("request_id"):
            request_id = last_message.request_id
        
        result = f"{message}_response" if message else None
        response_id = request_id+1 if request_id else None

        response = oneof_streaming_pb2.Response(new_age=new_age, result=result, response_id=response_id)
        return response
    
    
    # 'BidirectionalStreamingMethod' rpc performs the following operations on the 'Request' message fields and returns one 'Response' message for each 'Request' message:
    # 1) Increments age by 1 and saves in new_age of 'Response' message.
    # 2) Increments request_id of request_oneof by 1 and saves in response_oneof's response_id field of 'Response' message. 
    # 3) Appends '_response' to the message field of request_oneof and saves in result field of response_oneof of 'Response' message.
    def BidirectionalStreamingMethod(self, request_iterator, context):
        for request in request_iterator:
            new_age = request.age + 1
            message = request_id = None
            if request.HasField("message"):
                message = request.message
            if request.HasField("request_id"):
                request_id = request.request_id
        
            result = f"{message}_response" if message else None
            response_id = request_id+1 if request_id else None

            response = oneof_streaming_pb2.Response(new_age=new_age, result=result, response_id=response_id)
            yield response

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=2))
    oneof_streaming_pb2_grpc.add_GreeterServiceServicer_to_server(GreeterService(), server)
    server.add_insecure_port('[::]:50051')
    print("Server started on port 50051")
    server.start()
    server.wait_for_termination()

if __name__ == '__main__':
    serve()
