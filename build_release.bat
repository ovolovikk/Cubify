@echo off
if not exist build mkdir build
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
echo [SUCCESS] Build finished successfully
pause