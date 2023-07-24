import grpc
import helloworld_pb2
import helloworld_pb2_grpc
import json
import pytest
import os

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

SayHello_json_file_path = f'{os.path.dirname(os.path.abspath(__file__))}/testcases/SayHello.json'

def get_SayHello_output(test_input):
    name = test_input['name']
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = helloworld_pb2_grpc.GreeterServiceStub(channel)
        request = helloworld_pb2.HelloRequest(name=name)
        response = stub.SayHello(request)
    return {"message":response.message}

@pytest.mark.parametrize('testcase', read_json(SayHello_json_file_path))
def test_SayHello(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_SayHello_output(test_input) == expected

if __name__ == "__main__":
    res = get_SayHello_output({'name': "Yash"})
    print(res)