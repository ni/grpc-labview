import subprocess
import sys
import os
from pathlib import Path

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
    message_structures_test_path = Path(__file__).parent / "tests" / "message_structure_test.py"
    exported_functions_test_path = Path(__file__).parent / "tests" / "exported_functions_test.py"
    exported_functions_addition_test_path = Path(__file__).parent / "tests" / "exported_functions_addition_test.py"

    result_message_structure = subprocess.run(f"{activate_command} && python -m pytest {str(message_structures_test_path)} -vv", shell=True)
    if result_message_structure.returncode != 0:
        result_message_structure = subprocess.run(["python", "-m", "pytest", str(message_structures_test_path), "-vv"])
    if result_message_structure.returncode != 0:
        print(f"Message structural tests failed with exit code {result_message_structure.returncode}. Exiting.")
        sys.exit(result_message_structure.returncode)

    result_exported_functions = subprocess.run(f"{activate_command} && python -m pytest {str(exported_functions_test_path)} -vv", shell=True)
    if result_exported_functions.returncode != 0:
        result_exported_functions = subprocess.run(["python", "-m", "pytest", str(exported_functions_test_path), "-vv"])
    if result_exported_functions.returncode != 0:
        print(f"Function structural tests failed with exit code {result_exported_functions.returncode}. Exiting.")
        sys.exit(result_exported_functions.returncode)
    
    result_exported_functions_addition = subprocess.run(f"{activate_command} && python {str(exported_functions_addition_test_path)}", shell=True)
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