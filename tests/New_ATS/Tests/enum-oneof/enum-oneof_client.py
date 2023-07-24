import grpc
import enum_oneof_pb2
import enum_oneof_pb2_grpc
import json
import pytest
import os

def get_GetFeature_output(test_input):
    req_latitude = int(test_input.get('req_latitude'))
    req_name = test_input['req_oneof'].get('req_name')
    req_color = test_input['req_oneof'].get('req_color')
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = enum_oneof_pb2_grpc.GreeterServiceStub(channel)
        request = enum_oneof_pb2.req(req_latitude=req_latitude, req_name=req_name, req_color=req_color)
        response = stub.GetFeature(request)
    response_dict = {}
    response_dict['res_latitude'] = response.res_latitude
    response_dict['res_oneof'] = {}  
    response_dict['res_oneof']['res_color'] = enum_oneof_pb2.Color(response.res_color) if response.HasField('res_color') else None
    response_dict['res_oneof']['res_name'] = response.res_name if response.HasField('res_name') else None

    return(response_dict)

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

GetFeature_json_file_path = f'{os.path.dirname(os.path.abspath(__file__))}/testcases/GetFeature.json'

@pytest.mark.parametrize('testcase', read_json(GetFeature_json_file_path))
def test_SayHello(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_GetFeature_output(test_input) == expected