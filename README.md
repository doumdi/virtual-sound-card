# Virtual Sound Card Driver

[![CI](https://github.com/doumdi/virtual-sound-card/actions/workflows/ci.yml/badge.svg)](https://github.com/doumdi/virtual-sound-card/actions/workflows/ci.yml)

A cross-platform virtual sound card driver supporting Linux, Windows, and macOS with configurable input/output channels and MIDI support.

## Project Overview

This project aims to create a virtual sound card driver for multiple operating systems that provides:

- **Configurable Input/Output Channels**: Flexible number of audio input and output channels
- **MIDI Support**: Full MIDI device functionality
- **Cross-Platform**: Support for Linux, Windows, and macOS
- **Low Latency**: Optimized for real-time audio processing

## Project Structure

```
virtual-sound-card/
├── linux/          # Linux ALSA/PulseAudio driver implementation
├── windows/        # Windows WASAPI/WDM driver implementation
├── macos/          # macOS CoreAudio driver implementation
├── common/         # Shared code and utilities
├── docs/           # Documentation and specifications
└── README.md       # This file
```

## Platform-Specific Implementations

### Cross-Platform Virtual Audio Solutions Comparison

Each platform has different approaches to creating virtual audio devices for loopback and routing:

| Platform | Solution | Type | Description |
|----------|----------|------|-------------|
| **Linux** | ALSA loopback (`snd-aloop`) | Kernel module | Built-in kernel module providing virtual audio devices |
| **macOS** | BlackHole | System extension | Third-party virtual audio driver (free, open-source) |
| **Windows** | VB-Cable | Kernel driver | Third-party virtual audio cable (free) |
| **Windows** | Voicemeeter | Application + driver | Virtual mixer with audio cables (free) |
| **Windows** | Virtual Audio Cable | Kernel driver | Professional multi-cable solution (commercial) |
| **All** | JACK Audio | Audio server | Professional audio routing server (free, cross-platform) |

### Linux
The Linux implementation uses ALSA (Advanced Linux Sound Architecture) or PulseAudio for creating virtual audio devices.

**Technology Stack:**
- ALSA kernel modules
- PulseAudio module-loopback
- C/C++

**Requirements:**
- Linux kernel headers
- ALSA development libraries
- GCC/Clang compiler

### Windows
The Windows implementation uses Windows Driver Model (WDM) or WASAPI (Windows Audio Session API).

**Technology Stack:**
- Windows Driver Kit (WDK)
- WASAPI
- C/C++

**Virtual Audio Cable Solutions:**
Similar to Linux's ALSA loopback and macOS's BlackHole, Windows has several third-party virtual audio cable solutions:
- VB-Cable (Free) - Basic virtual audio routing
- Voicemeeter (Free) - Advanced virtual audio mixer
- Virtual Audio Cable (Commercial) - Professional multi-cable solution
- JACK Audio (Free) - Professional audio routing server

**Requirements:**
- Windows 10/11 SDK
- Visual Studio 2019 or later
- Windows Driver Kit (WDK)

**Quick Start:**
```bash
# Build virtual sound card userspace tools
mkdir build && cd build
cmake -DBUILD_WINDOWS=ON ..
cmake --build .

# Install a virtual audio cable (e.g., VB-Cable)
# Download from: https://vb-audio.com/Cable/

# Run sine generator to test
cd windows/Release
sine_generator_app.exe 440 10
```

Audio can be routed between applications using the virtual audio cable devices.

### macOS
The macOS implementation uses CoreAudio framework for creating virtual audio devices.

**Technology Stack:**
- CoreAudio framework
- Audio HAL (Hardware Abstraction Layer)
- C/C++/Objective-C

**Requirements:**
- Xcode 12 or later
- macOS SDK
- BlackHole or similar virtual audio driver (for loopback)

**Quick Start:**
```bash
# Install BlackHole virtual audio driver
brew install blackhole-2ch

# Build and run virtual sine device
mkdir build && cd build
cmake -DBUILD_MACOS=ON ..
cmake --build .
./macos/virtual_sine_device -d "BlackHole 2ch" -f 440
```

Now applications can read sine wave audio from "BlackHole 2ch" as an input device.

## Getting Started

### Prerequisites

- **Linux**: GCC/Clang, ALSA libraries, kernel headers
- **Windows**: Visual Studio, Windows SDK, WDK
- **macOS**: Xcode, macOS SDK

### Building

The project uses **CMake** as the cross-platform build system.

**Quick Start:**

```bash
# Clone the repository
git clone https://github.com/doumdi/virtual-sound-card.git
cd virtual-sound-card

# Build
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
ctest --output-on-failure
```

**Documentation:**
- [BUILD.md](BUILD.md) - Quick build reference
- [INSTALL.md](INSTALL.md) - Detailed installation guide

**Platform-specific implementation details:**
- [Linux Implementation](linux/README.md)
- [Windows Implementation](windows/README.md)
- [macOS Implementation](macos/README.md)

## Features

### Implemented
- [x] **CMake Build System**: Cross-platform build configuration
- [x] **Sine Wave Generator**: Test audio generation for verification
- [x] **Test Suite**: Automated testing with CTest
- [x] **Common API**: Cross-platform API definitions
- [x] **macOS Virtual Device**: Virtual sine wave device using CoreAudio

### Planned
- [ ] Configurable number of input channels (1-32)
- [ ] Configurable number of output channels (1-32)
- [ ] Sample rate support: 44.1kHz, 48kHz, 96kHz, 192kHz
- [ ] Bit depth support: 16-bit, 24-bit, 32-bit
- [ ] MIDI input/output support
- [ ] Low-latency audio routing
- [ ] Cross-platform configuration utility
- [ ] Audio loopback functionality

## Architecture

The virtual sound card driver consists of three main components:

1. **Kernel/System Driver**: Low-level driver that integrates with the OS audio subsystem
2. **User-space Control**: Configuration and management utilities
3. **Common Library**: Shared code for audio processing and configuration

## Development Roadmap

1. **Phase 1**: Basic project structure and documentation
2. **Phase 2**: Linux prototype with basic audio routing
3. **Phase 3**: Windows driver implementation
4. **Phase 4**: macOS driver implementation
5. **Phase 5**: MIDI support across all platforms
6. **Phase 6**: Configuration utility and testing

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

[To be determined]

## Resources

### Linux
- [ALSA Documentation](https://www.alsa-project.org/wiki/Main_Page)
- [PulseAudio Documentation](https://www.freedesktop.org/wiki/Software/PulseAudio/)

### Windows
- [Windows Driver Kit Documentation](https://docs.microsoft.com/en-us/windows-hardware/drivers/)
- [WASAPI Documentation](https://docs.microsoft.com/en-us/windows/win32/coreaudio/wasapi)

### macOS
- [CoreAudio Documentation](https://developer.apple.com/documentation/coreaudio)
- [Audio HAL Documentation](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/CoreAudioOverview/)

## Status

🚧 **Project is in active development** 🚧

### Current Status by Platform

- **Linux**: ✅ ALSA loopback implementation with userspace tools
- **Windows**: ✅ WASAPI userspace implementation (WDM driver in progress)
- **macOS**: ✅ Virtual sine wave device with CoreAudio (HAL plugin in progress)

### What's Working

- Cross-platform build system (CMake)
- Common audio utilities and sine wave generation
- Linux ALSA loopback device integration
- Windows WASAPI audio playback and capture
- **macOS virtual sine wave device with loopback support**
- Comprehensive test suite

### Next Steps

- Windows WDM kernel driver development
- macOS CoreAudio driver implementation
- MIDI support across all platforms
- Advanced routing and mixing features
