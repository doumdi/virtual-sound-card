# Virtual Sound Card Driver

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

**Requirements:**
- Windows 10/11 SDK
- Visual Studio 2019 or later
- Windows Driver Kit (WDK)

### macOS
The macOS implementation uses CoreAudio framework for creating virtual audio devices.

**Technology Stack:**
- CoreAudio framework
- Audio HAL (Hardware Abstraction Layer)
- C/C++/Objective-C

**Requirements:**
- Xcode 12 or later
- macOS SDK
- Apple Developer account (for driver signing)

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
- **macOS**: ⚠️ CoreAudio implementation planned

### What's Working

- Cross-platform build system (CMake)
- Common audio utilities and sine wave generation
- Linux ALSA loopback device integration
- Windows WASAPI audio playback and capture
- Comprehensive test suite

### Next Steps

- Windows WDM kernel driver development
- macOS CoreAudio driver implementation
- MIDI support across all platforms
- Advanced routing and mixing features
