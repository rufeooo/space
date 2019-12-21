call run_constants.bat

set file_arg=%1
for /F "delims=" %%i in ("%file_arg%") do set filename="%%~ni"

CL %1 /nologo /std:c++17 /I "src\" /Fo:%BIN_DIR%\ /DUNICODE /link user32.lib opengl32.lib gdi32.lib /OUT:%BIN_DIR%/%filename%.exe
