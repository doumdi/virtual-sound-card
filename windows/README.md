# Windows Virtual Sound Card Driver

This directory contains the Windows implementation of the virtual sound card driver using Windows Driver Model (WDM) and WASAPI.

## Overview

The Windows driver creates a virtual audio device that appears in the Windows Sound Control Panel and can be used by any Windows audio application.

### Virtual Audio Cable Solutions (Loopback)

Similar to Linux's ALSA loopback (`snd-aloop`) and macOS's BlackHole, Windows has several virtual audio cable solutions for audio loopback and routing:

#### Recommended Solutions

1. **VB-Cable (Free)** - [Download](https://vb-audio.com/Cable/)
   - Free virtual audio device driver
   - Creates one pair of virtual input/output devices
   - Low latency and stable
   - Best for basic virtual audio routing

2. **Voicemeeter (Free)** - [Download](https://vb-audio.com/Voicemeeter/)
   - Advanced virtual audio mixer with virtual I/O
   - Includes virtual audio cable functionality
   - Multiple virtual devices
   - GUI for audio routing and mixing

3. **Virtual Audio Cable (Commercial)** - [Download](https://vac.muzychenko.net/en/)
   - Professional virtual audio cable driver
   - Multiple virtual cables (up to 256)
   - Low latency, high quality
   - Advanced configuration options

4. **JACK Audio Connection Kit** - [Download](https://jackaudio.org/)
   - Professional audio server for low-latency routing
   - Cross-platform solution
   - Advanced MIDI and audio routing
   - Best for complex audio setups

These solutions provide the same core functionality as BlackHole on macOS and ALSA loopback on Linux, allowing applications to route audio between each other through virtual devices.

## Quick Example with VB-Cable

The `virtual_sine_device` program demonstrates virtual audio loopback on Windows:

```cmd
# 1. Install VB-Cable from https://vb-audio.com/Cable/

# 2. Build the project
mkdir build && cd build
cmake -DBUILD_WINDOWS=ON ..
cmake --build . --config Release

# 3. Generate continuous sine wave to VB-Cable
cd windows\Release
virtual_sine_device.exe -d "CABLE Input" -f 440

# 4. In another application, record from "CABLE Output"
# The application will receive the 440Hz sine wave
```

This is equivalent to:
- **Linux**: `sine_generator_app` playing to `hw:Loopback,0,0`
- **macOS**: `virtual_sine_device -d "BlackHole 2ch" -f 440`

## Architecture

The implementation consists of:
- **WDM Kernel Driver**: Core audio driver (AVStream/PortCls)
- **User-mode Driver**: WASAPI endpoint configuration
- **Control Panel Application**: Configuration GUI
- **Installation Package**: MSI installer

## Prerequisites

### Development Requirements

- Windows 10/11
- Visual Studio 2019 or later
- Windows SDK (10.0.19041.0 or later)
- Windows Driver Kit (WDK)
- Spectre-mitigated libraries

### Installation

1. Install Visual Studio with C++ Desktop Development workload
2. Install Windows SDK
3. Install Windows Driver Kit (WDK)
4. Install WDK Visual Studio extensions

Download from: https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk

## Building

### Using Visual Studio

1. Open `VirtualSoundCard.sln` in Visual Studio
2. Select configuration (Debug/Release) and platform (x64/ARM64)
3. Build → Build Solution (Ctrl+Shift+B)

### Using Command Line

```cmd
msbuild VirtualSoundCard.sln /p:Configuration=Release /p:Platform=x64
```

## Installation

### Developer Mode (Test Signing)

Enable test signing:
```cmd
bcdedit /set testsigning on
```

Install the driver:
```cmd
cd build\Release\x64
pnputil /add-driver VirtualSoundCard.inf /install
```

### Production (Signed Driver)

The driver must be signed with an EV certificate for production deployment:
1. Obtain EV certificate from Microsoft-approved CA
2. Sign the driver package
3. Submit to Microsoft Hardware Dev Center for attestation signing
4. Distribute via Windows Update or custom installer

## Configuration

The virtual sound card supports configuration through:

- **Device Manager**: View/update driver properties
- **Sound Control Panel**: Set as default device, configure channels
- **Control Panel App**: Advanced configuration (sample rate, buffer size, MIDI)

### Configurable Parameters

- Input channels: 1-32
- Output channels: 1-32
- Sample rates: 44.1kHz, 48kHz, 96kHz, 192kHz
- Bit depths: 16, 24, 32-bit
- Buffer sizes: 128, 256, 512, 1024, 2048 samples

## Development

### File Structure

```
windows/
├── driver/             # WDM kernel driver source
│   ├── miniport/      # Audio miniport driver
│   ├── adapter/       # Adapter driver
│   └── VirtualSoundCard.inf
├── usermode/          # User-mode components
│   └── wasapi/        # WASAPI endpoint
├── control/           # Control panel application
├── installer/         # MSI installer project
├── tests/             # Test applications
└── README.md          # This file
```

### Debugging

Use WinDbg or Visual Studio debugger with kernel debugging:

```cmd
# Set up kernel debugging (requires 2 machines or VM)
bcdedit /debug on
bcdedit /dbgsettings serial debugport:1 baudrate:115200
```

View debug output:
```cmd
# Use DebugView for user-mode debugging
# Use WinDbg for kernel-mode debugging
```

## Testing

### Manual Testing

1. Install the driver
2. Open Sound Control Panel (mmsys.cpl)
3. Verify virtual device appears in playback/recording devices
4. Test with audio application (e.g., Audacity, VLC)

### Automated Testing

```cmd
cd tests
run_tests.bat
```

## Code Signing

### Test Signing (Development)

```cmd
signtool sign /v /s "PrivateCertStore" /n "TestCert" /t http://timestamp.digicert.com VirtualSoundCard.sys
```

### Production Signing

1. Sign with EV certificate
2. Create driver package
3. Submit to Hardware Dev Center
4. Download attestation-signed package

## Known Issues

- None yet (project in development)

## References

- [Windows Driver Kit Documentation](https://docs.microsoft.com/en-us/windows-hardware/drivers/)
- [Audio Device Driver Reference](https://docs.microsoft.com/en-us/windows-hardware/drivers/audio/)
- [WASAPI Documentation](https://docs.microsoft.com/en-us/windows/win32/coreaudio/wasapi)
- [AVStream Minidriver Development](https://docs.microsoft.com/en-us/windows-hardware/drivers/stream/avstream-overview)
