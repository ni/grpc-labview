import glob
import os
import subprocess

def update_vipb_version(vipb_file, library_version):
    build_script_directory = os.path.abspath(os.path.dirname(__file__))
    build_vi_path = os.path.join(build_script_directory, "LV Build", "UpgradeVIPBVersion.vi")
    library_version = format_version(library_version)
    update_vipb_version = subprocess.run(["LabVIEWCLI", "-OperationName", "RunVI", "-VIPath", build_vi_path, vipb_file, library_version], capture_output = True)
    if(update_vipb_version.returncode != 0):
        raise Exception(update_vipb_version.stderr.decode())

def get_vipb_files(root_directory):
    labview_source_directory = os.path.join(root_directory, "labview source")
    vipb_paths = glob.glob(labview_source_directory + '**/**/*.vipb', recursive=True)
    return vipb_paths

# TODO: find a better way to handle this formatting
def format_version(input_version):
    if input_version !="" and input_version[0]=="v" :
        input_version = input_version[1:]
    return input_version
    
