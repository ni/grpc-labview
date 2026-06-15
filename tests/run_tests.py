
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
            _logger.error(failed_test_results)
            sys.exit(1)

def run_test(WrapperVI, testVI):
    if os.path.exists(testVI):
        testResult = subprocess.Popen(["LabVIEWCLI", "-OperationName", "RunVI", "-VIPath", os.path.normpath(WrapperVI), testVI],
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            shell=False)


        try:
            out, err = testResult.communicate(timeout=60) # TODO: find if this timeout can be eliminated
        except subprocess.TimeoutExpired:
                with testResult:
                    testResult.kill()
                out, err = testResult.communicate()

        if(testResult.returncode == 0):
            _logger.debug(f"[PASSED] {testVI}")
            return
        else:
            stdout_output = out.decode(errors="replace")
            stderr_output = err.decode(errors="replace")
            failed_test_output = f"[FAILED] {testVI} has failed\n"

            if stdout_output.strip() != "":
                failed_test_output += f"stdout:\n{stdout_output}\n"

            if stderr_output.strip() != "":
                failed_test_output += f"stderr:\n{stderr_output}\n"

            _logger.error(failed_test_output)
            return failed_test_output
    else:
        _logger.error(f"[FAILED] {testVI} \n {testVI} doesnot exist.")
        return testVI + " doesnot exist. \n"

main()
