import grpc
import sibling_oneof_pb2
import sibling_oneof_pb2_grpc
import json
import pytest
import os

# 'GetFeature' rpc performs the following operations on the 'req' message fields:
# 1) Increments req_latitude by 1 and saves in res_latitude of 'res' message.
# 2) Increments req_id of req_oneof by 1 and saves in res_oneof's res_id field of 'res' message. 
# 3) Appends '_response' to the req_name field of req_oneof and save in res_name field of res_oneof of 'res' message.
# 4) Appends '_response' to the data_a1 field of message_a field and saves in corresponding message_a field of 'res' message.
def get_GetFeature_output(test_input):
    req_latitude = test_input.get('req_latitude')
    message_a = req_id = req_name = None
    if test_input.get('message_a'):
        data_a1 = test_input['message_a'].get('data_a1')
        message_a = sibling_oneof_pb2.MessageA(data_a1=data_a1)
    if test_input.get('req_oneof'):
        req_name = test_input['req_oneof'].get('req_name')
        req_id = test_input['req_oneof'].get('req_id')
    
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = sibling_oneof_pb2_grpc.GreeterServiceStub(channel)
        request = sibling_oneof_pb2.req(req_latitude=req_latitude, req_id=req_id, req_name=req_name, message_a=message_a)
        response = stub.GetFeature(request)
    response_dict = {}
    response_dict['res_latitude'] = response.res_latitude
    response_dict['message_a'] = {}
    response_dict['message_a']['data_a1'] = response.message_a.data_a1
    response_dict['res_oneof'] = {}
    if(response.HasField('res_name')):
        response_dict['res_oneof']['res_name'] = response.res_name
    if(response.HasField('res_id')):
        response_dict['res_oneof']['res_id'] = response.res_id
    return(response_dict)

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

GetFeature_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/GetFeature.json'

@pytest.mark.parametrize('testcase', read_json(GetFeature_json_file_path))
def test_GetFeature(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_GetFeature_output(test_input) == expected

if __name__ == "__main__":
    # print(get_GetFeature_output({"req_latitude":30, "message_a":{"data_a1":"Labview"}, "req_oneof":{"req_name":"Yash"}}))
    print(get_GetFeature_output({"req_latitude":10, "messsage_a":{"data_a1":"Wow"}, "req_oneof":{"req_name":"Yash"}}))