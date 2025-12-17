import argparse
import os


# TODO: find a better way to handle this formatting
def format_version(input_version):
    if input_version != "" and input_version[0] == "v":
        input_version = input_version[1:]
    return input_version


def main():
    args = parse_args()
    build_script_directory = os.path.abspath(os.path.dirname(__file__))
    version_file_path = os.path.join(build_script_directory, "VERSION")

    # Write the version to the VERSION file
    with open(version_file_path, "w") as version_file:
        version_file.write(f"version={format_version(args.library_version)}\n")


def parse_args():
    parser = argparse.ArgumentParser(description="Parsing the script parameters")
    parser.add_argument(
        "--library_version",
        required=True,
        default="",
        help="New Library Version to write to VERSION file",
    )
    return parser.parse_args()


main()
