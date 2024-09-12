@echo off
pushd %~dp0
set script_dir=%CD%
popd

echo Searching for a virtual environment...
echo.

IF NOT exist %script_dir%\venv (
    echo Virtual Environment not found
    echo Creating Virtual Environment at \venv
    call python -m venv %script_dir%\venv
    echo Installing grpc and grpcio-tools into Virtual Environment
    call %script_dir%\venv\Scripts\python.exe -m pip install --upgrade pip
    call %script_dir%\venv\Scripts\python.exe -m pip install grpcio-tools==1.66.0 pytest
    echo Successfully Installed Virtual Environment
) ELSE (
    echo Virtual Environment found
)