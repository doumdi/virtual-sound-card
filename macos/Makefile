# Makefile for macOS Virtual Sound Card Implementation

CC = clang
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -framework CoreAudio -framework AudioToolbox

# Directories
USERSPACE_DIR = userspace
TESTS_DIR = tests
BUILD_DIR = build

# Targets
SINE_GEN = $(BUILD_DIR)/sine_generator_app
LOOPBACK_TEST = $(BUILD_DIR)/test_loopback_read

# Source files
SINE_GEN_SRC = $(USERSPACE_DIR)/sine_generator_app.c
LOOPBACK_TEST_SRC = $(TESTS_DIR)/test_loopback_read.c

.PHONY: all clean test install help setup

all: $(SINE_GEN) $(LOOPBACK_TEST)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SINE_GEN): $(SINE_GEN_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(LOOPBACK_TEST): $(LOOPBACK_TEST_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

setup:
	@echo "Setting up macOS virtual sound card..."
	@echo ""
	@echo "macOS uses CoreAudio framework for audio routing."
	@echo "To test the loopback functionality, you need to:"
	@echo ""
	@echo "Option 1: Use BlackHole virtual audio driver"
	@echo "  1. Install BlackHole: brew install blackhole-2ch"
	@echo "  2. Open Audio MIDI Setup.app"
	@echo "  3. Create an Aggregate Device combining BlackHole and your output"
	@echo "  4. Set the Aggregate Device as default input/output"
	@echo ""
	@echo "Option 2: Use built-in audio routing"
	@echo "  1. Open Audio MIDI Setup.app"
	@echo "  2. Create a Multi-Output Device"
	@echo "  3. Configure audio routing as needed"
	@echo ""
	@echo "After setup, you can run 'make test' to verify functionality."

test: $(SINE_GEN) $(LOOPBACK_TEST)
	@echo "==========================================="
	@echo "Virtual Sound Card Test Suite (macOS)"
	@echo "==========================================="
	@echo ""
	@echo "This test verifies audio loopback functionality."
	@echo ""
	@echo "PREREQUISITES:"
	@echo "  - A virtual audio device (e.g., BlackHole) must be installed"
	@echo "  - The virtual device should be set as both input and output"
	@echo "  - Run 'make setup' for setup instructions if needed"
	@echo ""
	@echo "The test will:"
	@echo "  1. Start the sine wave generator in the background"
	@echo "  2. Run the loopback read test"
	@echo "  3. Verify the audio signal"
	@echo ""
	@echo "Starting test..."
	@echo ""
	@# Start sine generator in background and test
	@$(SINE_GEN) 440 3 > /tmp/sine_gen.log 2>&1 & \
	SINE_PID=$$!; \
	sleep 1; \
	if $(LOOPBACK_TEST); then \
		echo ""; \
		echo "Test PASSED!"; \
		kill $$SINE_PID 2>/dev/null || true; \
		exit 0; \
	else \
		echo ""; \
		echo "Test FAILED!"; \
		kill $$SINE_PID 2>/dev/null || true; \
		exit 1; \
	fi

install: all
	@echo "Installing macOS virtual sound card utilities..."
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(SINE_GEN) $(DESTDIR)/usr/local/bin/
	install -m 755 $(LOOPBACK_TEST) $(DESTDIR)/usr/local/bin/
	@echo "Installation complete."
	@echo ""
	@echo "To use the virtual sound card:"
	@echo "  1. Set up a virtual audio device (see 'make setup')"
	@echo "  2. Run sine generator: sine_generator_app [frequency] [duration]"
	@echo "  3. Test with: test_loopback_read"

clean:
	rm -rf $(BUILD_DIR)
	rm -f /tmp/sine_gen.log

help:
	@echo "macOS Virtual Sound Card - Makefile Help"
	@echo ""
	@echo "Targets:"
	@echo "  all             - Build all programs (default)"
	@echo "  setup           - Show setup instructions for virtual audio"
	@echo "  test            - Run automated test suite"
	@echo "  install         - Install programs to /usr/local/bin"
	@echo "  clean           - Remove build artifacts"
	@echo "  help            - Show this help message"
	@echo ""
	@echo "Quick Start:"
	@echo "  make            - Build everything"
	@echo "  make setup      - Show setup instructions"
	@echo "  make test       - Run automated tests"
	@echo ""
	@echo "Manual Usage:"
	@echo "  # Terminal 1: Generate sine wave"
	@echo "  ./build/sine_generator_app 440 10"
	@echo ""
	@echo "  # Terminal 2: Read and verify"
	@echo "  ./build/test_loopback_read"
	@echo ""
	@echo "For detailed documentation, see README.md"
