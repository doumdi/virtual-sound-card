# Installation Guide

This document provides detailed instructions for building and installing the Virtual Sound Card on all supported platforms.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Building from Source](#building-from-source)
- [Platform-Specific Instructions](#platform-specific-instructions)
  - [Linux](#linux)
  - [Windows](#windows)
  - [macOS](#macos)
- [Testing](#testing)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### Common Requirements

All platforms require:
- **CMake** 3.15 or later
- A C compiler (GCC, Clang, or MSVC)
- A C++ compiler (for platform-specific implementations)

### Linux

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake git

# Fedora/RHEL
sudo dnf install cmake gcc gcc-c++ git

# Arch Linux
sudo pacman -S base-devel cmake git
```

### Windows

- **Visual Studio 2019 or later** with C++ development tools
- **CMake** 3.15 or later (included with Visual Studio or download from cmake.org)
- **Git for Windows**

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake via Homebrew (recommended)
brew install cmake
```

## Building from Source

### Quick Start (All Platforms)

```bash
# Clone the repository
git clone https://github.com/doumdi/virtual-sound-card.git
cd virtual-sound-card

# Create build directory
mkdir build
cd build

# Configure (CMake will auto-detect your platform)
cmake ..

# Build
cmake --build .

# Run tests
ctest --output-on-failure
```

### Configuration Options

CMake options can be specified during configuration:

```bash
cmake -DBUILD_TESTS=ON -DBUILD_LINUX=ON ..
```

Available options:
- `BUILD_TESTS` (default: ON) - Build test suite
- `BUILD_LINUX` (default: auto-detect) - Build Linux implementation
- `BUILD_WINDOWS` (default: auto-detect) - Build Windows implementation
- `BUILD_MACOS` (default: auto-detect) - Build macOS implementation

## Platform-Specific Instructions

### Linux

#### Building

```bash
cd virtual-sound-card
mkdir build && cd build
cmake -DBUILD_LINUX=ON ..
cmake --build .
```

#### Installation

```bash
# Install libraries and headers
sudo cmake --install .

# Install to custom prefix
cmake --install . --prefix /usr/local
```

#### Verification

```bash
# List installed files
ls /usr/local/lib/libvcard*
ls /usr/local/include/vcard.h
```

### Windows

#### Building with Visual Studio

1. Open Visual Studio 2019 or later
2. Select **File → Open → CMake...**
3. Navigate to the cloned repository and open `CMakeLists.txt`
4. Visual Studio will automatically configure the project
5. Select **Build → Build All** (Ctrl+Shift+B)

#### Building with Command Line

```cmd
cd virtual-sound-card
mkdir build
cd build

# Configure for Visual Studio 2019 (x64)
cmake -G "Visual Studio 16 2019" -A x64 ..

# Build
cmake --build . --config Release

# Run tests
ctest -C Release --output-on-failure
```

#### Installation

```cmd
# Install (requires administrator privileges)
cmake --install . --config Release

# Install to custom location
cmake --install . --config Release --prefix "C:\Program Files\VirtualSoundCard"
```

### macOS

#### Building

```bash
cd virtual-sound-card
mkdir build && cd build

# Configure
cmake -DBUILD_MACOS=ON ..

# Build
cmake --build .
```

#### Installation

```bash
# Install system-wide (requires sudo)
sudo cmake --install .

# Install to user location
cmake --install . --prefix ~/local
```

## Testing

### Running All Tests

```bash
cd build
ctest --output-on-failure
```

### Running Individual Tests

```bash
# Test sine wave generator
./tests/test_sine_generator

# Test API initialization
./tests/test_api_init

# Generate test audio file
./tests/test_sine_wave_file
```

### Test Output

The `test_sine_wave_file` test generates a WAV file (`test_sine_440hz.wav`) that you can play to verify audio generation:

**Linux:**
```bash
aplay test_sine_440hz.wav
```

**macOS:**
```bash
afplay test_sine_440hz.wav
```

**Windows:**
```cmd
start test_sine_440hz.wav
```

## Verification

After installation, verify the library is correctly installed:

### Linux/macOS

```bash
# Check library
ls -l /usr/local/lib/libvcard_common.a

# Check header
ls -l /usr/local/include/vcard.h

# Test linking
cat > test.c << 'EOF'
#include <vcard.h>
#include <stdio.h>
int main() {
    int maj, min, patch;
    vcard_get_version(&maj, &min, &patch);
    printf("Version: %d.%d.%d\n", maj, min, patch);
    return 0;
}
EOF

gcc test.c -lvcard_common -lm -o test
./test
```

### Windows

Check that files are installed in the specified prefix directory:
- `lib/vcard_common.lib`
- `include/vcard.h`

## Troubleshooting

### CMake Configuration Fails

**Problem:** CMake cannot find required tools or libraries

**Solution:**
- Ensure CMake version is 3.15 or later: `cmake --version`
- Verify compiler is in PATH
- On Windows, ensure Visual Studio C++ tools are installed

### Build Fails on Linux

**Problem:** Missing math library

**Solution:**
```bash
# The math library (-lm) should be automatically linked
# If issues persist, ensure build-essential is installed
sudo apt-get install build-essential
```

### Tests Fail

**Problem:** Tests fail during `ctest`

**Solution:**
```bash
# Run tests with verbose output
ctest --output-on-failure --verbose

# Run individual test to see detailed output
./tests/test_sine_generator
```

### Permission Denied on Installation

**Problem:** Cannot write to installation directory

**Solution:**
```bash
# Linux/macOS: Use sudo for system-wide install
sudo cmake --install .

# Or install to user directory
cmake --install . --prefix ~/.local
```

### Generated WAV File Won't Play

**Problem:** test_sine_440hz.wav doesn't play

**Solution:**
- Verify file was created: `ls -l test_sine_440hz.wav`
- Check file size (should be ~192KB for 2-second 48kHz audio)
- Try different media player
- Verify audio output device is working

## Next Steps

After successful installation:

1. **Explore the API**: See [docs/API.md](docs/API.md) for API documentation
2. **Platform Implementation**: See platform-specific README files:
   - [linux/README.md](linux/README.md)
   - [windows/README.md](windows/README.md)
   - [macos/README.md](macos/README.md)
3. **Contributing**: See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines

## Support

For issues and questions:
- **Issue Tracker**: https://github.com/doumdi/virtual-sound-card/issues
- **Discussions**: https://github.com/doumdi/virtual-sound-card/discussions

## Additional Resources

- [CMake Documentation](https://cmake.org/documentation/)
- [Project Architecture](docs/ARCHITECTURE.md)
- [API Reference](docs/API.md)
