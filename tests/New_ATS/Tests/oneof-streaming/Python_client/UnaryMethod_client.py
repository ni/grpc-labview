import grpc
import oneof_streaming_pb2
import oneof_streaming_pb2_grpc
import json
import pytest
import os

# 'UnaryMethod' rpc performs the following operations on the 'Request' message fields:
# 1) Increments age by 1 and saves in new_age of 'Response' message.
# 2) Increments request_id of request_oneof by 1 and saves in response_oneof's response_id field of 'Response' message. 
# 3) Appends '_response' to the message field of request_oneof and saves in result field of response_oneof of 'Response' message.
def get_UnaryMethod_output(test_input):
    age = test_input['age']
    message = test_input['request_oneof'].get('message')
    request_id = test_input['request_oneof'].get('request_id')
    
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = oneof_streaming_pb2_grpc.GreeterServiceStub(channel)   
        request = oneof_streaming_pb2.Request(age=age, message=message, request_id=request_id)
        response = stub.UnaryMethod(request)
    
    response_dict = {
        'new_age': response.new_age,
        'response_oneof': {}
    }
    
    if response.HasField('result'):
        response_dict['response_oneof']['result'] = response.result
    if response.HasField('response_id'):
        response_dict['response_oneof']['response_id'] = response.response_id
    
    return response_dict

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

UnaryMethod_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/UnaryMethod.json'

@pytest.mark.parametrize('testcase', read_json(UnaryMethod_json_file_path))
def test_UnaryMethod(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_UnaryMethod_output(test_input) == expected

if __name__ == "__main__":
    res = get_UnaryMethod_output({
        "age": 25,
        "request_oneof": {
            "message": "Hello, World!"
        }
    })
    print(res)
