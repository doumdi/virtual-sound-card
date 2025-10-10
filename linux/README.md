# Linux Virtual Sound Card Driver

This directory contains the Linux implementation of the virtual sound card driver using ALSA (Advanced Linux Sound Architecture).

## Overview

The Linux driver creates a virtual ALSA device that can be used by any application supporting ALSA or PulseAudio.

## Architecture

The implementation uses:
- **ALSA Loopback Module**: For basic audio routing
- **Custom Kernel Module** (optional): For advanced features and lower latency
- **User-space Control**: Configuration tools

## Prerequisites

### Build Dependencies

```bash
# Debian/Ubuntu
sudo apt-get install build-essential linux-headers-$(uname -r) alsa-utils libasound2-dev

# Fedora/RHEL
sudo dnf install kernel-devel alsa-lib-devel gcc make

# Arch Linux
sudo pacman -S base-devel linux-headers alsa-utils alsa-lib
```

## Building

```bash
cd linux
make
```

## Installation

```bash
sudo make install
```

## Usage

### Loading the Module

```bash
# Load with default configuration (2 input, 2 output channels)
sudo modprobe snd-vcard

# Load with custom configuration
sudo modprobe snd-vcard channels_in=4 channels_out=4
```

### Verifying Installation

```bash
# List ALSA devices
aplay -l

# Test playback
aplay -D hw:VirtualCard test.wav
```

## Configuration

Module parameters:
- `channels_in`: Number of input channels (default: 2, max: 32)
- `channels_out`: Number of output channels (default: 2, max: 32)
- `sample_rate`: Sample rate in Hz (default: 48000)
- `buffer_size`: Buffer size in frames (default: 1024)

## Development

### File Structure

```
linux/
├── kernel/         # Kernel module source
├── userspace/      # User-space utilities
├── tests/          # Test scripts and programs
├── Makefile        # Build configuration
└── README.md       # This file
```

### Debugging

Enable kernel debug messages:
```bash
echo 8 > /proc/sys/kernel/printk
dmesg -w | grep vcard
```

## Testing

```bash
# Run test suite
make test

# Manual testing with aplay/arecord
arecord -D hw:VirtualCard -f cd | aplay -D hw:VirtualCard -f cd
```

## Known Issues

- None yet (project in development)

## References

- [ALSA Driver Documentation](https://www.alsa-project.org/wiki/Main_Page)
- [Linux Kernel Module Programming Guide](https://tldp.org/LDP/lkmpg/2.6/html/)
- [Writing an ALSA Driver](https://www.kernel.org/doc/html/latest/sound/kernel-api/writing-an-alsa-driver.html)
