import grpc
import message_oneof_pb2
import message_oneof_pb2_grpc
import json
import pytest
import os

def get_GetFeature_output(test_input):
    req_id = test_input.get('req_id')
    req_name = test_input['req_oneof'].get('req_name')
    data_a1 = message_a = None
    if test_input['req_oneof'].get('message_a'):
        data_a1 = test_input['req_oneof']['message_a']['data_a1']
        message_a = message_oneof_pb2.MessageA(data_a1=data_a1)
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = message_oneof_pb2_grpc.GreeterServiceStub(channel)
        request = message_oneof_pb2.req(req_id=req_id, req_name=req_name, message_a=message_a)
        response = stub.GetFeature(request)
    response_dict = {}
    response_dict['res_id'] = response.res_id
    response_dict['res_oneof'] = {}
    if(response.HasField('res_name')):
        response_dict['res_oneof']['res_name'] = response.res_name
    if(response.HasField('message_a')):
        response_dict['res_oneof']['message_a'] = {}
        response_dict['res_oneof']['message_a']['data_a1'] = response.message_a.data_a1
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
