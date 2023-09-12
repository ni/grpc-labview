import grpc
import oneof_streaming_pb2  
import oneof_streaming_pb2_grpc 
import json
import pytest
import os

def get_ServerStreamingMethod_output(test_input):
    age = test_input['age']
    message = test_input['request_oneof'].get('message')
    request_id=test_input['request_oneof'].get('request_id')
    
    response_list = []
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = oneof_streaming_pb2_grpc.GreeterServiceStub(channel)   
        request = oneof_streaming_pb2.Request(age=age, message=message, request_id=request_id)
        response_stream = stub.ServerStreamingMethod(request)

        for response in response_stream:
            response_dict = {
                'new_age': response.new_age,
                'response_oneof': {}
            }
            if response.HasField('result'):
                response_dict['response_oneof']['result'] = response.result
            if response.HasField('response_id'):
                response_dict['response_oneof']['response_id'] = response.response_id
            response_list.append(response_dict)
    
    return response_list

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

ServerStreamingMethod_json_file_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'testcases', 'ServerStreamingMethod.json')

@pytest.mark.parametrize('testcase', read_json(ServerStreamingMethod_json_file_path))
def test_ServerStreamingMethod(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_ServerStreamingMethod_output(test_input) == expected

if __name__ == "__main__":
    res = get_ServerStreamingMethod_output({
        "age": 2,
        "request_oneof": {
            "message": "Hello, World!"
        }
    })
    print(res)
