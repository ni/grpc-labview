@echo off
set python_client_path=%1
set test_name=%2
pushd %~dp0
set script_dir=%CD%
popd

echo Running Python Client
if not exist "./logs" mkdir logs
call %script_dir%\venv\Scripts\python.exe -m pytest %python_client_path% --junitxml=%script_dir%/logs/%test_name%_test_results.xml -vv