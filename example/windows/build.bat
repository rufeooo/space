@echo off

REM If VisualStudioVersion not defined run vcvarsall
IF NOT defined VisualStudioVersion call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

cl win32_main.cc /nologo /DUNICODE /link user32.lib
