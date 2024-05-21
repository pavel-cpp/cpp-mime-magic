@echo off

REM Check if the installation path argument is passed
if "%~1"=="" (
    set /p INSTALLATION_PATH="Enter the installation path (for example, C:\Program Files\MyApp): "
) else (
    set INSTALLATION_PATH=%~1
)

REM Create a build directory
cmake -G  Ninja -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake -G  Ninja -DCMAKE_BUILD_TYPE=Release -S . -B build

REM Putting the project together
cmake --build build

REM Install the project
cmake --install build --prefix %INSTALLATION_PATH% %SILENT_FLAG%

rmdir /s /q build

pause
