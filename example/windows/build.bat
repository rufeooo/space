@echo off

REM If VisualStudioVersion not defined run vcvarsall
IF NOT DEFINED VisualStudioVersion call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

cl win32_main.cc /std:c++latest /nologo /DUNICODE /link user32.lib opengl32.lib gdi32.lib
