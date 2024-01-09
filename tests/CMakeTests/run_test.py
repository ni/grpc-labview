import subprocess
import sys
import os

def run_tests():
    test_example_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "test.py")
    # Run pytest
    result = subprocess.run(["python", "-m", "pytest", test_example_path, "-vv"])

    # Check the exit code
    if result.returncode != 0:
        print(f"Some structural tests failed with exit code {result.returncode}. Exiting.")
        sys.exit(result.returncode)
    else:
        print("All structural tests passed.")

if __name__ == "__main__":
    run_tests()