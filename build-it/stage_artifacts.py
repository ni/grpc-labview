import argparse
import logging
import os
from pathlib import Path
import sys
import tarfile
from zipfile import ZipFile
import shutil

_logger = logging.getLogger(__name__)
_logger.setLevel(logging.DEBUG)

handler = logging.StreamHandler(sys.stdout)
handler.setLevel(logging.DEBUG)
_logger.addHandler(handler)

def main():
    args = parse_args()
    _logger.debug(f"Downloaded artifacts are at {os.listdir(args.downloaded_path)}")
    # args.downloaded_path contains artifact folders(each artifact is downloaded as a folder of its own)
    artifact_folders = [os.path.join(args.downloaded_path, x) for x in os.listdir(args.downloaded_path)]
    stage_artifacts_for_export(artifact_folders, args.staging_path)

def parse_args():
    parser = argparse.ArgumentParser(
        description="Parsing the script parameters"
    )
    parser.add_argument(
        "--downloaded_path",
        help="artifact downloaded path",
        default="C:/abahetik/grpc-labview-artifacts",
    )
    parser.add_argument(
        "--staging_path",
        help="Artifact staging directory",
        default="C:/abahetik/grpc-labview-staged-artifacts"
    )
    _logger.debug(f"The parameters for the script are: {parser.parse_args()}")
    return parser.parse_args()

def stage_artifacts_for_export(artifact_folders, input_staging_directory):
    tar_staging_directory = prepare_directory(parent_directory=input_staging_directory, sub_directory="Tar_Staging_Directory")
    for artifact_folder in artifact_folders:
        _logger.debug(f"copy contents of {artifact_folder} to {tar_staging_directory}")
        copy_tar_artifacts(artifact_folder, tar_staging_directory)
    stage_artifacts(input_staging_directory, tar_staging_directory)

# add a comment this assumes that each artifact folder has only one tar.gz
def copy_tar_artifacts(artifact_folder, tar_staging_directory):
    tar_artifact = os.listdir(artifact_folder)
    shutil.copy(Path(artifact_folder)/tar_artifact[0], tar_staging_directory)

def prepare_directory(parent_directory, sub_directory):
    new_directory = Path(parent_directory) / sub_directory
    if not os.path.exists(new_directory):
        os.makedirs(new_directory, exist_ok=True)
    return new_directory

def stage_artifacts(working_directory, tar_staging_directory):
    tar_artifacts = os.listdir(tar_staging_directory)
    top_level_dll_directory = prepare_directory(working_directory, sub_directory="TopLevelDlls")
    for tar_artifact in tar_artifacts:
        extract_tarfile(tar_artifact, tar_staging_directory, top_level_dll_directory)

def extract_tarfile(tar_file_name, tar_staging_directory, top_level_dll_directory):
    _logger.debug(f"Creating folder Heirarchy for Server Dlls...")
    _logger.debug(f"extracing tar file {tar_file_name}...")
    with tarfile.open(Path(tar_staging_directory) / tar_file_name) as tar_file:
        for artifact in tar_file.getmembers():
            if(tar_file_name.find('x64') != -1):
                bitness_type = 'Win64'
            elif(tar_file_name.find('x86') != -1):
                bitness_type = 'Win32'
            elif(tar_file_name.find('linux') != -1):
                bitness_type = 'Linux'
            elif(tar_file_name.find('rt') != -1):
                bitness_type = 'LinuxRT'
            extract_and_stage_artifact(artifact, tar_file, bitness_type, top_level_dll_directory)
        _logger.debug(f"extracted {tar_file.getnames()} to {tar_staging_directory}")

def extract_and_stage_artifact(artifact, tar_file, bitness_type, top_level_dll_directory):
    server_folder = prepare_directory(top_level_dll_directory, sub_directory="LabVIEW gRPC Server/Libraries")
    generator_folder = prepare_directory(top_level_dll_directory, sub_directory="LabVIEW gRPC Generator/Libraries")
    if(artifact.name.find('server') != -1 ):
        extract_directory = (Path(server_folder) / bitness_type)
    elif(artifact.name.find('generator') != -1):
        extract_directory = (Path(generator_folder) / bitness_type)
    os.makedirs(extract_directory, exist_ok=True)
    tar_file.extract(artifact, extract_directory)

if __name__ == "__main__":
    main()
