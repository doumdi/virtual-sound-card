# macOS Virtual Sound Card - Quick Start Guide

Get started with the macOS virtual sine wave device in 5 minutes.

## Prerequisites

Install BlackHole (free, open-source virtual audio driver):

```bash
brew install blackhole-2ch
```

Or download from: https://github.com/ExistentialAudio/BlackHole/releases

## Build

```bash
cd /path/to/virtual-sound-card
mkdir build && cd build
cmake -DBUILD_MACOS=ON ..
cmake --build .
```

## Run

### Option 1: Run the Demo

```bash
cd /path/to/virtual-sound-card/macos
./demo.sh
```

### Option 2: Manual Setup

**Terminal 1 - Start the virtual device:**
```bash
cd build/macos
./virtual_sine_device -d "BlackHole 2ch" -f 440
```

**Terminal 2 - Test with recording:**
```bash
# Record 5 seconds from BlackHole
sox -t coreaudio "BlackHole 2ch" output.wav trim 0 5

# Play it back
afplay output.wav
```

### Option 3: Use with Applications

1. Start the virtual device:
   ```bash
   ./virtual_sine_device -d "BlackHole 2ch" -f 440
   ```

2. In any audio application (QuickTime Player, GarageBand, etc.):
   - Select "BlackHole 2ch" as the input device
   - The application will receive a 440Hz sine wave

## What's Happening?

1. **BlackHole** creates a virtual audio cable (like a loopback)
2. **virtual_sine_device** generates a sine wave and sends it to BlackHole's output
3. **Other applications** can read from BlackHole's input and receive the sine wave

This is the macOS equivalent of Linux's `snd-aloop` module.

## Command Options

```bash
./virtual_sine_device -h
```

Common options:
- `-f 440` - Set frequency to 440Hz
- `-d "BlackHole 2ch"` - Output to BlackHole
- `-a 0.5` - Set amplitude (0.0-1.0)
- `-l` - List available audio devices

## Examples

### 880Hz tone
```bash
./virtual_sine_device -d "BlackHole 2ch" -f 880
```

### Lower volume (30%)
```bash
./virtual_sine_device -d "BlackHole 2ch" -f 440 -a 0.3
```

### Different sample rate
```bash
./virtual_sine_device -d "BlackHole 2ch" -f 440 -r 44100
```

## Troubleshooting

**"Device not found" error:**
- Run `./virtual_sine_device -l` to list devices
- Make sure BlackHole is installed: `brew install blackhole-2ch`

**No audio in applications:**
- Make sure the virtual device is running
- Check that the application has "BlackHole 2ch" selected as input
- Try increasing amplitude: `-a 0.8`

**Permission errors:**
- Grant microphone permission in System Preferences → Security & Privacy → Privacy → Microphone

## Next Steps

See [VIRTUAL_DEVICE.md](VIRTUAL_DEVICE.md) for complete documentation.
