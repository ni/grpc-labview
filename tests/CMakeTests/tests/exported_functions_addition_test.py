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
    for func in new_function_map.keys():
        if old_function_map.get(func) == None:
           func_name += str(cnt) + ")" + func + ' '
           cnt += 1

    print(f"::warning::You have not added these function in the 'Exported Functions' cmake test: {func_name}. Please add these function(s) in tests/CMakeTests/testcases/ExportedFunctionList.json")

else:
    print("All tests passed. No addition in exported functions.")