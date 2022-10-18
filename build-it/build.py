import argparse
import os
import distutils
import distutils.dir_util
import distutils.file_util
import subprocess
import vipb_helper

class LVgRPCBuilder:

    def __init__(self):
        self.build_script_directory = os.path.abspath(os.path.dirname(__file__))
        self.root_directory = os.path.dirname(self.build_script_directory)
        self.server_binary_destination = os.path.join(self.root_directory, "labview source", "gRPC lv Support")
        self.generator_binary_destination = os.path.join(self.root_directory, "labview source", "Client Server Support New", "gRPC Scripting Tools", "Proto Parser API")
    
    def parse_args(self):
        parser = argparse.ArgumentParser(
            description="Parsing the script parameters"
        )
        parser.add_argument(
            "--libraryVersion",
            help="release tag",
            default=""
        )
        parser.add_argument(
            "--target",
            help="Build targets",
            default="",
        )
        parser.add_argument(
            "--pathToBinaries",
            help="Path to the pre-built binaries",
            default="C:",
        )
        parser.add_argument(
            "--buildcpp",
            help="Build cpp",
            default=False,
        )

        return parser.parse_args()

    def get_cmake_args(args):
        if args.target == "Win32":
            return ["-A", "Win32", ".."]
        elif args.target == "Win64":
            return [".."]

    def cpp_build(self, args):
        cpp_build_directory = os.path.join(self.root_directory, "build");
        if os.path.exists(cpp_build_directory):
            distutils.dir_util.remove_tree(cpp_build_directory)
        os.makedirs(cpp_build_directory)
        os.chdir(cpp_build_directory)

        subprocess.run(["cmake"] + self.get_cmake_args(args))
        subprocess.run(["cmake", "--build",  ".",  "--config", "Release"])

    def copy_binaries_all_targets(self, args):
        server_dll_source = os.path.join(args.pathToBinaries, "LabVIEW gRPC Server")
        distutils.dir_util.copy_tree(server_dll_source, self.server_binary_destination)
        generator_dll_source = os.path.join(args.pathToBinaries, "LabVIEW gRPC Generator")
        distutils.dir_util.copy_tree(generator_dll_source, self.generator_binary_destination)

    def copy_binaries_for_target(self):
        server_dll_source = os.path.join(self.root_directory, "build", "Release", "labview_grpc_server.dll")
        server_dll_destination = os.path.join(self.server_binary_destination, "Libraries", self.build_target)
        if not os.path.exists(server_dll_destination):
            os.makedirs(server_dll_destination)
        distutils.file_util.copy_file(server_dll_source, server_dll_destination)

        generator_dll_source = os.path.join(self.root_directory, "build", "Release", "labview_grpc_generator.dll")
        generator_dll_destination = os.path.join(self.generator_binary_destination, "Libraries", self.build_target)
        if not os.path.exists(generator_dll_destination):
            os.makedirs(generator_dll_destination)
        distutils.file_util.copy_file(generator_dll_source, generator_dll_destination)

    def copy_built_binaries(self, args):
        if args.target == "All":
            self.copy_binaries_all_targets(args)
        else:
            self.copy_binaries_for_target()

    def build(self, args):
        if not args.target == "All" and not args.buildcpp:
            self.cpp_build(args)
        self.copy_built_binaries(args)
        build_vi_path = os.path.join(self.build_script_directory, "LV Build", "BuildGRPCPackages.vi")
        build_vipkgs = subprocess.run(["LabVIEWCLI", "-OperationName", "RunVI", "-VIPath", build_vi_path, os.path.join(self.root_directory, "labview source")], capture_output = True)
        if (build_vipkgs.returncode != 0):
            raise Exception(f'Failed to Build vipkgs { build_vipkgs.stderr.decode() }')

def main():
    gRPCPackageBuilder = LVgRPCBuilder()
    args = gRPCPackageBuilder.parse_args()
    
    # libraryVersion will be null when we are build vipbs for Pull Requests/Testing
    if args.libraryVersion != "" :
        vipb_files = vipb_helper.get_vipb_files(gRPCPackageBuilder.root_directory)
        for vipb_file in vipb_files:
            vipb_helper.update_vipb_verion(vipb_file = vipb_file, library_version = args.libraryVersion)

    if args.target != "Win32" and args.target != "Win64" and args.target != "All":
            raise Exception("Build target should be one off Win32, Win64 or All. Passed build target is " + args.target)

    if args.target == "All" and args.pathToBinaries == "":
            raise Exception("For build target All a path to binaries should be specified using the --pathToBinaries option")

    gRPCPackageBuilder.build(args)

main()
