@echo off
if not exist build mkdir build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
echo [SUCCESS] Build finished successfully
pause