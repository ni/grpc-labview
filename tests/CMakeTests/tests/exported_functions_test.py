import re
import json
import pytest
import os
import Utils.get_exported_function_list as get_exported_function_list

struct_json_file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../testcases/ExportedFunctionList.json")

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

exported_function_list, function_map = get_exported_function_list.getFunctionSignatureList()
# function_map = get_exported_function_list.get_function_map(exported_function_list)

@pytest.mark.parametrize('function', read_json(struct_json_file_path)['signatures'])
def test_functions(function):
    test_input = function_map.get(function['function_name'])
    expected_output = {"function_name": function["function_name"], "return_type": function["return_type"], "parameter_list": function["parameter_list"]}
    assert test_input == expected_output
