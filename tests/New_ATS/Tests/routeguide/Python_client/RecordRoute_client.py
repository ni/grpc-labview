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

RecordRoute_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/RecordRoute.json'

def get_RecordRoute_output(test_input):
    points = [routeguide_pb2.Point(latitude=point['latitude'], longitude=point['longitude']) for point in test_input]
    
    ans = None
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = routeguide_pb2_grpc.RouteGuideStub(channel)
        response = stub.RecordRoute(iter(points))
        ans = {
            "point_count": response.point_count,
            "feature_count": response.feature_count,
            "distance": response.distance,
            "elapsed_time": response.elapsed_time
        }
    return ans;

@pytest.mark.parametrize('testcase', read_json(RecordRoute_json_file_path))
def test_RecordRoute(testcase):
    test_input = testcase['input']
    expected = testcase['output']
    assert get_RecordRoute_output(test_input) == expected

if __name__ == "__main__":
    res = get_RecordRoute_output([{"latitude": 1, "longitude": 5}, {"latitude": 12, "longitude": 10}, {"latitude": 20, "longitude": 25}])
    print(res)
