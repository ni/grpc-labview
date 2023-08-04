import json
import pathlib
import shutil
import subprocess
import os
import re

FAILED = 0

def count_failed_testcases(test_summary):
    pattern = r'(\d+) failed(?=, \d+ passed)'
    matches = re.findall(pattern, test_summary)
    count = 0
    if matches:
        count = int(matches[-1])
    return count


def run_command(command):
    output = subprocess.run(command, capture_output=True, text=True)
    if output.stderr:
        raise Exception(output.stderr)
    return output.stdout


def check_for_pre_requisites(test_config):
    # check if LabVIEW CLI is installed
    if not test_config["lvcli_path"].exists():
        raise Exception(f'LabVIEW CLI is not installed at {test_config["lvcli_path"]}')


def generate_server(test_config):
    # 1. Delete the Generated_server folder. TODO: Take a boolean in the config to say whether the build should be a clean build
    if test_config['generated_server'].exists():
        shutil.rmtree(test_config['generated_server'])

    # 2. Generate the server
    # todo: call the LabVIEW VI from LabVIEW CLI
    main_wrapper_vi_path = test_config['test_suite_folder'] / 'Main_CLIWrapper.vi'
    # subprocess.run([f'{labviewCLI_path} -OperationName RunVI',
    CLI_command = ' '.join([
        f'{test_config["lvcli_path"]}',
        f'-LabVIEWPath "{test_config["labview_path"]}"',
        '-OperationName RunVI',
        f"-VIPath {main_wrapper_vi_path}",
        f"{test_config['proto_path']}",
        f"{test_config['project_path']}",
        f"{test_config['gen_type']}"])
    run_command(CLI_command)


def run_test(test_config):
    global FAILED

    # 1. Check for pre_requisites
    check_for_pre_requisites(test_config)

    # 2. Generate the server
    generate_server(test_config)

    # 3. Copy the 'Run Service.vi' from the Impl folder to the Generated_server folder
    run_service_impl_path = test_config['impl'] / 'Run Service.vi'
    run_service_gen_path = test_config['generated_server'] / 'Run Service.vi'
    shutil.copyfile(run_service_impl_path, run_service_gen_path)

    # 4. Copy the 'Start Sync.vi' from the Impl folder to the "Generated_server/RPC Service/GreeterService/Server API" folder
    start_sync_impl_path = test_config['impl'] / 'Start Sync.vi'
    start_sync_gen_path = test_config['generated_server'] / 'RPC Service' / 'GreeterService' / 'Server API' / 'Start Sync.vi'
    shutil.copyfile(start_sync_impl_path, start_sync_gen_path)

    # 5. Quit LabVIEW if it is running
    run_command(['taskkill', '/f', '/im', 'labview.exe'])

    # 6. Start Run Service.vi from command prompt by launching labview.exe form lv_folder with the following arguments:
    # this must be non-blocking
    # subprocess.Popen([str(labview_path), str(test_config['generated_server'] / 'Run Service.vi')])     # shoud be non-blocking
    # TODO Need to add support for passing multiple protofiles
    runservice_wrapper_vi_path = test_config['test_suite_folder'] / 'RunService_CLIWrapper.vi'
    CLI_command = ' '.join([
        f'{test_config["lvcli_path"]}',
        f'-LabVIEWPath "{test_config["labview_path"]}"',
        '-OperationName RunVI',
        f"-VIPath {runservice_wrapper_vi_path}",
        f"{test_config['test_folder']}"])

    # TODO Check whether labviewCLI is installed or not before running the command
    run_command(CLI_command)

    # 7. Create python virtual environment
    CLI_command = ' '.join([
        str(test_config['test_suite_folder'] / 'CreatePythonVirtualEnv.bat')
    ])
    run_command(CLI_command)

    # 7. Generate python grpc classes
    generate_command = ' '.join([
        f"{test_config['python_path']} -m grpc_tools.protoc",
        f"--proto_path={test_config['test_folder']}",
        f"--python_out={test_config['test_folder']}",
        f"--pyi_out={test_config['test_folder']}",
        f"--grpc_python_out={test_config['test_folder']}",
        f"{test_config['test_name']}.proto"
    ])
    run_command(generate_command)

    # 8. Call the TestServer() from test_folder/test_name_client.py
    client_py_path = test_config['test_folder'] / str(test_config['test_name'] + '_client.py')
    run_client_command = ' '.join([
        str(test_config['test_suite_folder'] / 'RunPythonClient.bat'),
        str(client_py_path)])
    output = run_command(run_client_command)
    print(output)
    FAILED += count_failed_testcases(output)

    # 8. Quit LabVIEW if it is running
    run_command(['taskkill', '/f', '/im', 'labview.exe'])

    # 9. Delete python grpc generated files
    # for filename in os.listdir(test_config['test_folder']):
    #     if "pb2" in filename:
    #         os.remove(test_config['test_folder'] / filename)


# Desc: Run the tests in the testlist.json file
def main():
    global FAILED
    # read the list of tests from testlist.json
    test_list_json_path = pathlib.Path(__file__).parent.absolute() / 'testlist.json'
    with open(test_list_json_path) as f:
        tests = json.load(f)
        for test in tests:
            test_config = {}
            gen_type = test["gen_type"].strip()
            labview_version = test["labview_version"].strip()
            labview_bitness = test["labview_bitness"].strip()
            # globals
            program_files_w_bitness = 'C:\\Program Files'
            if labview_bitness == '32':
                program_files_w_bitness += ' (x86)'
            test_config['lv_folder'] = pathlib.Path(program_files_w_bitness + f'\\National Instruments\\LabVIEW {labview_version}')
            test_config['labview_path'] = test_config['lv_folder'] / 'labview.exe'
            test_config['lvcli_path'] = test_config['lv_folder'].parent.absolute() / 'Shared' / 'LabVIEW CLI' / 'LabVIEWCLI.exe'
            test_config['test_suite_pylib_folder'] = pathlib.Path(__file__).parent.absolute()
            test_config['test_suite_folder'] = test_config['test_suite_pylib_folder'].parent.absolute()
            test_config['tests_folder'] = test_config['test_suite_folder'] / 'Tests'
            test_config['python_path'] = test_config['test_suite_folder'] / 'venv' / "Scripts" / "python.exe"
            tests_folder = test_config['tests_folder']
            # locals
            for test_name in test['name']:
                print(f'\nRunning test for "{test_name}"...')
                test_config['test_name'] = test_name.strip()
                test_config['test_folder'] = tests_folder / test_name
                test_config['proto_path'] = test_config['test_folder'] / str(test_name + '.proto')
                test_config['project_path'] = test_config['test_folder'] / str(test_name + '.lvproj')
                generated_server_path = test_config['test_folder'] / 'Generated_server'
                test_config['generated_server'] = generated_server_path
                test_config['impl'] = test_config['test_folder'] / 'Impl'
                test_config['gen_type'] = gen_type
                run_test(test_config)
            if FAILED:
                raise Exception(f"{FAILED} test cases have failed. Please review the above results")            

if __name__ == '__main__':
    main()
