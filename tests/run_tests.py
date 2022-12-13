
import logging
import os
import subprocess
import sys

_logger = logging.getLogger(__name__)
_logger.setLevel(logging.DEBUG)

handler = logging.StreamHandler(sys.stdout)
handler.setLevel(logging.DEBUG)
_logger.addHandler(handler)

def main():
    _logger.debug(f"Looking for tests")
    run_all_tests()

def run_all_tests():
    test_directory = os.path.abspath(os.path.dirname(__file__))
    _logger.debug(f"Running tests in {test_directory}...")
    test_runner_vi = os.path.join(test_directory, "gRPC_ATS\\gRPC_ATS.vi")
    test_file = os.path.join(test_directory , "Tests.lst")
    failed_test_results = ""
    with open(test_file, 'r') as file:
        all_tests = file.read().splitlines()
        for test in all_tests:
            run_result = run_test(test_runner_vi, os.path.join(test_directory, test))
            # run_result will be None for PASSED tests
            if(run_result != None):
                failed_test_results += run_result 
        if (failed_test_results != ""):
            _logger.exception(failed_test_results)

def run_test(WrapperVI, testVI):
    if os.path.exists(testVI):
        testResult = subprocess.run(["LabVIEWCLI", "-OperationName", "RunVI", "-VIPath", os.path.normpath(WrapperVI), testVI], capture_output= True)
        if(testResult.returncode == 0):
            _logger.debug(f"[PASSED] {testVI}")
            return
        else:
            _logger.error(f"[FAILED] {testVI} has failed \n {testResult.stderr.decode()}")
            return (testResult.stderr.decode() + "\n")
    else:
        _logger.error(f"[FAILED] {testVI} \n {testVI} doesnot exist.")
        return testVI + " doesnot exist. \n"

main()
