import os
import re
import json

folder_path = os.path.abspath(os.path.join(__file__, '../../../../../src'))

# Regex pattern to match exported function signatures
pattern = re.compile(r'\bLIBRARY_EXPORT\b\s+(.*?)\s*{', re.DOTALL)

def getFunctionSignatureList():
    sorted_signature_list = {"size": 0, "signatures": []}
    unsorted_signature_list = []
    for root, dirs, files in os.walk(folder_path):
        for file in files:
            if file.endswith('.cc'):
                with open(os.path.join(root, file), 'r') as f:
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
    for i in range(1, len(sorted_signature_list["signatures"])+1):
        sorted_signature_list["signatures"][i-1]["id"] = i
    
    return sorted_signature_list

def get_function_map(sorted_signature_list):
    function_map = {}
    for signature in sorted_signature_list['signatures']:
        function_map[signature['function_name']] = {"function_name": signature["function_name"], "return_type": signature['return_type'], "parameter_list": signature['parameter_list']}
    return function_map

if __name__ == '__main__':
    with open("exported_function_list.json",'w') as f:
        json.dump(getFunctionSignatureList(),f, indent=2)