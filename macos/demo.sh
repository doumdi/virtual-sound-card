#!/bin/bash
# macOS Virtual Sound Card Demo
# Demonstrates the virtual sine wave device functionality

set -e

echo "========================================"
echo "Virtual Sound Card Demo for macOS"
echo "========================================"
echo ""

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "Error: This script is for macOS only"
    exit 1
fi

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Building project..."
    echo ""
    mkdir -p build
    cd build
    cmake -DBUILD_MACOS=ON ..
    cmake --build .
    cd ..
fi

# Check if virtual_sine_device exists
if [ ! -f "build/macos/virtual_sine_device" ]; then
    echo "Error: virtual_sine_device not built"
    echo "Please run: cd build && cmake -DBUILD_MACOS=ON .. && cmake --build ."
    exit 1
fi

echo "Step 1: Checking for virtual audio device..."
echo ""

# Check if BlackHole is installed
if system_profiler SPAudioDataType | grep -q "BlackHole"; then
    echo "✓ BlackHole virtual audio device found"
    DEVICE_NAME="BlackHole 2ch"
else
    echo "⚠ BlackHole not found"
    echo ""
    echo "BlackHole is required for this demo."
    echo "Install with: brew install blackhole-2ch"
    echo ""
    echo "Or download from: https://github.com/ExistentialAudio/BlackHole/releases"
    echo ""
    echo "Alternatively, you can:"
    echo "1. Open Audio MIDI Setup (/Applications/Utilities/)"
    echo "2. Create a Multi-Output Device"
    echo "3. Use it as the device name with -d option"
    echo ""
    exit 1
fi

echo ""
echo "Step 2: Available audio devices"
echo ""
./build/macos/virtual_sine_device -l

echo ""
echo "Step 3: Starting virtual sine wave device"
echo "==========================================="
echo "Device: $DEVICE_NAME"
echo "Frequency: 440 Hz"
echo "Duration: 10 seconds"
echo ""
echo "The virtual device will generate a 440Hz sine wave."
echo "Open another application and select '$DEVICE_NAME' as input to hear it."
echo ""
echo "Starting in 3 seconds..."
sleep 3

# Run virtual sine device for 10 seconds in background
echo "Running virtual sine device..."
./build/macos/virtual_sine_device -d "$DEVICE_NAME" -f 440 &
SINE_PID=$!

# Wait for it to start
sleep 2

echo ""
echo "Step 4: Testing loopback (optional)"
echo "===================================="
echo ""
echo "The virtual device is running in the background."
echo "You can test the loopback by recording from $DEVICE_NAME:"
echo "  sox -t coreaudio \"$DEVICE_NAME\" test_output.wav trim 0 5"
echo ""
echo "Or use QuickTime Player:"
echo "  File → New Audio Recording → Select '$DEVICE_NAME' as microphone"
echo ""
echo "Letting it run for 8 more seconds..."
sleep 8

# Stop the virtual sine device
echo ""
echo "Stopping virtual sine device..."
kill $SINE_PID 2>/dev/null || true
wait $SINE_PID 2>/dev/null || true

echo ""
echo "Demo complete!"
echo ""
echo "To run the virtual device manually:"
echo "  ./build/macos/virtual_sine_device -d \"$DEVICE_NAME\" -f 440"
echo ""
echo "For more options:"
echo "  ./build/macos/virtual_sine_device -h"
echo ""
