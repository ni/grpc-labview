import grpc
import enum_streaming_pb2
import enum_streaming_pb2_grpc
import json
import pytest
import os
from enum import Enum
 
# 'ServerStreamingMethod' rpc performs the following operations on the 'request' message fields and streams 'request_number' number of 'response' messages:
# 1) Increments response_number from the previous response_number by 1 starting with the request_number value in the first 'response' message's response_number.
# 2) Appends '_response' to the request_message field and saves in response_message field of each 'response' message.
# 3) Save the same request_color enum value in response_color of each 'response' message
class Color(Enum):
    RED = 0
    GREEN = 1
    BLUE = 2

def get_ServerStreamingMethod_output(test_input):
    request_number = test_input["request_number"]
    request_message = test_input["request_message"]
    request_color = test_input["request_color"]
    if Color.RED.value == request_color:
        request_color = Color.RED
    if Color.GREEN.value == request_color:
        request_color = Color.GREEN
    if Color.BLUE.value == request_color:
        request_color = Color.BLUE
    
    responses = []

    with grpc.insecure_channel('localhost:50051') as channel:
        stub = enum_streaming_pb2_grpc.RouteGuideStub(channel)   
        request = enum_streaming_pb2.request(request_number=request_number, request_message=request_message, request_color=request_color)
        response_stream = stub.ServerStreamingMethod(request)

        for response in response_stream:
            response_dict = {
                "response_number": response.response_number,
                "response_message": response.response_message,
                "response_color": response.response_color,
            }
            if response_dict["response_color"] == 0:
                response_dict["response_color"] = "RED"
            if response_dict["response_color"] == 1:
                response_dict["response_color"] = "GREEN"
            if response_dict["response_color"] == 2:
                response_dict["response_color"] = "BLUE"
    
            responses.append(response_dict)
    
    return responses

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

ServerStreamingMethod_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/ServerStreamingMethod.json'

@pytest.mark.parametrize('testcase', read_json(ServerStreamingMethod_json_file_path))
def test_ServerStreamingMethod(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_ServerStreamingMethod_output(test_input) == expected

if __name__ == "__main__":
    res = get_ServerStreamingMethod_output({
        "request_number": 10,
        "request_message": "Hello, World!",
        "request_color": "BLUE"
    })
    print(res)