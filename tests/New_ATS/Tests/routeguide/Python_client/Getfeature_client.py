import grpc
import routeguide_pb2
import routeguide_pb2_grpc
import json
import pytest
import os

# A simple RPC.
#
# Obtains the feature at a given position.
#
# A feature with an empty name is returned if there's no feature at the given
# position.

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

GetFeature_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/GetFeature.json'

def get_GetFeature_output(test_input):
    latitude = test_input['latitude']
    longitude = test_input['longitude']
    
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = routeguide_pb2_grpc.RouteGuideStub(channel)
        point = routeguide_pb2.Point(latitude=latitude, longitude=longitude)
        response = stub.GetFeature(point)
    
    return {
        "name": response.name,
        "location": {
            "latitude": response.location.latitude,
            "longitude": response.location.longitude
        }
    }

@pytest.mark.parametrize('testcase', read_json(GetFeature_json_file_path))
def test_GetFeature(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_GetFeature_output(test_input) == expected

if __name__ == "__main__":
    res = get_GetFeature_output({'latitude': 1, 'longitude': 2})
    print(res)
