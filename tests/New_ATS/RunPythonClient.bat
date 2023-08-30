@echo off
setlocal

set "python_client_path=%~1"
set "test_name=%~2"
set "rpc_name=%~3"

pushd %~dp0
set "script_dir=%CD%"
popd

if not exist "%script_dir%\logs" mkdir "%script_dir%\logs"
if not exist "%script_dir%\logs\%test_name%" mkdir "%script_dir%\logs\%test_name%"

call "%script_dir%\venv\Scripts\python.exe" -m pytest "%python_client_path%" --junitxml="%script_dir%\logs\%test_name%\%test_name%_%rpc_name%_test_results.xml" -vv

endlocal