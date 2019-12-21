@echo off
set BIN_DIR="bin"

if not defined VisualStudioVersion call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
