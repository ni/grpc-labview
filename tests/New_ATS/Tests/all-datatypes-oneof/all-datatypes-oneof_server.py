from concurrent import futures

import grpc
import all_datatypes_oneof_pb2
import all_datatypes_oneof_pb2_grpc

class GreeterService(all_datatypes_oneof_pb2_grpc.GreeterServiceServicer):
    def GetFeature(self, request, context):
        response = all_datatypes_oneof_pb2.res(res_latitude=request.req_latitude+1)
        if request.HasField('req_int32'):
            response.res_int32 = request.req_int32 + 1
        if request.HasField('req_int64'):
            response.res_int64 = request.req_int64 + 1
        if request.HasField('req_uint32'):
            response.res_uint32 = request.req_uint32 + 1
        if request.HasField('req_uint64'):
            response.res_uint64 = request.req_uint64 + 1
        if request.HasField('req_sint32'):
            response.res_sint32 = request.req_sint32 + 1
        if request.HasField('req_sint64'):
            response.res_sint64 = request.req_sint64 + 1
        if request.HasField('req_fixed32'):
            response.res_fixed32 = request.req_fixed32 + 1
        if request.HasField('req_fixed64'):
            response.res_fixed64 = request.req_fixed64 + 1
        if request.HasField('req_sfixed32'):
            response.res_sfixed32 = request.req_sfixed32 + 1
        if request.HasField('req_sfixed64'):
            response.res_sfixed64 = request.req_sfixed64 + 1
        if request.HasField('req_float'):
            response.res_float = request.req_float + 1
        if request.HasField('req_double'):
            response.res_double = request.req_double + 1
        if request.HasField('req_bool'):
            response.res_bool = False if request.req_bool else True
        if request.HasField('req_string'):
            response.res_string = request.req_string + "_response"
        if request.HasField('req_bytes'):
            response.res_bytes = request.req_bytes
        return response
	  
def server():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=2))
    all_datatypes_oneof_pb2_grpc.add_GreeterServiceServicer_to_server(GreeterService(), server)
    server.add_insecure_port('[::]:50051')
    print("gRPC starting")
    server.start()
    server.wait_for_termination()

if __name__ == "__main__":
    server()