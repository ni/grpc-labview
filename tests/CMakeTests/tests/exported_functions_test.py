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

exported_function_list = get_exported_function_list.getFunctionSignatureList()
function_map = get_exported_function_list.get_function_map(exported_function_list)

@pytest.mark.parametrize('struct', read_json(struct_json_file_path)['signatures'])
def test_structs(struct):
    test_input = function_map[struct['function_name']]
    expected_output = {"function_name": struct["function_name"], "return_type": struct["return_type"], "parameter_list": struct["parameter_list"]}
    assert test_input == expected_output
