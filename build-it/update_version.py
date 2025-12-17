import argparse
import os


def main():
    args = parse_args()
    build_script_directory = os.path.abspath(os.path.dirname(__file__))
    version_file_path = os.path.join(build_script_directory, "VERSION")

    # Write the version to the VERSION file
    with open(version_file_path, "w") as version_file:
        version_file.write(f"version={args.library_version}\n")


def parse_args():
    parser = argparse.ArgumentParser(description="Parsing the script parameters")
    parser.add_argument(
        "--library_version",
        required=True,
        default="",
        help="New Libray Version for the VIPB files",
    )
    return parser.parse_args()


main()
