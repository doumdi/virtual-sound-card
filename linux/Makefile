# Makefile for Linux Virtual Sound Card Implementation

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm -lasound

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
	@echo "Setting up ALSA loopback device..."
	@if ! lsmod | grep -q snd_aloop; then \
		echo "Loading snd-aloop kernel module..."; \
		sudo modprobe snd-aloop; \
		echo "Module loaded successfully."; \
	else \
		echo "snd-aloop module already loaded."; \
	fi
	@echo ""
	@echo "Available loopback devices:"
	@aplay -l | grep Loopback || echo "No loopback devices found!"

test: $(SINE_GEN) $(LOOPBACK_TEST)
	@echo "==========================================="
	@echo "Virtual Sound Card Test Suite"
	@echo "==========================================="
	@echo ""
	@echo "This test requires the snd-aloop module to be loaded."
	@echo "Run 'make setup' if you haven't already."
	@echo ""
	@echo "The test will:"
	@echo "  1. Start the sine wave generator in the background"
	@echo "  2. Run the loopback read test"
	@echo "  3. Verify the audio signal"
	@echo ""
	@echo "Starting test..."
	@echo ""
	@# Check if snd-aloop is loaded
	@if ! lsmod | grep -q snd_aloop; then \
		echo "ERROR: snd-aloop module not loaded. Run 'make setup' first."; \
		exit 1; \
	fi
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
	@echo "Installing Linux virtual sound card utilities..."
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(SINE_GEN) $(DESTDIR)/usr/local/bin/
	install -m 755 $(LOOPBACK_TEST) $(DESTDIR)/usr/local/bin/
	@echo "Installation complete."
	@echo ""
	@echo "To use the virtual sound card:"
	@echo "  1. Load the kernel module: sudo modprobe snd-aloop"
	@echo "  2. Run sine generator: sine_generator_app [frequency] [duration]"
	@echo "  3. Test with: test_loopback_read"

clean:
	rm -rf $(BUILD_DIR)
	rm -f /tmp/sine_gen.log

help:
	@echo "Linux Virtual Sound Card - Makefile Help"
	@echo ""
	@echo "Targets:"
	@echo "  all             - Build all programs (default)"
	@echo "  setup           - Load snd-aloop kernel module"
	@echo "  test            - Run automated test suite"
	@echo "  install         - Install programs to /usr/local/bin"
	@echo "  clean           - Remove build artifacts"
	@echo "  help            - Show this help message"
	@echo ""
	@echo "Quick Start:"
	@echo "  make            - Build everything"
	@echo "  make setup      - Load ALSA loopback module"
	@echo "  ./demo.sh       - Run interactive demo"
	@echo "  make test       - Run automated tests"
	@echo ""
	@echo "Manual Usage:"
	@echo "  # Terminal 1: Generate sine wave"
	@echo "  ./build/sine_generator_app 440 10"
	@echo ""
	@echo "  # Terminal 2: Read and verify"
	@echo "  ./build/test_loopback_read"
	@echo ""
	@echo "For detailed documentation, see README.md or QUICKSTART.md"
