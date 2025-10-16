@echo off
REM Demo script for Windows Virtual Sound Card
REM This script demonstrates the virtual sine wave device with VB-Cable loopback

setlocal EnableDelayedExpansion

echo ========================================
echo Windows Virtual Sound Card Demo
echo ========================================
echo.

REM Check if programs exist
if not exist "build\virtual_sine_device.exe" (
    echo Error: Programs not built. Run 'nmake' or 'cmake --build .' first.
    exit /b 1
)

REM Check if audio service is running
sc query Audiosrv | find "RUNNING" >nul
if errorlevel 1 (
    echo Warning: Windows Audio service is not running
    echo Attempting to start service...
    sc start Audiosrv >nul 2>&1
    if errorlevel 1 (
        echo Error: Could not start Windows Audio service
        echo Please start it manually:
        echo   sc start Audiosrv
        exit /b 1
    )
    echo Audio service started successfully
) else (
    echo Audio service is running
)
echo.

REM Demo parameters
set FREQUENCY=440
set DURATION=10

echo Demo Configuration:
echo   Frequency: %FREQUENCY% Hz (A4 note)
echo   Program: virtual_sine_device.exe
echo.

echo This demo showcases the Windows equivalent of:
echo   - macOS: virtual_sine_device with BlackHole
echo   - Linux: sine_generator_app with ALSA loopback
echo.

echo ========================================
echo Checking for VB-Cable Installation
echo ========================================
echo.

REM List available devices
echo Available audio output devices:
build\virtual_sine_device.exe -l

echo.
echo ========================================
echo Demo Options
echo ========================================
echo.
echo Choose an option:
echo   1. Play to default device (you can hear it)
echo   2. Play to VB-Cable "CABLE Input" (for loopback testing)
echo   3. Exit
echo.

set /p choice="Enter choice (1-3): "

if "%choice%"=="1" (
    echo.
    echo Playing sine wave to default device...
    echo Press Ctrl+C to stop
    echo.
    build\virtual_sine_device.exe -f %FREQUENCY%
) else if "%choice%"=="2" (
    echo.
    echo Playing sine wave to VB-Cable...
    echo.
    echo If VB-Cable is installed, open another terminal and run:
    echo   build\test_loopback_read.exe
    echo.
    echo Press Ctrl+C to stop
    echo.
    build\virtual_sine_device.exe -d "CABLE Input" -f %FREQUENCY%
) else (
    echo Exiting demo
    exit /b 0
)

echo.
echo ========================================
echo Demo Complete
echo ========================================
echo.
echo For full loopback testing with VB-Cable:
echo   1. Install VB-Cable from https://vb-audio.com/Cable/
echo   2. Terminal 1: build\virtual_sine_device.exe -d "CABLE Input" -f 440
echo   3. Terminal 2: build\test_loopback_read.exe
echo   4. The test will verify the audio signal
echo.
echo See QUICKSTART.md for detailed instructions
echo ========================================

endlocal
