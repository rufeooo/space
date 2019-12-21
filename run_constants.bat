@echo off
SET BIN_DIR="bin"

IF NOT DEFINED VisualStudioVersion call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
IF NOT EXIST "%BIN_DIR%" mkdir "%BIN_DIR%"
