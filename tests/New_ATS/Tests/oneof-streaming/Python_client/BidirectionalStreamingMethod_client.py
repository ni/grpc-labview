import grpc
import oneof_streaming_pb2
import oneof_streaming_pb2_grpc
import json
import pytest
import os

# 'BidirectionalStreamingMethod' rpc performs the following operations on the 'Request' message fields and returns one 'Response' message for each 'Request' message:
# 1) Increments age by 1 and saves in new_age of 'Response' message.
# 2) Increments request_id of request_oneof by 1 and saves in response_oneof's response_id field of 'Response' message. 
# 3) Appends '_response' to the message field of request_oneof and saves in result field of response_oneof of 'Response' message.
def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

BidirectionalStreamingMethod_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/BidirectionalStreamingMethod.json'

def get_BidirectionalStreamingMethod_output(test_input):
    output = []
    
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = oneof_streaming_pb2_grpc.GreeterServiceStub(channel)
        
        def generate_requests():
            for request_data in test_input:
                yield oneof_streaming_pb2.Request(
                    age=request_data['age'],
                    message=request_data['request_oneof'].get('message'),
                    request_id=request_data['request_oneof'].get('request_id')
                )
        
        responses = stub.BidirectionalStreamingMethod(generate_requests())
        
        for response in responses:
            output.append({
                "new_age": response.new_age,
                "response_oneof": {
                    "result": response.result
                }
            })
    
    return output

@pytest.mark.parametrize('testcase', read_json(BidirectionalStreamingMethod_json_file_path))
def test_BidirectionalStreamingMethod(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_BidirectionalStreamingMethod_output(test_input) == expected

if __name__ == "__main__":
    res = get_BidirectionalStreamingMethod_output([
        { "age": 2, "request_oneof": { "message": "Hello, World!" } },
        { "age": 2, "request_oneof": { "message": "Hello, World!" } }
    ])
    print(res)
