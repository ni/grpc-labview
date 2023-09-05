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

ListFeatures_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/ListFeatures.json'

def get_ListFeatures_output(test_input):
    lo = test_input['lo']
    hi = test_input['hi']
    
    output = []
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = routeguide_pb2_grpc.RouteGuideStub(channel)
        rectangle = routeguide_pb2.Rectangle(
            lo=routeguide_pb2.Point(latitude=lo['latitude'], longitude=lo['longitude']),
            hi=routeguide_pb2.Point(latitude=hi['latitude'], longitude=hi['longitude'])
        )
        response = stub.ListFeatures(rectangle)
        stub.ListFeatures
    
        for feature in response:
            output.append({
                "name": feature.name,
                "location": {
                    "latitude": feature.location.latitude,
                    "longitude": feature.location.longitude
                }
            })
    return output


@pytest.mark.parametrize('testcase', read_json(ListFeatures_json_file_path))
def test_ListFeatures(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_ListFeatures_output(test_input) == expected

if __name__ == "__main__":
    res = get_ListFeatures_output({
        "lo": { "latitude": 1, "longitude": 2 },
        "hi": { "latitude": 3, "longitude": 4 }
    })
    print(res)
