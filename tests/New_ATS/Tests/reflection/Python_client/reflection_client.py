import grpc
from grpc_reflection.v1alpha import reflection_pb2, reflection_pb2_grpc
from google.protobuf import descriptor_pb2
import json
import pytest
import os
import sys


def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

reflection_json_file_path = f'{os.path.dirname(os.path.dirname(os.path.abspath(__file__)))}/testcases/reflection.json'

# Returns an array of services which are published with gRPC reflection
def get_services_via_reflection(target: str) -> list[str]:
    channel = grpc.insecure_channel(target)
    stub = reflection_pb2_grpc.ServerReflectionStub(channel)

    request = reflection_pb2.ServerReflectionRequest(
        host='',
        list_services=''
    )

    responses = stub.ServerReflectionInfo(iter([request]))
    for response in responses:
        if response.HasField("list_services_response"):
            return [service.name for service in response.list_services_response.service]
        elif response.HasField("error_response"):
            raise RuntimeError(f"Error {response.error_response.error_code}: {response.error_response.error_message}")

    return []  # Fallback in case no valid response is received

# Returns an array of methods for a specified service which are published with gRPC reflection
def get_service_methods_via_reflection(target: str, service_name: str) -> list[str]:
    channel = grpc.insecure_channel(target)
    stub = reflection_pb2_grpc.ServerReflectionStub(channel)

    file_request = reflection_pb2.ServerReflectionRequest(
        file_containing_symbol=service_name
    )

    responses = stub.ServerReflectionInfo(iter([file_request]))

    for response in responses:
        if response.HasField("file_descriptor_response"):
            method_names = []
            for fd_proto_bytes in response.file_descriptor_response.file_descriptor_proto:
                fd_proto = descriptor_pb2.FileDescriptorProto()
                fd_proto.ParseFromString(fd_proto_bytes)

                for service in fd_proto.service:
                    if service.name == service_name or f"{fd_proto.package}.{service.name}" == service_name:
                        for method in service.method:
                            method_names.append(method.name)
            return method_names
        elif response.HasField("error_response"):
            raise RuntimeError(f"Error {response.error_response.error_code}: {response.error_response.error_message}")

    return []

# Returns an array of field names for a specified message which are published with gRPC reflection
def get_message_description_via_reflection(target: str, message_name: str) -> list[str]:
    channel = grpc.insecure_channel(target)
    stub = reflection_pb2_grpc.ServerReflectionStub(channel)

    request = reflection_pb2.ServerReflectionRequest(
        file_containing_symbol=message_name
    )

    responses = stub.ServerReflectionInfo(iter([request]))

    for response in responses:
        if response.HasField("file_descriptor_response"):
            for fd_bytes in response.file_descriptor_response.file_descriptor_proto:
                fd_proto = descriptor_pb2.FileDescriptorProto()
                fd_proto.ParseFromString(fd_bytes)

                prefix = fd_proto.package + "." if fd_proto.package else ""

                # Search top-level message types - this code does not recursively look for nested messages
                for message in fd_proto.message_type:
                    fq_name = prefix + message.name
                    if fq_name == message_name:
                        return [field.name for field in message.field]

            raise ValueError(f"Message '{message_name}' not found in file descriptors.")
        elif response.HasField("error_response"):
            raise RuntimeError(f"Error {response.error_response.error_code}: {response.error_response.error_message}")
    
    raise RuntimeError("No valid response received from reflection service.")

def get_reflection_output(test_input):
    function = test_input.get('function')
    service = test_input.get('service')
    message = test_input.get('message')

    match function:
        case "get_services":
            response = get_services_via_reflection("localhost:50051")
        case "get_methods":
            response = get_service_methods_via_reflection("localhost:50051", service)
        case "get_message":
            response = get_message_description_via_reflection("localhost:50051", message)
        case _:
            response = [""]
    
    return response

@pytest.mark.parametrize('testcase', read_json(reflection_json_file_path))
def test_reflection(testcase):
    # Function = "get_services", "get_methods", or "get_message"
    #
    # Example:
    # {
    #     "input": { "function": "get_services" },
    #     "output": { "required_output": ["grpc.reflection.v1alpha.ServerReflection", "test_service.TestService"]}
    #   },
    #   {
    #     "input": { "function": "get_methods", "service": "test_service.TestService"},
    #     "output": { "required_output": ["Method1", "Method2", "Method3", "Method4"]}
    #   },
    #   {
    #     "input": { "function": "get_message", "message": "test_service.Method1Request"},
    #     "output": { "required_output": ["input1", "input2"]}
    #   }
    #
    # Output = An array of values to check for.  The actual output from the specified function may
    # contain /more/ than these values.  However, the function checks that /at least/ the values
    # specified in the test case are returned.

    test_input = testcase['input']
    expected_output = testcase['output']['required_output']
   
    # Call reflection test
    response = get_reflection_output(test_input)

    # Verify that each item in expected_output exists
    for output_element in expected_output:
        assert output_element in response


if __name__ == "__main__":
    target = sys.argv[1] if len(sys.argv) > 1 else "localhost:50051"

    # These methods are just used as an example using the associated proto file to generate a LV server
    res = get_services_via_reflection(target)
    print(res)
    res = get_service_methods_via_reflection(target, "test_service.TestService")
    print(res)
    res = get_message_description_via_reflection(target, "test_service.Method1Request")
    print(res)
