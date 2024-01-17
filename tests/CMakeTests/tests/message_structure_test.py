import re
import json
import pytest
from pathlib import Path

struct_json_file_path = Path(__file__).parent.parent / "testcases" / "MessageStructures.json"

def read_json(filepath):
    with open(filepath, 'r') as file:
        test_data = json.load(file)
    return test_data

def extract_struct_by_name(file_path, target_struct_name):
    with open(file_path, 'r') as file:
        content = file.read()

    struct_pattern = re.compile(r'\bstruct\s+(\w+)\s*{([^}]*)\s*};', re.DOTALL)
    field_pattern = re.compile(r'\b(\w+)\s+(\w+)\s*;')

    struct_matches = struct_pattern.findall(content)

    for match in struct_matches:
        struct_name = match[0]
        if struct_name == target_struct_name:
            field_matches = field_pattern.findall(match[1])
            fields = [f"{field[0]} {field[1]}" for field in field_matches]
            return {'name': struct_name, 'fields': fields}

    return {'name': '', 'fields': []}

cpp_file_path = Path(__file__).parent.parent.parent.parent / "src" / "message_metadata.h"

@pytest.mark.parametrize('struct', read_json(struct_json_file_path))
def test_struct_compatibility(struct):
    test_input = extract_struct_by_name(cpp_file_path, struct['name'])['fields']
    expected_output = struct['fields']
    assert test_input == expected_output
