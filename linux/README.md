# Linux Virtual Sound Card Driver

This directory contains the Linux implementation of the virtual sound card driver using ALSA (Advanced Linux Sound Architecture).

**Quick Start:** See [QUICKSTART.md](QUICKSTART.md) for a 5-minute setup guide.

**Demo:** Run `./demo.sh` for an automated demonstration.

## Overview

The Linux driver creates a virtual ALSA device that can be used by any application supporting ALSA or PulseAudio.

For cross-platform professional audio routing, see also: [JACK Audio Connection Kit](#jack-audio-connection-kit)

## Architecture

The implementation uses:
- **ALSA Loopback Module**: For basic audio routing
- **Custom Kernel Module** (optional): For advanced features and lower latency
- **User-space Control**: Configuration tools
- **JACK2** (optional): For professional, low-latency cross-platform audio routing

## Prerequisites

### Build Dependencies

```bash
# Debian/Ubuntu
sudo apt-get install build-essential linux-headers-$(uname -r) alsa-utils libasound2-dev

# Optional: JACK2 for cross-platform audio
sudo apt-get install jackd2 libjack-jackd2-dev qjackctl

# Fedora/RHEL
sudo dnf install kernel-devel alsa-lib-devel gcc make

# Optional: JACK2
sudo dnf install jack-audio-connection-kit-devel qjackctl

# Arch Linux
sudo pacman -S base-devel linux-headers alsa-utils alsa-lib

# Optional: JACK2
sudo pacman -S jack2 qjackctl
```

## Building

### Using CMake (Recommended)

```bash
cd /path/to/virtual-sound-card
mkdir build && cd build
cmake ..
cmake --build .
```

The Linux programs will be built in `build/linux/`:
- `sine_generator_app` - Sine wave generator
- `test_loopback_read` - Audio verification test

### Using Makefile

```bash
cd linux
make
```

Programs will be built in `linux/build/`:
- `sine_generator_app` - Sine wave generator
- `test_loopback_read` - Audio verification test

## Installation

```bash
cd linux
sudo make install
```

This installs programs to `/usr/local/bin/`.

## Usage

### Loading the Module

The Linux implementation uses the ALSA loopback module (`snd-aloop`) which provides a virtual sound card.

```bash
# Load the ALSA loopback module
sudo modprobe snd-aloop

# Verify it loaded successfully
aplay -l | grep Loopback
```

The module creates a loopback device with two subdevices:
- `hw:Loopback,0,0` - Playback subdevice (write audio here)
- `hw:Loopback,1,0` - Capture subdevice (read audio here)

### Running the Sine Wave Generator

```bash
# Generate a 440Hz sine wave for 5 seconds (default)
./build/sine_generator_app

# Generate a 880Hz sine wave for 10 seconds
./build/sine_generator_app 880 10

# Or if installed:
sine_generator_app 880 10
```

### Testing the Loopback

In one terminal, run the sine wave generator:
```bash
./build/sine_generator_app 440 10
```

In another terminal, run the test to verify:
```bash
./build/test_loopback_read
```

The test will:
1. Read audio from the loopback device
2. Analyze the signal amplitude
3. Detect the frequency using zero-crossing detection
4. Verify it matches the expected 440Hz tone

### Verifying Installation

```bash
# List ALSA devices
aplay -l

# Test playback
aplay -D hw:VirtualCard test.wav
```

## Configuration

### ALSA Loopback Module Parameters

The `snd-aloop` module can be loaded with custom parameters:

```bash
# Load with custom configuration
sudo modprobe snd-aloop pcm_substreams=2 index=1

# Remove and reload with different settings
sudo modprobe -r snd-aloop
sudo modprobe snd-aloop pcm_substreams=4
```

Module parameters:
- `index`: Sound card index (default: -1, auto-assign)
- `id`: Sound card ID string (default: "Loopback")
- `pcm_substreams`: Number of subdevice pairs (default: 8, max: 8)
- `pcm_notify`: Notification for PCM (default: 0)

### Application Configuration

The sine wave generator uses:
- Sample rate: 48000 Hz
- Channels: 2 (stereo)
- Format: S16_LE (16-bit signed little-endian)
- Buffer size: 1024 frames

## Development

### Implementation Details

This implementation uses the ALSA loopback module (`snd-aloop`) instead of a custom kernel driver. This approach:
- Uses existing, well-tested kernel functionality
- Requires no custom kernel module compilation
- Provides full ALSA device capabilities
- Works across different kernel versions

The loopback device creates virtual audio cables where:
- Audio written to one subdevice can be read from its paired subdevice
- Multiple applications can use the device simultaneously
- Supports standard ALSA configuration and routing

### File Structure

```
linux/
├── userspace/          # User-space utilities
│   └── sine_generator_app.c   # Sine wave generator
├── tests/              # Test programs
│   └── test_loopback_read.c   # Loopback verification test
├── Makefile            # Build configuration
├── CMakeLists.txt      # CMake build configuration
└── README.md           # This file
```

### Debugging

Enable kernel debug messages:
```bash
echo 8 > /proc/sys/kernel/printk
dmesg -w | grep vcard
```

## Testing

### Automated Test

```bash
cd linux
make setup    # Load snd-aloop module
make test     # Run automated test suite
```

The automated test:
1. Starts the sine wave generator in the background
2. Runs the loopback read test
3. Verifies frequency and amplitude
4. Reports pass/fail

### Manual Testing

Terminal 1 - Generate sine wave:
```bash
./build/sine_generator_app 440 10
```

Terminal 2 - Verify audio:
```bash
./build/test_loopback_read
```

### Testing with ALSA Tools

```bash
# Record from loopback and play to default output
arecord -D hw:Loopback,1,0 -f cd | aplay

# Generate test tone with speaker-test
speaker-test -D hw:Loopback,0,0 -c 2 -t sine

# In another terminal, record it
arecord -D hw:Loopback,1,0 -f cd -d 5 test.wav
```

## Known Issues

- The `snd-aloop` module may not be available in some minimal kernel configurations
- On some systems, you may need to install `linux-modules-extra-$(uname -r)` package
- Audio latency depends on buffer size and system load

## JACK Audio Connection Kit

For professional, low-latency audio routing that works across Linux, macOS, and Windows:

### Installation

```bash
# Ubuntu/Debian
sudo apt-get install jackd2 libjack-jackd2-dev qjackctl

# Fedora/RHEL
sudo dnf install jack-audio-connection-kit-devel qjackctl

# Arch Linux
sudo pacman -S jack2 qjackctl
```

### Usage

```bash
# Start JACK server
# Note: May need to specify device and buffer settings
# Example: jackd -d alsa -d hw:0 -r 48000 -p 1024
jackd -d alsa

# Or use QjackCtl GUI (recommended for easier configuration)
qjackctl

# Build with JACK support
cd /path/to/virtual-sound-card
mkdir build && cd build
cmake -DBUILD_JACK=ON ..
cmake --build .

# Run JACK sine wave generator
./jack_sine_generator 440 10

# Or use the demo
../jack_demo.sh
```

### Benefits of JACK

- **Cross-platform**: Same code works on Linux, macOS, and Windows
- **Low latency**: Optimized for real-time audio processing
- **Professional routing**: Connect any audio application to any other
- **Sample-accurate**: Precise synchronization between applications
- **Active community**: Well-supported and widely used in audio production

Learn more: https://jackaudio.org/

## References

- [ALSA Driver Documentation](https://www.alsa-project.org/wiki/Main_Page)
- [Linux Kernel Module Programming Guide](https://tldp.org/LDP/lkmpg/2.6/html/)
- [Writing an ALSA Driver](https://www.kernel.org/doc/html/latest/sound/kernel-api/writing-an-alsa-driver.html)
- [JACK Audio Connection Kit](https://jackaudio.org/)
