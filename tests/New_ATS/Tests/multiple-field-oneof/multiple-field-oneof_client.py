import grpc
import multiple_field_oneof_pb2
import multiple_field_oneof_pb2_grpc
import json
import pytest
import os

def get_GetFeature_output(test_input):
    req_latitude = test_input.get('req_latitude')
    req_name = test_input['req_oneof'].get('req_name')
    req_num = test_input['req_oneof'].get('req_num')
    flag = test_input['req_oneof'].get('flag')
    price = test_input['req_oneof'].get('price')
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = multiple_field_oneof_pb2_grpc.GreeterServiceStub(channel)
        request = multiple_field_oneof_pb2.req(req_latitude=req_latitude, req_name=req_name, req_num=req_num, flag=flag, price=price)
        response = stub.GetFeature(request)
    response_dict = {}
    response_dict['res_latitude'] = response.res_latitude
    response_dict['res_oneof'] = {}  
    if response.HasField('res_num'):
        response_dict['res_oneof']['res_num'] = response.res_num
    if response.HasField('res_name'):
        response_dict['res_oneof']['res_name'] = response.res_name
    if response.HasField('flag'):
        response_dict['res_oneof']['flag'] = response.flag
    if response.HasField('price'):
        response_dict['res_oneof']['price'] = response.price

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

if __name__ == "__main__":
    response = get_GetFeature_output({"req_latitude":100, "req_oneof":{"price":123.3}})
    print(response)