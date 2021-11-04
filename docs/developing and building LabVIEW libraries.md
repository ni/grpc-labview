# Required software:
- LabVIEW 2019
- VIPM 2020 community edition

# Developing "gRPC lv Support"

gRPC lv Support library doesnt have external dependency. The c dlls are referenced from the "Library" folder present along with this library.

Step 1: This library can be modified from source (LabVIEW source) location.

# Building "gRPC lv support"

The source location (LabVIEW source) contains the build spec (vipb) for the "grpc lv support". The build spec is from VIPM 2020 community edition.

Step 1: Open the LabVIEW gRPC Library.vipb file.

Step 2: Click Build

The built .vip file will be created in a build folder next to build spec folder. The built .vip file will have the new version in the name of the file.

# Installing the "gRPC lv support"

Step 1: Install the latest ni_lib_labview_grpc_library-x.x.x.x.vip using the VI Package Manager 2020.

The library will be installed under - "LabVIEW folder\vi.lib\gRPC\..". On installation, the "grpc-labview.lvlib" will be renamed as "grpc-labview-release.lvlib"

# Developing "Client Server Support"

This library is dependent on the installed "grpc-labview-release.lvlib". The c dlls are referenced from the "Library" folder along with this library.

Step 1: Install the latest "ni_lib_labview_grpc_library-x.x.x.x.vip". Refer above in case, if you need to develop or build a new package. Or download the latest package from the github "Release" location.

Step 2: Modify the library from the source (LabVIEW source) location.

# Building "Client Server Support"

The source location (LabVIEW source) contains the build spec (vipb) for the "Client Server Support". The build spec is from VIPM 2020 community edition.

Step 1: Open the gRPC Server and Client Template.vipb file.

Step 2: Click Build

The built .vip file will be created in a build folder next to build spec folder. The built .vip file will have the new version in the name of the file.

# Installing the "Client Server Support"

Step 1: Install the latest ni_lib_grpc_server_and_client_template-x.x.x.x.vip using the VI Package Manager 2020.

The library will be installed under - "LabVIEW folder\vi.lib\gRPC\..". On installation, the "gRPC Scripting Tools.lvlib" will be renamed as "gRPC Scripting Tools-release.lvlib"
