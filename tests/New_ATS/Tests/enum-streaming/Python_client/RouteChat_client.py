import grpc
import routeguide_pb2
import routeguide_pb2_grpc
import json
import pytest
import os

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

RouteChat_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/RouteChat.json'

def get_RouteChat_output(test_input):
    output = []
    
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = routeguide_pb2_grpc.RouteGuideStub(channel)
        
        def generate_notes():
            for point in test_input:
                yield routeguide_pb2.RouteNote(
                    location=routeguide_pb2.Point(
                        latitude=point['location']['latitude'],
                        longitude=point['location']['longitude']
                    ),
                    message=point['message']
                )
        
        response = stub.RouteChat(generate_notes())
        
        for note in response:
            output.append({
                "location": {
                    "latitude": note.location.latitude,
                    "longitude": note.location.longitude
                },
                "message": note.message
            })
    
    return output

@pytest.mark.parametrize('testcase', read_json(RouteChat_json_file_path))
def test_RouteChat(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_RouteChat_output(test_input) == expected

if __name__ == "__main__":
    res = get_RouteChat_output([
        { "location": { "latitude": 1, "longitude": 4 }, "message": "Point 1" },
        { "location": { "latitude": 2, "longitude": 3 }, "message": "Point 2" },
        { "location": { "latitude": 3, "longitude": 2 }, "message": "Point 3" },
        { "location": { "latitude": 4, "longitude": 1 }, "message": "Point 4" }
    ])
    print(res)
