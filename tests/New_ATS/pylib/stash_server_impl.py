import json
import pathlib
import shutil
import subprocess
import os
import sys

def backup_server_impl(test_folder_path):
    # check if the test_folder_path exists
    if not os.path.exists(test_folder_path):
        print(f'{test_folder_path} does not exist')
        return
    
    # check if the test_folder_path is a directory
    if not os.path.isdir(test_folder_path):
        print(f'{test_folder_path} is not a directory')
        return
    
    # check if the test_folder_path has a subfolder named Impl
    impl_path = os.path.join(test_folder_path, 'Impl')
    if not os.path.exists(impl_path):
        os.mkdir(impl_path)

    # copy the contents of the Generated_server folder to the Impl folder
    generated_server_path = os.path.join(test_folder_path, 'Generated_server')
    if not os.path.exists(generated_server_path):
        print(f'{generated_server_path} does not exist. Regenerate the server to backup')
        return
    
    run_service_path = os.path.join(generated_server_path, 'Run Service.vi')
    if not os.path.exists(run_service_path):
        print(f'{run_service_path} does not exist. Regenerate the server to backup')
        return
    
    # copy the run_service.vi at run_service_path to impl/run_service.vi
    shutil.copyfile(run_service_path, os.path.join(impl_path, 'Run Service.vi'))

    services = [filename for filename in os.listdir(os.path.join(generated_server_path, 'RPC Service'))]
    start_sync_path = os.path.join(generated_server_path, 'RPC Service', f'{services[0]}', 'Server API', 'Start Sync.vi')
    if not os.path.exists(start_sync_path):
        print(f'{start_sync_path} does not exist. Regenerate the server to backup')
        return
    
    # copy the start_sync.vi at start_sync_path to impl/start_sync.vi
    shutil.copyfile(start_sync_path, os.path.join(impl_path, 'Start Sync.vi'))

if __name__ == '__main__':
    # check if there are zero arguments
    if len(sys.argv) < 2:
        print("Usage stash_server_impl.py <test_folder_name>")
        exit(1)
    backup_server_impl(os.path.join(os.getcwd(), sys.argv[1]))
