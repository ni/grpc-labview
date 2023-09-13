import grpc
import all_datatypes_oneof_pb2
import all_datatypes_oneof_pb2_grpc
import json
import pytest
import os

# 'GetFeature' rpc performs the following operations on the 'req' message fields:
# 1) Increments req_latitude by 1 and saves in res_latitude of 'res' message.
# 2) Increments all the integer and decimal fields of req_oneof by 1 and saves in corresponding res_oneof fields of 'res' message. 
# 3) Appends '_response' to the string fields of req_oneof and saves in corresponding res_oneof fields of 'res' message.
# 4) Does a NOT operation on the boolean fields of req_oneof and saves in corresponding res_oneof fields of 'res' message. 
# 5) Doesn't do any operation on the bytes fields of req_oneof and saves in corresponding res_oneof fields of 'res' message.
def get_GetFeature_output(test_input):
    
    req_latitude = test_input.get('req_latitude')
    req_int32 = test_input['req_oneof'].get('req_int32')
    req_int64 = test_input['req_oneof'].get('req_int64')
    req_uint32 = test_input['req_oneof'].get('req_uint32')
    req_uint64 = test_input['req_oneof'].get('req_uint64')
    req_sint32 = test_input['req_oneof'].get('req_sint32')
    req_sint64 = test_input['req_oneof'].get('req_sint64')
    req_fixed32 = test_input['req_oneof'].get('req_fixed32')
    req_fixed64 = test_input['req_oneof'].get('req_fixed64')
    req_sfixed32 = test_input['req_oneof'].get('req_sfixed32')
    req_sfixed64 = test_input['req_oneof'].get('req_sfixed64')
    req_float = test_input['req_oneof'].get('req_float')
    req_double = test_input['req_oneof'].get('req_double')
    req_bool = test_input['req_oneof'].get('req_bool')
    req_string = test_input['req_oneof'].get('req_string')
    req_bytes = test_input['req_oneof'].get('req_bytes')
    if req_bytes is not None:
        req_bytes=req_bytes.encode('utf-8')

    with grpc.insecure_channel('localhost:50051') as channel:
        stub = all_datatypes_oneof_pb2_grpc.GreeterServiceStub(channel)
        request = all_datatypes_oneof_pb2.req(
            req_latitude=req_latitude,
            req_int32=req_int32,
            req_int64=req_int64,
            req_uint32=req_uint32,
            req_uint64=req_uint64,
            req_sint32=req_sint32,
            req_sint64=req_sint64,
            req_fixed32=req_fixed32,
            req_fixed64=req_fixed64,
            req_sfixed32=req_sfixed32,
            req_sfixed64=req_sfixed64,
            req_double=req_double,
            req_float=req_float,
            req_bool=req_bool,
            req_string=req_string,
            req_bytes=req_bytes
        )
        response = stub.GetFeature(request)

    response_dict = {}
    response_dict['res_latitude'] = response.res_latitude
    response_dict['res_oneof'] = {}
    if(response.HasField('res_int32')):
        response_dict['res_oneof']['res_int32'] = response.res_int32
    if(response.HasField('res_int64')):
        response_dict['res_oneof']['res_int64'] = response.res_int64
    if(response.HasField('res_uint32')):
        response_dict['res_oneof']['res_uint32'] = response.res_uint32
    if(response.HasField('res_uint64')):
        response_dict['res_oneof']['res_uint64'] = response.res_uint64
    if(response.HasField('res_sint32')):
        response_dict['res_oneof']['res_sint32'] = response.res_sint32
    if(response.HasField('res_sint64')):
        response_dict['res_oneof']['res_sint64'] = response.res_sint64
    if(response.HasField('res_fixed32')):
        response_dict['res_oneof']['res_fixed32'] = response.res_fixed32
    if(response.HasField('res_fixed64')):
        response_dict['res_oneof']['res_fixed64'] = response.res_fixed64
    if(response.HasField('res_sfixed32')):
        response_dict['res_oneof']['res_sfixed32'] = response.res_sfixed32
    if(response.HasField('res_sfixed64')):
        response_dict['res_oneof']['res_sfixed64'] = response.res_sfixed64
    if(response.HasField('res_float')):
        response_dict['res_oneof']['res_float'] = response.res_float
    if(response.HasField('res_double')):
        response_dict['res_oneof']['res_double'] = response.res_double
    if(response.HasField('res_bool')):
        response_dict['res_oneof']['res_bool'] = response.res_bool
    if(response.HasField('res_string')):
        response_dict['res_oneof']['res_string'] = response.res_string
    if(response.HasField('res_bytes')):
        response_dict['res_oneof']['res_bytes'] = response.res_bytes.decode('utf-8')
    return(response_dict)

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

GetFeature_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/GetFeature.json'

@pytest.mark.parametrize('testcase', read_json(GetFeature_json_file_path))
def test_SayHello(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_GetFeature_output(test_input) == expected

if __name__ == "__main__":
    print(get_GetFeature_output({"req_latitude":100, "req_oneof":{"req_string":"wow"}}))