# Linux Virtual Sound Card - Quick Start Guide

Get up and running with the Linux virtual sound card in 5 minutes.

## Prerequisites

Install ALSA development libraries:

```bash
# Ubuntu/Debian
sudo apt-get install libasound2-dev alsa-utils

# Fedora/RHEL
sudo dnf install alsa-lib-devel alsa-utils

# Arch Linux
sudo pacman -S alsa-lib alsa-utils
```

## Build

Choose one of these methods:

### Method 1: CMake (Recommended for full project)

```bash
cd /path/to/virtual-sound-card
mkdir build && cd build
cmake ..
cmake --build .

# Programs are in build/linux/
./linux/sine_generator_app
```

### Method 2: Makefile (Quick Linux-only build)

```bash
cd /path/to/virtual-sound-card/linux
make

# Programs are in build/
./build/sine_generator_app
```

## Setup

Load the ALSA loopback kernel module:

```bash
sudo modprobe snd-aloop
```

Verify it's loaded:

```bash
aplay -l | grep Loopback
```

You should see:
```
card X: Loopback [Loopback], device 0: Loopback PCM [Loopback PCM]
card X: Loopback [Loopback], device 1: Loopback PCM [Loopback PCM]
```

## Test

### Quick Test (Automated)

```bash
cd linux
make test
```

This will:
1. Start sine generator in background
2. Run verification test
3. Report results

### Manual Test

**Terminal 1** - Generate audio:
```bash
./build/sine_generator_app 440 10
```

**Terminal 2** - Verify audio:
```bash
./build/test_loopback_read
```

You should see:
```
=== Analysis Results ===
Signal amplitude OK (RMS: xxxxx, mean: x.xx)
Detected frequency: 440.xx Hz
PASS: Frequency within tolerance (x.xx Hz)

=== TEST PASSED ===
```

## Usage Examples

### Generate Different Frequencies

```bash
# 440 Hz (A4) for 5 seconds
./build/sine_generator_app 440 5

# 880 Hz (A5) for 10 seconds
./build/sine_generator_app 880 10

# 261.63 Hz (Middle C) for 3 seconds
./build/sine_generator_app 261.63 3
```

### Test with Standard ALSA Tools

Record from loopback while playing:
```bash
# Terminal 1: Play audio
speaker-test -D hw:Loopback,0,0 -c 2 -t sine

# Terminal 2: Record it
arecord -D hw:Loopback,1,0 -f cd -d 5 test.wav

# Terminal 3: Play the recording
aplay test.wav
```

### Route Audio Between Applications

Route audio from application A to application B:
```bash
# App A plays to: hw:Loopback,0,0
# App B records from: hw:Loopback,1,0
```

## Troubleshooting

### "Module snd-aloop not found"

The kernel module is not available. Solutions:

1. Install kernel extras:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install linux-modules-extra-$(uname -r)
   
   # Fedora/RHEL
   sudo dnf install kernel-modules-extra
   ```

2. Rebuild kernel with CONFIG_SND_ALOOP=m

### "No such device"

1. Ensure module is loaded:
   ```bash
   lsmod | grep snd_aloop
   ```

2. If not loaded:
   ```bash
   sudo modprobe snd-aloop
   ```

### "Permission denied"

Add your user to the audio group:
```bash
sudo usermod -a -G audio $USER
# Log out and back in for changes to take effect
```

## Make it Permanent

To load snd-aloop on boot:

```bash
echo "snd-aloop" | sudo tee /etc/modules-load.d/snd-aloop.conf
```

## Next Steps

- Read [README.md](README.md) for detailed documentation
- See [../docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md) for system architecture
- Explore the [API](../common/vcard.h) for integration

## Getting Help

- Check [README.md](README.md) for detailed troubleshooting
- Open an issue: https://github.com/doumdi/virtual-sound-card/issues
- Read ALSA docs: https://www.alsa-project.org/wiki/Main_Page
