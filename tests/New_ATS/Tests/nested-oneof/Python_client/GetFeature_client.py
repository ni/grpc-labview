import grpc
import nested_oneof_pb2
import nested_oneof_pb2_grpc
import json
import pytest
import os

def get_GetFeature_output(test_input):
    req_id = test_input.get('req_id')
    req_price = data_a1 = data_a2 = anotherField = None
    if test_input['req_oneof'].get('req_price'):
        req_price = test_input['req_oneof'].get('req_price')
    if test_input['req_oneof'].get('message_a'):
        anotherField = test_input['req_oneof'].get('message_a').get('anotherField')
        data_a1 = test_input['req_oneof'].get('message_a').get('oneof_a').get('data_a1')
        data_a2 = test_input['req_oneof'].get('message_a').get('oneof_a').get('data_a2')
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = nested_oneof_pb2_grpc.GreeterServiceStub(channel)
        request = nested_oneof_pb2.req()
        request.req_id = req_id
        if req_price != None:
            request.req_price = req_price
        if data_a1 != None:
            request.message_a.data_a1 = data_a1
        if data_a2 != None:
            request.message_a.data_a2 = data_a2
        if anotherField != None:
            request.message_a.anotherField = anotherField

        print(request)
        response = stub.GetFeature(request)
        print(response)
    response_dict = {}
    response_dict['res_id'] = response.res_id
    response_dict['res_oneof'] = {}
    if response.HasField('res_price'):
        response_dict['res_oneof']['res_price'] = response.res_price
    if response.HasField('message_a'):
        response_dict['res_oneof']['message_a'] = {}
        response_dict['res_oneof']['message_a']['oneof_a'] = {}
        response_dict['res_oneof']['message_a']['anotherField'] = response.message_a.anotherField
        if response.message_a.HasField('data_a1'):
            response_dict['res_oneof']['message_a']['oneof_a']['data_a1'] = response.message_a.data_a1
        if response.message_a.HasField('data_a2'):
            response_dict['res_oneof']['message_a']['oneof_a']['data_a2'] = response.message_a.data_a2
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
    response = get_GetFeature_output({'req_id': 561, 'req_oneof': {'message_a': {'anotherField': 'JustAnotherField', 'oneof_a': {'data_a2': 'Ankush'}}}})
    print(response)