import subprocess
import sys
import os
from pathlib import Path
import datetime

# Setting global variables
if os.name == 'nt':     # windows
    venv_command = "python -m venv venv"
    activate_script = "venv\\Scripts\\activate"
    activate_command = f"call {activate_script}"
else: 
    venv_command = "python -m venv venv"
    activate_script = "venv/bin/activate"
    activate_command = f". {activate_script}"

def create_virtual_environment():
    venv_folder_path = Path(__file__).parent / "venv"
    if not venv_folder_path.exists():
        subprocess.run(venv_command, shell=True)

def install_dependencies():
    subprocess.run(f"{activate_command} && pip install pytest", shell=True)

def run_tests():

    current_time = datetime.datetime.now().strftime("%d-%m-%Y_%H-%M-%S")
    log_file_path = Path(__file__).parent / "logs" / f"CMakeTests_log_{current_time}.txt"
    logs_folder_path = Path(__file__).parent / "logs"

    if not logs_folder_path.exists():
        logs_folder_path.mkdir(parents=True)

    message_structures_test_path = Path(__file__).parent / "tests" / "message_structure_test.py"
    exported_functions_test_path = Path(__file__).parent / "tests" / "exported_functions_test.py"
    exported_functions_addition_test_path = Path(__file__).parent / "tests" / "exported_functions_addition_test.py"

    result_message_structure = subprocess.run(f"{activate_command} && python -m pytest {str(message_structures_test_path)} -vv", shell=True, stdout=subprocess.PIPE)
    result_message_structure_output = result_message_structure.stdout.decode('utf-8')    
    with open(log_file_path, "a") as log_file:
        log_file.write("-----------------------------------------------------------------------")
        log_file.write("\n----------------------- Message structure tests -----------------------")
        log_file.write("\n-----------------------------------------------------------------------\n\n")
        if os.name == 'nt':
            result_message_structure_output = result_message_structure_output.replace("\r", "")
        log_file.write(result_message_structure_output)
        print(result_message_structure_output)
    if result_message_structure.returncode != 0:
        result_message_structure = subprocess.run(["python", "-m", "pytest", str(message_structures_test_path), "-vv"])
    if result_message_structure.returncode != 0:
        print(f"Message structural tests failed with exit code {result_message_structure.returncode}. Exiting.")
        sys.exit(result_message_structure.returncode)

    result_exported_functions = subprocess.run(f"{activate_command} && python -m pytest {str(exported_functions_test_path)} -vv", shell=True,stdout=subprocess.PIPE)
    result_exported_functions_output = result_exported_functions.stdout.decode('utf-8')
    with open(log_file_path, "a") as log_file:
        log_file.write("\n------------------------------------------------------------------------")
        log_file.write("\n----------------------- Exported functions tests -----------------------")
        log_file.write("\n------------------------------------------------------------------------\n\n")
        if os.name == 'nt':
            result_exported_functions_output = result_exported_functions_output.replace("\r", "")
        log_file.write(result_exported_functions_output)
        print(result_exported_functions_output)
    if result_exported_functions.returncode != 0:
        result_exported_functions = subprocess.run(["python", "-m", "pytest", str(exported_functions_test_path), "-vv"])
    if result_exported_functions.returncode != 0:
        print(f"Function structural tests failed with exit code {result_exported_functions.returncode}. Exiting.")
        sys.exit(result_exported_functions.returncode)
    
    result_exported_functions_addition = subprocess.run(f"{activate_command} && python {str(exported_functions_addition_test_path)}", shell=True, stdout=subprocess.PIPE)
    result_exported_functions_addition_output = result_exported_functions_addition.stdout.decode('utf-8')
    with open(log_file_path, "a") as log_file:
        log_file.write("\n------------------------------------------------------------------------")
        log_file.write("\n------------------- Exported function addition tests -------------------")
        log_file.write("\n------------------------------------------------------------------------\n\n")
        if os.name == 'nt':
            result_exported_functions_addition_output = result_exported_functions_addition_output.replace("\r", "")
        if result_exported_functions_addition_output == "":
            result_exported_functions_addition_output = "All tests passed. No addition in exported functions."
            print(result_exported_functions_addition_output ,'\n')
            log_file.write(result_exported_functions_addition_output)
        else:
            log_file.write("You have not added these function in the 'Exported Functions Cmake Test: " + "\n\n" + result_exported_functions_addition_output + "Please add these function(s) in tests/CMakeTests/testcases/ExportedFunctionList.json")
            warning_output = "::warning::" + "You have not added these function in the 'Exported Functions Cmake Test: " + " ".join(result_exported_functions_addition_output.split('\n')) + ". Please add these function(s) in tests/CMakeTests/testcases/ExportedFunctionList.json"
            subprocess.run(["echo", warning_output], shell=True)
    if result_exported_functions_addition.returncode != 0:
        result_exported_functions_addition = subprocess.run(["python", str(exported_functions_addition_test_path)])
    if result_exported_functions_addition.returncode != 0:
        print(f"Function structural tests failed with exit code {result_exported_functions_addition.returncode}. Exiting.")
        sys.exit(result_exported_functions_addition.returncode)
    
    print("All structural tests passed.")

if __name__ == "__main__":
    create_virtual_environment()
    install_dependencies()
    run_tests()