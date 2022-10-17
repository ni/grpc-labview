import argparse
import glob
import os
import subprocess

def main():
    args = parse_args()
    build_script_directory = os.path.abspath(os.path.dirname(__file__))
    root_directory = os.path.dirname(build_script_directory)
    build_vi_path = os.path.join(build_script_directory, "LV Build", "UpgradeVIPBVersion.vi")
    vipbpaths = get_vipb_files(root_directory = root_directory)
    for x in vipbpaths:
        update_vipb_version = subprocess.run(["LabVIEWCLI", "-OperationName", "RunVI", "-VIPath", build_vi_path, x, args.library_version], capture_output = True)
        if(update_vipb_version.returncode != 0):
            raise Exception(update_vipb_version.stderr.decode())

def parse_args():
    parser = argparse.ArgumentParser(
        description="Parsing the script parameters"
    )
    parser.add_argument(
        "--library_version",
        help="Release Library Version",
        default="0.8.0.1"
    )
    return parser.parse_args()

def get_vipb_files(root_directory):
        labview_source_directory = os.path.join(root_directory, "labview source")
        vipb_paths = glob.glob(labview_source_directory + '**/**/*.vipb', recursive=True)
        return vipb_paths

main()