import Utils.get_exported_function_list as get_exported_functions
import json
import os

struct_json_file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../testcases/ExportedFunctionList.json")

def read_json(filepath):
    with open(filepath, 'r') as file:
        data = json.load(file)
    return data

new_function_list, new_function_map = get_exported_functions.getFunctionSignatureList()
old_function_list = read_json(struct_json_file_path)
old_function_map = get_exported_functions.getFunctionMap(old_function_list)

if new_function_list["size"] != old_function_list["size"]:
    func_name = ""
    cnt = 1
    for func in new_function_map.keys():
        if old_function_map.get(func) == None:
           func_name += "    " + str(cnt) + ". " + func + '\n'
           cnt += 1

    print(f"::warning::You have not added these function in the 'Exported Functions' cmake test:\n{func_name}Please add these function(s) in tests/CMakeTests/testcases/ExportedFunctionList.json")