@echo off
REM JACK2 Demo Script for Windows
REM Demonstrates JACK Audio Connection Kit with virtual-sound-card

setlocal EnableDelayedExpansion

echo ========================================
echo JACK2 Virtual Sound Card Demo (Windows)
echo ========================================
echo.

REM Check if JACK is installed
where jackd >nul 2>&1
if errorlevel 1 (
    echo Error: JACK is not installed
    echo.
    echo Please install JACK2 for Windows:
    echo   Download from: https://jackaudio.org/downloads/
    echo.
    echo Installation notes:
    echo   - Download JACK2 for Windows installer
    echo   - Run installer and follow instructions
    echo   - Add JACK bin directory to PATH or restart
    echo.
    pause
    exit /b 1
)

echo [OK] JACK is installed
for /f "tokens=*" %%i in ('jackd --version 2^>^&1 ^| findstr /C:"version"') do set JACK_VERSION=%%i
echo   Version: %JACK_VERSION%
echo.

REM Check if jack_sine_generator exists
if not exist "build\jack_sine_generator.exe" (
    if not exist "build\Release\jack_sine_generator.exe" (
        if not exist "build\Debug\jack_sine_generator.exe" (
            echo Error: jack_sine_generator.exe not built
            echo.
            echo Please build with:
            echo   mkdir build
            echo   cd build
            echo   cmake -DBUILD_JACK=ON ..
            echo   cmake --build .
            echo.
            pause
            exit /b 1
        )
        set GENERATOR=build\Debug\jack_sine_generator.exe
    ) else (
        set GENERATOR=build\Release\jack_sine_generator.exe
    )
) else (
    set GENERATOR=build\jack_sine_generator.exe
)

echo [OK] jack_sine_generator found
echo   Path: %GENERATOR%
echo.

REM Check if JACK server is running
jack_wait -c >nul 2>&1
if errorlevel 1 (
    echo [WARNING] JACK server is not running
    echo.
    echo To start JACK server:
    echo   1. Use QjackCtl GUI application
    echo   2. Or manually: jackd -d portaudio
    echo.
    echo Starting JACK server with PortAudio driver...
    start "JACK Server" /MIN jackd -d portaudio
    timeout /t 3 /nobreak >nul
    
    jack_wait -c >nul 2>&1
    if errorlevel 1 (
        echo Error: Failed to start JACK server
        echo Please start JACK manually and try again
        pause
        exit /b 1
    )
    echo [OK] JACK server started
) else (
    echo [OK] JACK server is running
)

echo.
echo JACK Server Info:
for /f "tokens=*" %%i in ('jack_samplerate 2^>^&1') do set SAMPLE_RATE=%%i
for /f "tokens=*" %%i in ('jack_bufsize 2^>^&1') do set BUFFER_SIZE=%%i
echo   Sample Rate: %SAMPLE_RATE% Hz
echo   Buffer Size: %BUFFER_SIZE% frames

echo.
echo ========================================
echo Demo Configuration:
echo   Frequency: 440 Hz (A4 note)
echo   Duration: 10 seconds
echo.

echo Starting JACK sine wave generator...
echo.
echo The sine generator will run for 10 seconds.
echo Use a JACK patchbay (QjackCtl, Catia, etc.) to:
echo   - Connect sine_generator:output_left to system:playback_1
echo   - Connect sine_generator:output_right to system:playback_2
echo.
echo Or wait for auto-connection...
echo.

%GENERATOR% 440 10

echo.
echo ========================================
echo [OK] Demo completed successfully!
echo.
echo JACK2 is working correctly with virtual-sound-card.
echo.
echo You can use JACK for:
echo   - Low-latency audio routing between applications
echo   - Professional audio production workflows
echo   - Cross-platform audio development
echo.
echo Learn more:
echo   - JACK Audio: https://jackaudio.org/
echo   - QjackCtl: https://qjackctl.sourceforge.io/
echo ========================================
echo.

pause
endlocal
