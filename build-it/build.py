import argparse
import distutils
import distutils.dir_util
import distutils.file_util
import os
import subprocess

import vipb_helper


class LVgRPCBuilder:
    def __init__(self):
        self.build_script_directory = os.path.abspath(os.path.dirname(__file__))
        self.root_directory = os.path.dirname(self.build_script_directory)
        self.server_binary_destination = os.path.join(
            self.root_directory, "labview source", "gRPC lv Support"
        )
        self.generator_binary_destination = os.path.join(
            self.root_directory,
            "labview source",
            "Client Server Support New",
            "gRPC Scripting Tools",
            "Proto Parser API",
        )
        self.version_file_path = os.path.join(self.build_script_directory, "VERSION")

    # Notes on how the --libraryVersion and --lib-version args are used:
    # --libraryVersion is used to update the vipb files before building the vipkgs
    #   --libraryVersion will be null when we are building vipbs for Pull Requests/Testing
    # If --libraryVersion is provided, we will first update the VERSION file with that value
    # --lib-version is provided as a way for testing to set the version in the vipb files without updating the VERSION file
    # It seems there is some duplication in how the version is handled, but this is to maintain compatibility with existing build and test processes

    def parse_args(self):
        parser = argparse.ArgumentParser(description="Parsing the script parameters")
        parser.add_argument("--libraryVersion", help="release tag", default="")
        parser.add_argument(
            "--lib-version",
            help="Library version to set in the vipb files",
            default="",
        )
        parser.add_argument(
            "--target",
            help="Build targets",
            default="",
        )
        parser.add_argument(
            "--pathToBinaries",
            help="Path to the pre-built binaries",
            default="",
        )
        parser.add_argument("--buildcpp", help="Build cpp", action="store_true")
        parser.add_argument(
            "--labview-version",
            help="Specify the Labview version by year using either 2 or 4 digits. For example, 19, 2019, 2023",
            default="2019",
        )
        parser.add_argument(
            "--labview-port",
            help="LabVIEW port. The default of Auto will attempt to read the port from the LabVIEW.ini or specify this to override.",
            default="Auto",
        )
        parser.add_argument(
            "--labview-bits", help="LabVIEW bitness (32 or 64)", default="32"
        )
        return parser.parse_args()

    def get_cmake_args(self, args):
        if args.target == "Win32":
            return ["-A", "Win32", ".."]
        elif args.target == "Win64":
            return [".."]

    def cpp_build(self, args):
        cpp_build_directory = os.path.join(self.root_directory, "build")
        if os.path.exists(cpp_build_directory):
            distutils.dir_util.remove_tree(cpp_build_directory)
        os.makedirs(cpp_build_directory)
        os.chdir(cpp_build_directory)

        subprocess.run(["cmake"] + self.get_cmake_args(args))
        subprocess.run(["cmake", "--build", ".", "--config", "Release"])

    def copy_binaries_all_targets(self, args):
        server_dll_source = os.path.join(args.pathToBinaries, "LabVIEW gRPC Server")
        distutils.dir_util.copy_tree(server_dll_source, self.server_binary_destination)
        generator_dll_source = os.path.join(
            args.pathToBinaries, "LabVIEW gRPC Generator"
        )
        distutils.dir_util.copy_tree(
            generator_dll_source, self.generator_binary_destination
        )

    def copy_binaries_for_target(self, args):
        server_dll_source = os.path.join(
            self.root_directory, "build", "Release", "labview_grpc_server.dll"
        )
        server_dll_destination = os.path.join(
            self.server_binary_destination, "Libraries", args.target
        )
        if not os.path.exists(server_dll_destination):
            os.makedirs(server_dll_destination)
        distutils.file_util.copy_file(server_dll_source, server_dll_destination)

        generator_dll_source = os.path.join(
            self.root_directory, "build", "Release", "labview_grpc_generator.dll"
        )
        generator_dll_destination = os.path.join(
            self.generator_binary_destination, "Libraries", args.target
        )
        if not os.path.exists(generator_dll_destination):
            os.makedirs(generator_dll_destination)
        distutils.file_util.copy_file(generator_dll_source, generator_dll_destination)

    def copy_built_binaries(self, args):
        if args.target == "All":
            self.copy_binaries_all_targets(args)
        else:
            self.copy_binaries_for_target(args)

    def get_labview_exe_path(self, args):
        # Convert 2-digit year to 4-digit year if necessary
        if len(args.labview_version) == 2:
            year = "20" + args.labview_version
        else:
            year = args.labview_version

        # Determine Program Files folder based on bitness
        if args.labview_bits == "32":
            program_files = "Program Files (x86)"
        else:
            program_files = "Program Files"

        # Construct the path to LabVIEW.exe
        labview_path = os.path.join(
            "C:\\",
            program_files,
            "National Instruments",
            f"LabVIEW {year}",
            "LabVIEW.exe",
        )

        # Check if the path exists
        if not os.path.exists(labview_path):
            raise Exception(
                f"LabVIEW {year} ({args.labview_bits}-bit) not found at {labview_path}"
            )

        return labview_path

    def get_labview_port(self, labview_exe_path, args):
        if args.labview_port.lower() == "auto":
            # Replace .exe with .ini to get the ini file path
            labview_ini_path = labview_exe_path.replace(".exe", ".ini")

            if not os.path.exists(labview_ini_path):
                raise Exception(f"LabVIEW.ini not found at {labview_ini_path}")

            with open(labview_ini_path, "r") as ini_file:
                for line in ini_file:
                    if line.startswith("server.tcp.port="):
                        return str(line.split("=")[1].strip())
            raise Exception("server.tcp.port not found in LabVIEW.ini")
        else:
            return str(args.labview_port)

    def build(self, args):
        if not args.target == "All" and args.buildcpp:
            self.cpp_build(args)
        self.copy_built_binaries(args)

        build_vi_path = os.path.join(
            self.build_script_directory, "LV Build", "BuildGRPCPackages.vi"
        )
        build_output_path = os.path.join(
            self.root_directory, "labview source", "Builds"
        )
        build_labview_path = self.get_labview_exe_path(args)
        labview_port = self.get_labview_port(build_labview_path, args)

        print(
            f"Using LabVIEW at {build_labview_path} on port {labview_port} to build vipkgs"
        )

        # Before building vipkgs, ensure the output directory is empty
        if os.path.exists(build_output_path):
            distutils.dir_util.remove_tree(build_output_path)
        os.makedirs(build_output_path)

        # Build the packages using LabVIEWCLI
        build_vipkgs = subprocess.run(
            [
                "LabVIEWCLI",
                "-LabVIEWPath",
                str(build_labview_path),
                "-PortNumber",
                str(labview_port),
                "-OperationName",
                "RunVI",
                "-VIPath",
                build_vi_path,
                os.path.join(self.root_directory, "labview source"),
                str(args.labview_version),
                str(args.labview_bits),
                args.lib_version,
            ],
            capture_output=True,
        )

        # Close the LabVIEW instance after building
        subprocess.run(
            [
                "LabVIEWCLI",
                "-LabVIEWPath",
                str(build_labview_path),
                "-PortNumber",
                str(labview_port),
                "-OperationName",
                "CloseLabVIEW",
            ],
            capture_output=False,
        )

        if build_vipkgs.returncode != 0:
            raise Exception(f"Failed to Build vipkgs {build_vipkgs.stderr.decode()}")


