import grpc
import simple_oneof_pb2
import simple_oneof_pb2_grpc
import json
import pytest
import os

def get_GetFeature_output(test_input):
    req_latitude = int(test_input.get('req_latitude'))
    req_name = test_input['req_oneof'].get('req_name')
    req_num = test_input['req_oneof'].get('req_num')
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = simple_oneof_pb2_grpc.GreeterServiceStub(channel)
        request = simple_oneof_pb2.req(req_latitude=req_latitude, req_name=req_name, req_num=req_num)
        response = stub.GetFeature(request)
    response_dict = {}
    response_dict['res_latitude'] = response.res_latitude
    response_dict['res_oneof'] = {}  
    if response.HasField('res_num'):
        response_dict['res_oneof']['res_num'] = response.res_num
    if response.HasField('res_name'):
        response_dict['res_oneof']['res_name'] = response.res_name
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