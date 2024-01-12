import subprocess
import sys
import os

def run_tests():
    message_structures_test_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "tests/message_structure_test.py")
    exported_functions_test_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "tests/exported_functions_test.py")

    # Check the exit code
    result_message_structure = subprocess.run(["python", "-m", "pytest", message_structures_test_path, "-vv"])
    if result_message_structure.returncode != 0:
        print(f"Message structural tests failed with exit code {result_message_structure.returncode}. Exiting.")
        sys.exit(result_message_structure.returncode)
    result_exported_functions = subprocess.run(["python", "-m", "pytest", exported_functions_test_path, "-vv"])
    if result_exported_functions.returncode != 0:
        print(f"Function structural tests failed with exit code {result_exported_functions.returncode}. Exiting.")
        sys.exit(result_exported_functions.returncode)
    
    print("All structural tests passed.")

if __name__ == "__main__":
    run_tests()