# TODO: find a better way to handle this formatting
# This was copied from the vipb_helper module for consistency
def format_version(input_version):
    if input_version != "" and input_version[0] == "v":
        input_version = input_version[1:]
    return input_version


def read_version_from_file(version_file_path):
    """Reads the version from the VERSION file by parsing key=value format."""
    with open(version_file_path, "r") as version_file:
        for line in version_file:
            line = line.strip()
            if line.startswith("version="):
                return line.split("=", 1)[1]
    raise Exception("version= key not found in VERSION file")


def main():
    gRPCPackageBuilder = LVgRPCBuilder()
    args = gRPCPackageBuilder.parse_args()
    used_lib_version = ""

    # libraryVersion will be null when we are building vipbs for Pull Requests/Testing
    if args.libraryVersion != "":
        # ########## Old code ######################################################
        # vipb_files = vipb_helper.get_vipb_files(gRPCPackageBuilder.root_directory)
        # for vipb_file in vipb_files:
        # vipb_helper.update_vipb_version(vipb_file = vipb_file, library_version = args.libraryVersion)

        # This means we should update VERSION file for a new release
        used_lib_version = format_version(args.libraryVersion)
        with open(gRPCPackageBuilder.version_file_path, "w") as version_file:
            version_file.write(f"version={args.libraryVersion}\n")
    elif args.used_lib_version != "":
        # This means we are overriding the version from the command line for testing purposes
        used_lib_version = format_version(args.lib_version)
    else:
        # Read the version from the VERSION file
        used_lib_version = read_version_from_file(gRPCPackageBuilder.version_file_path)
    # now that we have determined the version to use, we need to update this in the args for the build command
    args.lib_version = used_lib_version

    if args.target != "Win32" and args.target != "Win64" and args.target != "All":
        raise Exception(
            "Build target should be one off Win32, Win64 or All. Passed build target is "
            + args.target
        )

    if args.target == "All" and args.pathToBinaries == "":
        raise Exception(
            "For build target All a path to binaries should be specified using the --pathToBinaries option"
        )

    gRPCPackageBuilder.build(args)


main()
