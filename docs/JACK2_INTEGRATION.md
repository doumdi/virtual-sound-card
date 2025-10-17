# JACK2 Integration Guide

This document describes the JACK2 (JACK Audio Connection Kit) integration in the Virtual Sound Card project.

## Overview

JACK2 is a professional audio server that provides low-latency audio routing across applications. This project now includes full JACK2 support on all major operating systems: Linux, Windows, and macOS.

## Features

- **Cross-platform**: Works identically on Linux, Windows, and macOS
- **Low latency**: Professional-grade real-time audio performance
- **Flexible routing**: GUI and CLI tools for managing audio connections
- **Optional dependency**: Project builds without JACK2 if not available
- **Easy to use**: Simple API with example implementations

## Installation

### Linux

```bash
# Ubuntu/Debian
sudo apt-get install libjack-jackd2-dev pkg-config

# Fedora/RHEL
sudo dnf install jack-audio-connection-kit-devel

# Arch Linux
sudo pacman -S jack2
```

### macOS

```bash
brew install jack pkg-config
```

### Windows

Download and install JACK2 from: https://jackaudio.org/

## Building

The project automatically detects JACK2 during CMake configuration:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

If JACK2 is found, `jack_sine_generator` will be built for your platform.

## Usage

### Starting JACK Server

**Linux:**
```bash
# Command line
jackd -d alsa &

# Or use GUI
qjackctl
```

**macOS:**
```bash
# Command line
jackd -d coreaudio &

# Or use GUI
qjackctl
```

**Windows:**
```
Start QjackCtl from the Start Menu and click "Start"
```

### Running Examples

Once JACK server is running:

```bash
# Linux
./build/linux/jack_sine_generator 440 5

# macOS
./build/macos/jack_sine_generator 440 5

# Windows
.\build\windows\jack_sine_generator.exe 440 5
```

### Demo Scripts

We provide demo scripts that check for JACK server and run the examples:

```bash
# Linux
cd linux && ./demo_jack.sh

# macOS
cd macos && ./demo_jack.sh

# Windows
cd windows && demo_jack.bat
```

## Implementation Details

### Code Structure

Each platform has an identical `jack_sine_generator.c` implementation with platform-specific includes:

- `linux/userspace/jack_sine_generator.c`
- `windows/userspace/jack_sine_generator.c`
- `macos/userspace/jack_sine_generator.c`

### Key Components

1. **JACK Client**: Created with `jack_client_open()`
2. **Audio Ports**: Two output ports (left and right)
3. **Process Callback**: Real-time audio generation function
4. **Connection Management**: Automatic connection to system outputs
5. **Signal Handling**: Graceful shutdown on Ctrl+C

### Code Example

```c
// Create JACK client
client = jack_client_open("sine_generator", JackNullOption, &status, NULL);

// Register output ports
output_port_left = jack_port_register(client, "output_left",
                                      JACK_DEFAULT_AUDIO_TYPE,
                                      JackPortIsOutput, 0);

// Set process callback
jack_set_process_callback(client, jack_process_callback, NULL);

// Activate client
jack_activate(client);

// Connect to physical outputs
ports = jack_get_ports(client, NULL, NULL,
                       JackPortIsPhysical | JackPortIsInput);
jack_connect(client, jack_port_name(output_port_left), ports[0]);
```

## Testing

The project includes a JACK2 availability test:

```bash
./build/tests/test_jack_availability
```

This test verifies that JACK2 libraries are available and can be linked. It does NOT require a running JACK server.

## CI/CD Integration

GitHub Actions workflows automatically install JACK2 on all platforms:

- **Linux**: `libjack-jackd2-dev pkg-config`
- **macOS**: `jack pkg-config` via Homebrew
- **Windows**: JACK2 installer via automated download

See `.github/workflows/ci.yml` for details.

## Troubleshooting

### "JACK server is not running"

Start the JACK server:
- Linux/macOS: `jackd -d alsa` or `jackd -d coreaudio`
- Windows: Start QjackCtl and click "Start"

### "Failed to open JACK client"

1. Ensure JACK server is running
2. Check JACK server logs
3. Try restarting JACK server

### Build Issues

**Linux**: Install `libjack-jackd2-dev` and `pkg-config`
**macOS**: Install `jack` and `pkg-config` via Homebrew
**Windows**: Ensure JACK2 is installed from https://jackaudio.org/

### No Audio Output

1. Check JACK connections with `jack_lsp` or QjackCtl
2. Verify system audio device is working
3. Check JACK sample rate matches system audio
4. Increase JACK buffer size if experiencing dropouts

## Advantages Over Platform-Specific Solutions

| Feature | JACK2 | ALSA Loopback | BlackHole | VB-Cable |
|---------|-------|---------------|-----------|----------|
| Cross-platform | ✓ | Linux only | macOS only | Windows only |
| Low latency | ✓ | ✓ | ✓ | ✓ |
| GUI routing | ✓ (QjackCtl) | - | - | - |
| MIDI support | ✓ | - | - | - |
| Professional grade | ✓ | ✓ | ✓ | ✓ |
| Active development | ✓ | ✓ | ✓ | Limited |

## Performance

JACK2 provides excellent low-latency performance:

- **Typical latency**: 5-10ms (configurable)
- **Sample rates**: 44.1kHz, 48kHz, 96kHz, 192kHz
- **Bit depths**: 16-bit, 24-bit, 32-bit float
- **CPU usage**: Minimal in real-time thread

## Further Reading

- [JACK Audio Connection Kit](https://jackaudio.org/)
- [JACK API Documentation](http://jackaudio.org/api/)
- [QjackCtl Documentation](https://qjackctl.sourceforge.io/)
- [JACK Tutorial](https://github.com/jackaudio/jackaudio.github.com/wiki/JackUserManual)

## Contributing

To add JACK2 support to new components:

1. Add JACK2 detection to CMakeLists.txt
2. Implement JACK client following existing examples
3. Add tests for new functionality
4. Update documentation
5. Test on all platforms

See [CONTRIBUTING.md](../CONTRIBUTING.md) for more details.
