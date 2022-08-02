@ECHO OFF
IF "%~1" == "" (
    ECHO No Build Configuration specified. Specify Win32 or Win64.
    goto end
)

SET BuildConfiguration=%1
SET CPPBUILDFOLDER=build
SET ROOTFOLDER=%~dp0\..\
SET LVSUPPORTFOLDER="labview source\gRPC lv Support"
SET SERVERDLL=labview_grpc_server.dll
SET GENERATORDLL_COPY_PATH="labview source\Client Server Support New\gRPC Scripting Tools\Proto Parser API"
SET GENERATORDLL=labview_grpc_generator.dll

IF %BuildConfiguration% == Win32 goto ConfigureWin32
IF %BuildConfiguration% == Win64 goto ConfigureWin64

ECHO Build Configuration %BuildConfiguration% is invalid.
goto end

:ConfigureWin32
SET CMAKEARGS=-A Win32 ..
goto Build

:ConfigureWin64
SET CMAKEARGS=..
goto Build

:Build
IF "%~2" == "LV_ONLY" goto LV_BUILD

cd %ROOTFOLDER%
git submodule update --init --recursive

IF EXIST %CPPBUILDFOLDER% RMDIR /s /q %CPPBUILDFOLDER%
mkdir %CPPBUILDFOLDER%
chdir %CPPBUILDFOLDER%
cmake %CMAKEARGS%
cmake --build . --config Release
goto LV_BUILD

:LV_BUILD
cd %ROOTFOLDER%
cd %LVSUPPORTFOLDER%
mkdir Libraries\%BuildConfiguration%
xcopy %ROOTFOLDER%\%CPPBUILDFOLDER%\Release\%SERVERDLL% Libraries\%BuildConfiguration% /y

cd %ROOTFOLDER%
cd %GENERATORDLL_COPY_PATH%
mkdir Libraries\%BuildConfiguration%
xcopy %ROOTFOLDER%\%CPPBUILDFOLDER%\Release\%GENERATORDLL% Libraries\%BuildConfiguration% /y

:end
