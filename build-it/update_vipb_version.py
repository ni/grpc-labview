import argparse
import os
import vipb_helper

def main():
    args = parse_args()
    build_script_directory = os.path.abspath(os.path.dirname(__file__))
    root_directory = os.path.dirname(build_script_directory)
    vipb_files =  vipb_helper.get_vipb_files(root_directory=root_directory)
    for vipb_file in vipb_files:
        vipb_helper.update_vipb_version(vipb_file=vipb_file, library_version=args.library_version)

def parse_args():
    parser = argparse.ArgumentParser(
            description="Parsing the script parameters"
        )
    parser.add_argument(
        "--library_version",
        required=True,
        default="",
        help="New Libray Version for the VIPB files",
    )
    return parser.parse_args()  

main()
