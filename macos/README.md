# macOS Virtual Sound Card Driver

This directory contains the macOS implementation of the virtual sound card driver using CoreAudio framework.

## Overview

The macOS driver creates a virtual audio device using CoreAudio framework. The device appears in System Preferences and can be used by any CoreAudio-compatible application.

For cross-platform professional audio routing, see also: [JACK Audio Connection Kit](#jack-audio-connection-kit)

### Quick Start: Virtual Sine Wave Device

For immediate use, we provide a **virtual sine wave device** that generates a sine wave when applications read from it:

1. Install BlackHole (virtual audio driver):
   ```bash
   brew install blackhole-2ch
   ```

2. Build the project:
   ```bash
   mkdir build && cd build
   cmake -DBUILD_MACOS=ON ..
   cmake --build .
   ```

3. Start the virtual sine device:
   ```bash
   ./macos/virtual_sine_device -d "BlackHole 2ch" -f 440
   ```

4. Applications can now read sine wave audio from "BlackHole 2ch" as an input device.

See [VIRTUAL_DEVICE.md](VIRTUAL_DEVICE.md) for detailed documentation.

### JACK2 Quick Start (Cross-Platform)

For professional audio routing that works on macOS, Linux, and Windows:

1. Install JACK2:
   ```bash
   brew install jack
   ```

2. Build with JACK support:
   ```bash
   mkdir build && cd build
   cmake -DBUILD_JACK=ON ..
   cmake --build .
   ```

3. Start JACK server and run sine generator:
   ```bash
   jackd -d coreaudio
   ./jack_sine_generator 440 10
   # Or use: ../jack_demo.sh
   ```

## Architecture

The implementation consists of:
- **Audio Driver Extension**: Modern DriverKit-based audio driver (macOS 11+)
- **Kernel Extension (Legacy)**: KEXT-based driver for older macOS versions
- **User Agent**: Background service for device management
- **Configuration Utility**: Menu bar app for settings
- **JACK Integration** (optional): Cross-platform professional audio routing

## Prerequisites

### Development Requirements

- macOS 11.0 (Big Sur) or later
- Xcode 12 or later
- macOS SDK
- Apple Developer account (for code signing)
- Entitlements for DriverKit development

### Optional: JACK2

- JACK Audio Connection Kit: `brew install jack`
- QjackCtl (GUI): `brew install qjackctl`

### Installation

1. Install Xcode from Mac App Store
2. Install Command Line Tools:
   ```bash
   xcode-select --install
   ```
3. Enroll in Apple Developer Program (required for driver signing)
4. (Optional) Install JACK2 for cross-platform audio:
   ```bash
   brew install jack qjackctl
   ```

## Building

### Using Xcode

1. Open `VirtualSoundCard.xcodeproj`
2. Select scheme (VirtualSoundCard or VirtualSoundCard-Legacy)
3. Product → Build (⌘B)

### Using Command Line

```bash
cd macos
xcodebuild -project VirtualSoundCard.xcodeproj -scheme VirtualSoundCard -configuration Release
```

## Installation

### Modern DriverKit Driver (macOS 11+)

```bash
# Build and install driver extension
cd build/Release
sudo driverkit install VirtualSoundCard.dext

# Activate system extension (requires user approval)
systemextensionsctl developer on
```

User must approve the extension in System Preferences → Security & Privacy.

### Legacy Kernel Extension (macOS 10.x)

```bash
# Load the kernel extension
sudo kextload build/Release/VirtualSoundCard.kext

# Make it load on startup
sudo cp -R build/Release/VirtualSoundCard.kext /Library/Extensions/
sudo kextcache -i /
```

**Note**: Kernel extensions require disabling System Integrity Protection (SIP) on modern macOS.

## Configuration

### Command Line

```bash
# List audio devices
system_profiler SPAudioDataType

# Configure virtual device
/Applications/VirtualSoundCard.app/Contents/MacOS/vscard-config --channels-in 4 --channels-out 4
```

### Configuration App

Launch the configuration app from Applications or menu bar icon:
- Set input/output channel count
- Configure sample rate and buffer size
- Enable/disable MIDI support
- Create multiple virtual devices

### Configurable Parameters

- Input channels: 1-32
- Output channels: 1-32
- Sample rates: 44.1kHz, 48kHz, 88.2kHz, 96kHz, 176.4kHz, 192kHz
- Bit depths: 16, 24, 32-bit integer, 32-bit float
- Buffer sizes: 64, 128, 256, 512, 1024, 2048 frames
- Clock source: Internal, External

## Development

### File Structure

```
macos/
├── driver/              # Audio driver implementation
│   ├── DriverKit/      # Modern DriverKit driver (macOS 11+)
│   └── KEXT/           # Legacy kernel extension (macOS 10.x)
├── user-agent/         # Background service
├── config-app/         # Configuration application
├── common/             # Shared code
├── tests/              # Test applications
└── README.md           # This file
```

### Code Structure

Key components:
- **Audio Driver**: Implements IOAudioDevice or DriverKit AudioDriverKit
- **HAL Plugin**: CoreAudio HAL plugin for device control
- **User Agent**: XPC service for driver communication
- **Config App**: SwiftUI/AppKit application

### Debugging

Enable audio HAL debugging:
```bash
# Enable verbose audio HAL logging
sudo defaults write com.apple.audio.coreaudiod "Log Level" -int 7

# View logs
log stream --predicate 'subsystem == "com.apple.audio"' --level debug
```

Debug driver:
```bash
# DriverKit driver logging
log stream --predicate 'subsystem == "com.virtualcard.driver"' --level debug

# Kernel extension logging (legacy)
sudo dmesg | grep VirtualSoundCard
```

## Testing

### Manual Testing

1. Install the driver
2. Open Audio MIDI Setup.app
3. Verify virtual device appears
4. Configure device properties
5. Test with audio applications (GarageBand, Logic Pro, etc.)

### Automated Testing

```bash
cd tests
./run_tests.sh
```

### Audio Testing

```bash
# List devices
system_profiler SPAudioDataType

# Play test tone through virtual device
afplay -q 1 --device "Virtual Sound Card" test.wav

# Record from virtual device
sox -t coreaudio "Virtual Sound Card" output.wav
```

## Code Signing and Notarization

### Development

```bash
# Sign with development certificate
codesign --sign "Apple Development: Your Name" --timestamp VirtualSoundCard.dext
```

### Distribution

1. Sign with Developer ID certificate:
   ```bash
   codesign --sign "Developer ID Application: Your Name" --timestamp VirtualSoundCard.dext
   ```

2. Notarize with Apple:
   ```bash
   xcrun notarytool submit VirtualSoundCard.zip --apple-id your@email.com --wait
   ```

3. Staple notarization ticket:
   ```bash
   xcrun stapler staple VirtualSoundCard.dext
   ```

## Known Issues

- None yet (project in development)
- DriverKit limitations: Some advanced features require kernel extension
- SIP restrictions: Legacy KEXT requires SIP disabled on modern macOS

## JACK Audio Connection Kit

For professional, low-latency audio routing that works across macOS, Linux, and Windows:

### Installation

```bash
# Install JACK2
brew install jack

# Optional: Install GUI control panel
brew install qjackctl
```

### Usage

```bash
# Build with JACK support
mkdir build && cd build
cmake -DBUILD_JACK=ON ..
cmake --build .

# Start JACK server
# Note: This may temporarily take over system audio
# Use QjackCtl GUI for easier configuration
jackd -d coreaudio

# Or use QjackCtl GUI (recommended)
qjackctl

# Run JACK sine wave generator
./jack_sine_generator 440 10

# Or use the demo
../jack_demo.sh
```

### Benefits of JACK

- **Cross-platform**: Same code works on macOS, Linux, and Windows
- **Low latency**: Optimized for real-time audio processing
- **Professional routing**: Connect any audio application to any other
- **Sample-accurate**: Precise synchronization between applications
- **Active community**: Well-supported and widely used in audio production

Learn more: https://jackaudio.org/

## References

- [CoreAudio Documentation](https://developer.apple.com/documentation/coreaudio)
- [DriverKit Audio Documentation](https://developer.apple.com/documentation/audiodriverkit)
- [Audio Hardware Abstraction Layer](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/CoreAudioOverview/)
- [System Extensions Programming Guide](https://developer.apple.com/documentation/systemextensions)
- [Audio Unit Extensions](https://developer.apple.com/documentation/audiotoolbox/audio_unit_v3_plug-ins)
- [JACK Audio Connection Kit](https://jackaudio.org/)
