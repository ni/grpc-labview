import sys
import os
import distutils
import distutils.dir_util
import distutils.file_util
import subprocess

class LVgRPCBuilder:
    def __init__(self):
        self.build_script_directory = os.path.abspath(os.path.dirname(__file__))
        self.root_directory = os.path.dirname(self.build_script_directory)
        self.server_binary_destination = os.path.join(self.root_directory, "labview source", "gRPC lv Support")
        self.generator_binary_destination = os.path.join(self.root_directory, "labview source", "Client Server Support New", "gRPC Scripting Tools", "Proto Parser API")
        self.parse_configuration()

    def parse_configuration(self):
        self.build_target = None
        self.path_to_binaries = None
        self.vipb_build_only = True

        for i in range(1, len(sys.argv)):
            current_arg = sys.argv[i]

            if current_arg == "--target" or current_arg == "-t":
                self.build_target = sys.argv[i+1]
            elif current_arg == "--pathToBinaries" or current_arg == "-p":
                self.path_to_binaries = sys.argv[i+1]
            elif current_arg == "--buildcpp":
                self.vipb_build_only = False

        if self.build_target != "Win32" and self.build_target != "Win64" and self.build_target != "All":
            raise Exception("Build target should be one off Win32, Win64 or All. Passed build target is " + self.build_target)

        if self.build_target == "All" and self.path_to_binaries == None:
            raise Exception("For build target All a path to binaries should be specified using the --pathToBinaries/-p option")

    def get_cmake_args(self):
        if self.build_target == "Win32":
            return ["-A", "Win32", ".."]
        elif self.build_target == "Win64":
            return [".."]

    def cpp_build(self):
        cpp_build_directory = os.path.join(self.root_directory, "build");
        if os.path.exists(cpp_build_directory):
            distutils.dir_util.remove_tree(cpp_build_directory)
        os.makedirs(cpp_build_directory)
        os.chdir(cpp_build_directory)

        subprocess.run(["cmake"] + self.get_cmake_args())
        subprocess.run(["cmake", "--build",  ".",  "--config", "Release"])

    def copy_binaries_all_targets(self):
        server_dll_source = os.path.join(self.path_to_binaries, "LabVIEW gRPC Server")
        distutils.dir_util.copy_tree(server_dll_source, self.server_binary_destination)
        generator_dll_source = os.path.join(self.path_to_binaries, "LabVIEW gRPC Generator")
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

    def copy_built_binaries(self):
        if self.build_target == "All":
            self.copy_binaries_all_targets()
        else:
            self.copy_binaries_for_target()


    def build(self):
        if not self.build_target == "All" and not self.vipb_build_only:
            self.cpp_build()
        self.copy_built_binaries()
        build_vi_path = os.path.join(self.build_script_directory, "LV Build", "BuildGRPCPackages.vi")
        subprocess.run(["LabVIEWCLI", "-OperationName", "RunVI", "-VIPath", build_vi_path, os.path.join(self.root_directory, "labview source")])

def main():
    gRPCPackageBuilder = LVgRPCBuilder()
    gRPCPackageBuilder.build()

main()