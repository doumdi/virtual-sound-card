#!/bin/bash
# Demo script for Linux Virtual Sound Card with JACK2
# This script demonstrates the JACK2 sine wave generator

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Linux Virtual Sound Card - JACK2 Demo${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if JACK program exists - try multiple possible locations
JACK_BIN=""
if [ -f "../build/linux/jack_sine_generator" ]; then
    JACK_BIN="../build/linux/jack_sine_generator"
elif [ -f "build/jack_sine_generator" ]; then
    JACK_BIN="build/jack_sine_generator"
elif [ -f "../build/jack_sine_generator" ]; then
    JACK_BIN="../build/jack_sine_generator"
elif [ -f "jack_sine_generator" ]; then
    JACK_BIN="jack_sine_generator"
fi

if [ -z "$JACK_BIN" ]; then
    echo -e "${RED}Error: jack_sine_generator not built.${NC}"
    echo "Build the project with JACK2 support:"
    echo "  sudo apt-get install libjack-jackd2-dev pkg-config"
    echo "  mkdir build && cd build"
    echo "  cmake .."
    echo "  cmake --build ."
    exit 1
fi

echo -e "${GREEN}Found JACK2 sine generator: ${JACK_BIN}${NC}"
echo ""

# Check if JACK server is running
if ! jack_lsp >/dev/null 2>&1; then
    echo -e "${YELLOW}JACK server is not running${NC}"
    echo ""
    echo "You need to start JACK server first. Options:"
    echo ""
    echo "1. Start JACK with default settings:"
    echo "   ${BLUE}jackd -d alsa &${NC}"
    echo ""
    echo "2. Start JACK with PipeWire (if using PipeWire):"
    echo "   ${BLUE}# JACK is already running via PipeWire${NC}"
    echo ""
    echo "3. Use QjackCtl GUI (recommended):"
    echo "   ${BLUE}qjackctl${NC}"
    echo ""
    
    # Try to detect if PipeWire JACK is available
    if command -v pw-jack >/dev/null 2>&1; then
        echo -e "${YELLOW}PipeWire detected. You can use:${NC}"
        echo "   ${BLUE}pw-jack ${JACK_BIN} 440 5${NC}"
        echo ""
    fi
    
    exit 1
fi

echo -e "${GREEN}JACK server is running${NC}"

# Get JACK server info
SAMPLE_RATE=$(jack_sample_rate 2>/dev/null || echo "unknown")
BUFFER_SIZE=$(jack_buffer_size 2>/dev/null || echo "unknown")

echo "  Sample Rate: ${SAMPLE_RATE} Hz"
echo "  Buffer Size: ${BUFFER_SIZE} frames"
echo ""

# Show available JACK ports
echo "Available JACK ports:"
jack_lsp | head -10
if [ $(jack_lsp | wc -l) -gt 10 ]; then
    echo "  ... (and more)"
fi
echo ""

# Demo parameters
FREQUENCY=440
DURATION=5

echo -e "${BLUE}Demo Configuration:${NC}"
echo "  Frequency: ${FREQUENCY} Hz (A4 note)"
echo "  Duration: ${DURATION} seconds"
echo ""

echo -e "${YELLOW}Starting JACK sine wave generator...${NC}"
echo -e "${YELLOW}Press Ctrl+C to stop early${NC}"
echo ""

# Run the JACK sine generator
if ${JACK_BIN} ${FREQUENCY} ${DURATION}; then
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${GREEN}✓ Demo completed successfully!${NC}"
    echo ""
    echo "The JACK sine wave generator worked correctly."
    echo "You should have heard a ${FREQUENCY}Hz sine wave for ${DURATION} seconds."
    echo ""
    echo "JACK provides:"
    echo "  • Low-latency audio routing"
    echo "  • Professional audio quality"
    echo "  • Cross-platform compatibility"
    echo "  • Flexible connection management"
    echo ""
    echo "Use QjackCtl or jack_connect to route audio between applications."
    echo -e "${BLUE}========================================${NC}"
    exit 0
else
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${RED}✗ Demo failed${NC}"
    echo ""
    echo "The JACK sine generator encountered an error."
    echo "Check the output above for details."
    echo -e "${BLUE}========================================${NC}"
    exit 1
fi
