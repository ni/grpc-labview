import json
import pytest
from pathlib import Path
import Utils.get_exported_function_list as get_exported_function_list

exported_functions_json_file_path = Path(__file__).parent.parent / "testcases" / "ExportedFunctionList.json"

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

parsed_exported_function_list, parsed_function_map = get_exported_function_list.getFunctionSignatureList()

@pytest.mark.parametrize('function_signature', read_json(exported_functions_json_file_path)['signatures'])
def test_function_compatibility(function_signature):
    test_input_list = parsed_function_map.get(function_signature['function_name'])
    expected_output = {"function_name": function_signature["function_name"], "return_type": function_signature["return_type"], "parameter_list": function_signature["parameter_list"]}
    if test_input_list == None:
        assert None == expected_output
    for test_input in test_input_list:
        if test_input == expected_output:
            assert test_input == expected_output
            return
    assert None == expected_output