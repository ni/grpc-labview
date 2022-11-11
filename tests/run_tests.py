
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
    traverse_tests()

def traverse_tests():
    test_directory = os.path.abspath(os.path.dirname(__file__))
    _logger.debug(f"Finding tests in {test_directory}...")
    WrapperVI = os.path.join(test_directory, "LV_ATS\\gRPC_ATS.vi")
    allTestJSONs = os.path.join(test_directory , "Tests.lst")
    failed_flag = ""
    with open(allTestJSONs, 'r') as file:
        alltests = file.readlines()
        for test in alltests:
            failed_flag += runTest(WrapperVI, test)
        if (failed_flag != ""):
            raise Exception(failed_flag)

def runTest(WrapperVI, testVI):
    testResult = subprocess.run(["LabVIEWCLI", "-OperationName", "RunVI", "-VIPath", os.path.normpath(WrapperVI), testVI], capture_output= True)
    if(testResult.returncode == 0):
        _logger.debug(f"[PASSED] {testVI}")
    else:
        _logger.exception(f"[FAILED] {testVI} has failed with {testResult.stdout.decode()} {testResult.stderr.decode()}")
        return (testResult.stdout.decode() + "\n" + testResult.stderr.decode() + "\n")
main()
