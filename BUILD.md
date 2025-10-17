# Build Guide

Quick reference for building the Virtual Sound Card project.

## Requirements

- CMake 3.15 or later
- C compiler (GCC, Clang, or MSVC)
- C++ compiler

## Quick Build

```bash
# Clone and build
git clone https://github.com/doumdi/virtual-sound-card.git
cd virtual-sound-card
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
ctest --output-on-failure
```

## Build Options

```bash
# Enable/disable tests
cmake -DBUILD_TESTS=ON ..

# Build for specific platform
cmake -DBUILD_LINUX=ON ..
cmake -DBUILD_WINDOWS=ON ..
cmake -DBUILD_MACOS=ON ..

# Build with JACK2 support (cross-platform)
cmake -DBUILD_JACK=ON ..
```

## Build Output

The build produces:
- `common/libvcard_common.a` - Static library with common functionality
- `tests/test_sine_generator` - Sine wave generator tests
- `tests/test_api_init` - API initialization tests
- `tests/test_sine_wave_file` - WAV file generation test

### JACK2 Output

When building with JACK2 support (`-DBUILD_JACK=ON`):
- `jack_sine_generator` - Cross-platform JACK sine wave generator
- `tests/test_jack_connection` - JACK API connection test

### Linux-Specific Output

When building on Linux with ALSA support:
- `linux/sine_generator_app` - Sine wave generator application
- `linux/test_loopback_read` - Loopback device verification test

### Windows-Specific Output

When building on Windows with WASAPI support:
- `windows/sine_generator_app.exe` - Sine wave generator application
- `windows/test_loopback_read.exe` - Loopback device verification test

## Installation

```bash
# Install to system default location (/usr/local)
sudo cmake --install .

# Install to custom prefix
cmake --install . --prefix /custom/path
```

## Testing

```bash
# Run all tests
ctest

# Run with detailed output
ctest --output-on-failure --verbose

# Run specific test
./tests/test_sine_generator

# Test JACK connection (if built with JACK support)
./tests/test_jack_connection

# Play generated test audio (Linux/macOS)
aplay tests/test_sine_440hz.wav  # Linux
afplay tests/test_sine_440hz.wav  # macOS
```

### JACK2 Testing

```bash
# Ensure JACK server is running
jackd -d alsa      # Linux
jackd -d coreaudio # macOS  
jackd -d portaudio # Windows

# Run JACK demo
../jack_demo.sh      # Linux/macOS
..\jack_demo.bat     # Windows

# Or run generator manually
./jack_sine_generator 440 10
```

### Windows Testing

```cmd
REM Run tests
ctest --output-on-failure

REM Run Windows demo
cd windows
demo.bat

REM Run programs manually
build\sine_generator_app.exe 440 5
build\test_loopback_read.exe
```

## Cleaning

```bash
# Remove build directory
cd .. && rm -rf build

# Or use CMake
cd build && cmake --build . --target clean
```

## For More Information

See [INSTALL.md](INSTALL.md) for detailed installation instructions and troubleshooting.
