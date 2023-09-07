import grpc
import enum_streaming_pb2
import enum_streaming_pb2_grpc
import json
import pytest
import os
from enum import Enum
 
class Color(Enum):
    RED = 0
    GREEN = 1
    BLUE = 2

def get_BidirectionalStreamingMethod_output(test_input):
    
    responses = []
    
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = enum_streaming_pb2_grpc.RouteGuideStub(channel)   
        def get_requests():
            for request in test_input:
                request_number = request["request_number"]
                request_message = request["request_message"]
                request_color = request["request_color"]
                if Color.RED.value == request_color:
                    request_color = Color.RED
                if Color.GREEN.value == request_color:
                    request_color = Color.GREEN
                if Color.BLUE.value == request_color:
                    request_color = Color.BLUE
                yield enum_streaming_pb2.request(request_number=request_number, request_message=request_message, request_color=request_color)
        response_stream = stub.BidirectionalStreamingMethod(get_requests())
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

BidirectionalStreamingMethod_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/BidirectionalStreamingMethod.json'

@pytest.mark.parametrize('testcase', read_json(BidirectionalStreamingMethod_json_file_path))
def test_BidirectionalStreamingMethod(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_BidirectionalStreamingMethod_output(test_input) == expected

if __name__ == "__main__":
    res = get_BidirectionalStreamingMethod_output([
        {
            "request_number": 25,
            "request_message": "Hello, World!",
            "request_color": "RED"
        },
        {
            "request_number": 52,
            "request_message": "Goodbye, World!",
            "request_color": "BLUE"
        }
    ])
    print(res)