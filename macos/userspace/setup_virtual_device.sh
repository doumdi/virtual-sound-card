#!/bin/bash
# Setup Virtual Audio Device for macOS
#
# This script helps set up a virtual audio device using macOS's built-in
# Multi-Output Device or by installing BlackHole (an open-source virtual audio driver).
#
# Usage: ./setup_virtual_device.sh

echo "========================================"
echo "Virtual Audio Device Setup for macOS"
echo "========================================"
echo ""

# Check if BlackHole is installed
if system_profiler SPAudioDataType | grep -q "BlackHole"; then
    echo "✓ BlackHole virtual audio device is installed"
    echo ""
    echo "You can use BlackHole as your virtual device."
    echo "Run the sine wave generator to output to BlackHole:"
    echo "  ./sine_generator_app 440 3600  # Generates 440Hz for 1 hour"
    echo ""
    echo "Then configure your applications to read from BlackHole as input device."
    exit 0
fi

echo "BlackHole virtual audio device is not installed."
echo ""
echo "BlackHole is an open-source virtual audio driver for macOS that allows"
echo "applications to pass audio to other applications with zero additional latency."
echo ""
echo "Installation options:"
echo ""
echo "1. Install via Homebrew (recommended):"
echo "   brew install blackhole-2ch"
echo ""
echo "2. Download from GitHub:"
echo "   https://github.com/ExistentialAudio/BlackHole/releases"
echo ""
echo "3. Use macOS built-in Multi-Output Device:"
echo "   a. Open 'Audio MIDI Setup' (/Applications/Utilities/)"
echo "   b. Click '+' button and select 'Create Multi-Output Device'"
echo "   c. Check 'Built-in Output' or your preferred output"
echo "   d. Check 'Drift Correction' for Built-in Output"
echo "   e. Name it 'Virtual Sound Card'"
echo ""
echo "After setup, run:"
echo "  ./sine_generator_app"
echo "to generate a sine wave to the virtual device."
echo ""
