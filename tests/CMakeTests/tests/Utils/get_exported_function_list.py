import re
import json
from pathlib import Path

folder_path = Path(__file__).parent.parent.parent.parent.parent / "src"

# Regex pattern to match exported function signatures
pattern = re.compile(r'\bLIBRARY_EXPORT\b\s+(.*?)\s*{', re.DOTALL)

def getFunctionSignatureList():
    sorted_signature_list = {"size": 0, "signatures": []}
    unsorted_signature_list = []

    # Traversing all the files in the src folder
    for file_path in folder_path.rglob("*"):
        if str(file_path).endswith('.cc'):
            with open(file_path, 'r') as f:
                content = f.read()
                matches = pattern.findall(content)
                for match in matches:
                    # Replace new line with space
                    match = match.replace('\n', ' ')

                    # Extract Parameters
                    match_list = match.split('(')
                    return_type = match_list[0].split()[0].strip()
                    function_name = match_list[0].split()[-1].strip()
                    parameter_list = match_list[1].split(')')[0].split(',')

                    # Remove the parameter name
                    for i in range(len(parameter_list)):
                        parameter = parameter_list[i]
                        parameter_name = parameter.split()[-1]
                        cur = ' '.join(parameter.split()[:-1])
                        if parameter_name[0] == '*':
                            cur += '*'
                        if len(parameter_name) > 1 and parameter_name[1] == '*':
                            cur += '*'  
                        cur += ' '+parameter.split()[-1]
                        parameter_list[i] = cur
                    parameter_list = [' '.join(parameter.split()[:-1]) for parameter in parameter_list]

                    # Add parameter list and function name
                    # match = return_type + ' ' + function_name + '(' + ', '.join(parameter_list) + ')'

                    # Add each signature into unsorted list
                    unsorted_signature_list.append({"id": 404, "function_name": function_name, "return_type": return_type, "parameter_list": parameter_list})

    sorted_signature_list["size"] = len(unsorted_signature_list)
    sorted_signature_list["signatures"] = sorted(unsorted_signature_list, key=lambda k: k['function_name'])

    # Setting the id and creating a function map that maps the function values to the name of the function
    function_map = {}
    for i in range(len(sorted_signature_list["signatures"])):
        sorted_signature_list["signatures"][i]["id"] = i
        signature = sorted_signature_list["signatures"][i]
        function_map[signature['function_name']] = {"function_name": signature["function_name"], "return_type": signature['return_type'], "parameter_list": signature['parameter_list']}
    
    return [sorted_signature_list, function_map]

def getFunctionMap(sorted_signature_list):
    function_map = {}
    for signature in sorted_signature_list['signatures']:
        function_map[signature['function_name']] = {"function_name": signature["function_name"], "return_type": signature['return_type'], "parameter_list": signature['parameter_list']}
    return function_map

if __name__ == '__main__':
    with open("exported_function_list.json",'w') as f:
        json.dump(getFunctionSignatureList()[0],f, indent=2)