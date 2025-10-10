@echo off
REM Demo script for Windows Virtual Sound Card
REM This script demonstrates the sine wave generator and verification test

setlocal EnableDelayedExpansion

echo ========================================
echo Windows Virtual Sound Card Demo
echo ========================================
echo.

REM Check if programs exist
if not exist "build\sine_generator_app.exe" (
    echo Error: Programs not built. Run 'nmake' first.
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
set DURATION=5

echo Demo Configuration:
echo   Frequency: %FREQUENCY% Hz (A4 note)
echo   Duration: %DURATION% seconds
echo.

echo Note: For loopback testing, you need a virtual audio cable installed.
echo       Recommended: VB-Cable (https://vb-audio.com/Cable/)
echo.
echo If you don't have a virtual audio cable, this demo will play audio
echo through your default output device. You can hear it but loopback
echo verification will not work.
echo.

pause

echo Starting sine wave generator...
echo.

REM Run sine generator in background
start /B "Sine Generator" build\sine_generator_app.exe %FREQUENCY% %DURATION% > nul 2>&1

REM Give it a moment to start
timeout /t 2 /nobreak >nul

echo Sine generator started
echo Playing %FREQUENCY% Hz sine wave for %DURATION% seconds...
echo You should hear the tone through your speakers/headphones
echo.

REM Wait for playback to complete
timeout /t %DURATION% /nobreak

echo.
echo Playback complete!
echo.
echo ========================================
echo Demo Information
echo ========================================
echo.
echo What just happened:
echo   1. A sine wave generator started
echo   2. It played a %FREQUENCY% Hz tone for %DURATION% seconds
echo   3. Audio was sent to your default audio output device
echo.
echo For full loopback testing:
echo   1. Install a virtual audio cable (e.g., VB-Cable)
echo   2. Set it as your default playback device
echo   3. Run: build\sine_generator_app.exe (in one terminal)
echo   4. Run: build\test_loopback_read.exe (in another terminal)
echo   5. The test will verify the audio signal
echo.
echo See QUICKSTART.md for detailed instructions
echo ========================================

endlocal
