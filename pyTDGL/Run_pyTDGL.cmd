@echo off
set "PROJECT_ROOT=%~dp0"
set "PYTDGL_PY=%USERPROFILE%\.venvs\pyTDGL\Scripts\python.exe"

cd /d "%PROJECT_ROOT%"

"%PYTDGL_PY%" -m notebook "%PROJECT_ROOT%docs\notebooks\quickstart.ipynb"
pause
