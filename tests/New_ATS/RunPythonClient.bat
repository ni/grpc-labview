@echo off
set python_client_path=%1
pushd %~dp0
set script_dir=%CD%
popd

echo Running Python Client
echo %script_dir%\venv\Scripts\python.exe -m pytest %python_client_path% -vv
call %script_dir%\venv\Scripts\python.exe -m pytest %python_client_path% -vv