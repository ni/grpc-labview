import Utils.get_exported_function_list as get_exported_functions
import json
from pathlib import Path

exported_functions_json_file_path = Path(__file__).parent.parent / "testcases" / "ExportedFunctionList.json"

def read_json(filepath):
    with open(filepath, 'r') as file:
        data = json.load(file)
    return data

new_function_list, new_function_map = get_exported_functions.getFunctionSignatureList()
old_function_list = read_json(exported_functions_json_file_path)
old_function_map = get_exported_functions.getFunctionMap(old_function_list)

if new_function_list["size"] != old_function_list["size"]:
    func_name = ""
    cnt = 1
    for function_name in new_function_map.keys():
        new_function_list = new_function_map.get(function_name)
        old_function_list = old_function_map.get(function_name)
        for function_signature in new_function_list:
            if (old_function_list == None) or (function_signature not in old_function_list):
                func_name += str(cnt) + ") " + function_signature['function_name'] + '\n'
                cnt += 1

    print(f"{func_name}")