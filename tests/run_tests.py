
import glob
import json
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
    allTestJSONs = glob.glob(test_directory + '/*.json', recursive=True)
    for testJSON in allTestJSONs:
        WrapperVI, testVI =  readTestVILocationForJson(testJSON)
        _logger.debug(f"Running {testVI}")
        runTest(WrapperVI, testVI)

def runTest(WrapperVI, testVI):
    testResult = subprocess.run(["LabVIEWCLI", "-OperationName", "RunVI", "-VIPath", os.path.normpath(WrapperVI), testVI], capture_output= True)
    if(testResult.returncode == 0):
        _logger.debug(f"[PASSED] {testVI} ")
    else:
        _logger.debug(f"[FAILED] {testVI} has failed with {testResult.stdout.decode()}")

def readTestVILocationForJson(testJSON):
    #read json file
    with open(testJSON, 'r') as myfile:
        data = myfile.read()    
    jsonObj = json.loads(data)
    _logger.debug(f"JOSN object = {jsonObj}")
    WrapperVI = str(jsonObj[0]["VI Specs"][0]["VI Path"])
    _logger.debug(f"Wrapper VI: {WrapperVI}")
    TestVI = str(jsonObj[0]["VI Specs"][0]["Test Json"]["VI path"])
    _logger.debug(f"Test VI {TestVI}")
    return WrapperVI, TestVI

main()
