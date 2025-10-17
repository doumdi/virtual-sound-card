#!/bin/bash
# JACK2 Demo Script
# Cross-platform demo for JACK Audio Connection Kit
# Works on Linux, macOS, and Windows (with bash)

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}JACK2 Virtual Sound Card Demo${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Detect platform
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="Linux"
    JACK_CMD="jackd -d alsa"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
    JACK_CMD="jackd -d coreaudio"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    PLATFORM="Windows"
    JACK_CMD="jackd -d portaudio"
else
    PLATFORM="Unknown"
    JACK_CMD="jackd"
fi

echo "Platform: ${PLATFORM}"
echo ""

# Check if JACK is installed
if ! command -v jackd &> /dev/null; then
    echo -e "${RED}Error: JACK is not installed${NC}"
    echo ""
    echo "Please install JACK2:"
    echo "  Linux:   sudo apt-get install jackd2"
    echo "  macOS:   brew install jack"
    echo "  Windows: Download from https://jackaudio.org/"
    echo ""
    exit 1
fi

echo -e "${GREEN}✓ JACK is installed${NC}"
JACK_VERSION=$(jackd --version 2>&1 | head -n1 || echo "Unknown version")
echo "  Version: ${JACK_VERSION}"
echo ""

# Check if jack_sine_generator exists
if [ ! -f "build/jack_sine_generator" ]; then
    echo -e "${RED}Error: jack_sine_generator not built${NC}"
    echo "Please build with:"
    echo "  mkdir -p build && cd build"
    echo "  cmake -DBUILD_JACK=ON .."
    echo "  cmake --build ."
    exit 1
fi

echo -e "${GREEN}✓ jack_sine_generator found${NC}"
echo ""

# Check if JACK server is running
if jack_wait -c 2>/dev/null; then
    echo -e "${GREEN}✓ JACK server is running${NC}"
    JACK_RUNNING=1
else
    echo -e "${YELLOW}JACK server is not running${NC}"
    echo ""
    echo "To start JACK server manually:"
    echo "  ${JACK_CMD}"
    echo ""
    echo "Or use QjackCtl GUI application"
    echo ""
    echo "For this demo, we'll run without auto-connecting to outputs."
    JACK_RUNNING=0
fi

# Get JACK info if running
if [ $JACK_RUNNING -eq 1 ]; then
    echo ""
    echo "JACK Server Info:"
    SAMPLE_RATE=$(jack_samplerate 2>/dev/null || echo "unknown")
    BUFFER_SIZE=$(jack_bufsize 2>/dev/null || echo "unknown")
    echo "  Sample Rate: ${SAMPLE_RATE} Hz"
    echo "  Buffer Size: ${BUFFER_SIZE} frames"
fi

echo ""
echo -e "${BLUE}========================================${NC}"
echo "Demo Configuration:"
echo "  Frequency: 440 Hz (A4 note)"
echo "  Duration: 10 seconds"
echo ""

echo -e "${YELLOW}Starting JACK sine wave generator...${NC}"
echo ""

# Run the generator
if [ $JACK_RUNNING -eq 1 ]; then
    echo "The sine generator will run for 10 seconds."
    echo "Use a JACK patchbay (qjackctl, Catia, etc.) to:"
    echo "  - Connect sine_generator:output_left -> system:playback_1"
    echo "  - Connect sine_generator:output_right -> system:playback_2"
    echo ""
    echo "Or wait for auto-connection..."
    echo ""
fi

./build/jack_sine_generator 440 10

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}✓ Demo completed successfully!${NC}"
echo ""
echo "JACK2 is working correctly with virtual-sound-card."
echo ""
echo "You can use JACK for:"
echo "  - Low-latency audio routing between applications"
echo "  - Professional audio production workflows"
echo "  - Cross-platform audio development"
echo ""
echo "Learn more:"
echo "  - JACK Audio: https://jackaudio.org/"
echo "  - QjackCtl: https://qjackctl.sourceforge.io/"
echo -e "${BLUE}========================================${NC}"
