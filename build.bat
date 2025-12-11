@echo off
REM Simple build script for ULUI project on Windows

setlocal enabledelayedexpansion

echo ================================
echo ULUI Build Script for Windows
echo ================================
echo.

REM Configuration
set BUILD_TYPE=Release
if not "%1"=="" set BUILD_TYPE=%1

set BUILD_DIR=build

echo Build Type: %BUILD_TYPE%
echo.

REM Create build directory
if exist "%BUILD_DIR%" (
    echo Build directory exists, cleaning...
    rmdir /s /q "%BUILD_DIR%"
)

mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

REM Configure
echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    echo.
    echo Trying with default generator...
    cmake ..
    if errorlevel 1 (
        echo CMake configuration failed completely!
        goto :error
    )
)

REM Build
echo.
echo Building project...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo Build failed!
    goto :error
)

REM Success message
echo.
echo ================================
echo Build completed successfully!
echo ================================
echo Executable location: %BUILD_DIR%\bin\%BUILD_TYPE%\
echo.
echo To run the application:
echo   cd %BUILD_DIR%\bin\%BUILD_TYPE%
echo   ului_app.exe
echo.
goto :end

:error
echo.
echo ================================
echo Build failed with errors!
echo ================================
exit /b 1

:end
cd ..
endlocal
