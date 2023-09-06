import grpc
import oneof_streaming_pb2
import oneof_streaming_pb2_grpc
import json
import pytest
import os

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

ClientStreamingMethod_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/ClientStreamingMethod.json'

def get_ClientStreamingMethod_output(test_input):
    response_dict = None
    request_list = []
    for request in test_input:
        request_list.append(oneof_streaming_pb2.Request(
            age=request['age'],
            message=request['request_oneof'].get('message'),
            request_id=request['request_oneof'].get('request_id')
            )
        )

    with grpc.insecure_channel('localhost:50051') as channel:
        
        stub = oneof_streaming_pb2_grpc.GreeterServiceStub(channel)
    
        response = stub.ClientStreamingMethod(iter(request_list))
        
        response_dict = {
            "new_age":response.new_age,
            "response_oneof":{}
        }
        if response.HasField('result'):
            response_dict['response_oneof']['result'] = response.result
        if response.HasField('response_id'):
            response_dict['response_oneof']['response_id'] = response.response_id
        
    return response_dict

@pytest.mark.parametrize('testcase', read_json(ClientStreamingMethod_json_file_path))
def test_ClientStreamingMethod(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_ClientStreamingMethod_output(test_input) == expected

if __name__ == "__main__":
    res = get_ClientStreamingMethod_output([
        {
            "age": 2,
            "request_oneof": {
                "message": "Hello, World!"
            }
        },
        {
            "age": 2,
            "request_oneof": {
                "message": "Hello, World!"
            }
        }
    ])
    print(res)
