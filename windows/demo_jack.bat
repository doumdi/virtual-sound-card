@echo off
REM Demo script for Windows Virtual Sound Card with JACK2
REM This script demonstrates the JACK2 sine wave generator

setlocal enabledelayedexpansion

echo ========================================
echo Windows Virtual Sound Card - JACK2 Demo
echo ========================================
echo.

REM Check if JACK program exists - try multiple possible locations
set JACK_BIN=
if exist "..\build\windows\jack_sine_generator.exe" (
    set JACK_BIN=..\build\windows\jack_sine_generator.exe
) else if exist "build\Release\jack_sine_generator.exe" (
    set JACK_BIN=build\Release\jack_sine_generator.exe
) else if exist "..\build\jack_sine_generator.exe" (
    set JACK_BIN=..\build\jack_sine_generator.exe
) else if exist "jack_sine_generator.exe" (
    set JACK_BIN=jack_sine_generator.exe
)

if "%JACK_BIN%"=="" (
    echo [ERROR] jack_sine_generator.exe not built.
    echo.
    echo Build the project with JACK2 support:
    echo   1. Install JACK2 from https://jackaudio.org/
    echo   2. mkdir build ^&^& cd build
    echo   3. cmake -DBUILD_WINDOWS=ON ..
    echo   4. cmake --build . --config Release
    exit /b 1
)

echo [OK] Found JACK2 sine generator: !JACK_BIN!
echo.

REM Check if JACK server is running
jack_lsp >nul 2>&1
if errorlevel 1 (
    echo [WARNING] JACK server is not running
    echo.
    echo You need to start JACK server first. Options:
    echo.
    echo 1. Use QjackCtl GUI ^(recommended^):
    echo    - Install from https://jackaudio.org/
    echo    - Start QjackCtl from Start Menu
    echo    - Click "Start" button
    echo.
    echo 2. Start JACK from command line:
    echo    jackd -d portaudio
    echo.
    exit /b 1
)

echo [OK] JACK server is running
echo.

REM Show available JACK ports (first 9 lines)
echo Available JACK ports:
set COUNT=0
for /f "delims=" %%i in ('jack_lsp 2^>nul') do (
    echo %%i
    set /a COUNT+=1
    if !COUNT! GEQ 9 goto :done_ports
)
:done_ports
echo.

REM Demo parameters
set FREQUENCY=440
set DURATION=5

echo Demo Configuration:
echo   Frequency: %FREQUENCY% Hz (A4 note)
echo   Duration: %DURATION% seconds
echo.

echo Starting JACK sine wave generator...
echo Press Ctrl+C to stop early
echo.

REM Run the JACK sine generator
!JACK_BIN! %FREQUENCY% %DURATION%
if errorlevel 1 (
    echo.
    echo ========================================
    echo [FAILED] Demo failed
    echo.
    echo The JACK sine generator encountered an error.
    echo Check the output above for details.
    echo ========================================
    exit /b 1
)

echo.
echo ========================================
echo [SUCCESS] Demo completed successfully!
echo.
echo The JACK sine wave generator worked correctly.
echo You should have heard a %FREQUENCY%Hz sine wave for %DURATION% seconds.
echo.
echo JACK provides:
echo   * Low-latency audio routing
echo   * Professional audio quality
echo   * Cross-platform compatibility
echo   * Flexible connection management
echo.
echo Use QjackCtl to route audio between applications.
echo ========================================

exit /b 0
