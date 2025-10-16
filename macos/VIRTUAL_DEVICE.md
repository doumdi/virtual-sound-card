# Virtual Sine Wave Device for macOS

This directory contains a virtual sine wave device implementation for macOS using CoreAudio.

## Overview

The `virtual_sine_device` program generates a continuous sine wave that can be output to any audio device, including virtual loopback devices. When used with a virtual audio device like BlackHole, other applications can read the sine wave as an input source.

## Quick Start

### Prerequisites

You need a virtual audio device to create a loopback. We recommend **BlackHole**:

```bash
# Install BlackHole via Homebrew
brew install blackhole-2ch
```

Alternatively, download from: https://github.com/ExistentialAudio/BlackHole/releases

### Building

```bash
cd /path/to/virtual-sound-card
mkdir build && cd build
cmake -DBUILD_MACOS=ON ..
cmake --build .
```

The programs will be built in `build/macos/`:
- `virtual_sine_device` - Virtual sine wave generator
- `sine_generator_app` - Simple sine wave player
- `test_loopback_read` - Audio verification test

### Usage

1. **Start the virtual sine device:**

```bash
# Output to BlackHole (or your virtual device)
./virtual_sine_device -d "BlackHole 2ch" -f 440

# List available devices
./virtual_sine_device -l

# Custom frequency and amplitude
./virtual_sine_device -d "BlackHole 2ch" -f 880 -a 0.3
```

2. **Read from the virtual device in another application:**

Use any audio application and select "BlackHole 2ch" as the input device. The application will receive the sine wave audio.

3. **Test with the loopback read tool:**

```bash
# In terminal 1: Start virtual sine device
./virtual_sine_device -d "BlackHole 2ch" -f 440

# In terminal 2: Set BlackHole as default input, then test
# (Or modify test_loopback_read.c to specify BlackHole explicitly)
./test_loopback_read
```

## Command Line Options

```
./virtual_sine_device [options]

Options:
  -f <frequency>   Sine wave frequency in Hz (default: 440.0)
  -d <device>      Output device name (e.g., "BlackHole 2ch")
  -r <rate>        Sample rate in Hz (default: 48000)
  -c <channels>    Number of channels (default: 2)
  -a <amplitude>   Amplitude 0.0-1.0 (default: 0.5)
  -l               List available audio devices
  -h               Show help message
```

## How It Works

1. **Virtual Audio Device**: BlackHole or similar creates a virtual audio cable that allows audio to be routed between applications.

2. **Sine Wave Generator**: `virtual_sine_device` continuously generates a sine wave and outputs it to the specified virtual device.

3. **Input to Applications**: Other applications can select the virtual device as their input source and receive the sine wave audio in real-time.

This is similar to the Linux implementation using `snd-aloop`, where audio written to one end can be read from the other end.

## Alternative: Using Audio MIDI Setup

If you don't want to install BlackHole, you can create a Multi-Output Device:

1. Open **Audio MIDI Setup** (`/Applications/Utilities/Audio MIDI Setup.app`)
2. Click the **+** button (bottom left)
3. Select **Create Multi-Output Device**
4. Check your desired output device(s)
5. Enable **Drift Correction** for your main output
6. Name it "Virtual Sound Card"

Then use it with:
```bash
./virtual_sine_device -d "Virtual Sound Card" -f 440
```

**Note:** Multi-Output Device is primarily for playback. For capturing audio, BlackHole is recommended.

## Testing

### Test the Virtual Device

```bash
# Terminal 1: Start virtual device
./virtual_sine_device -d "BlackHole 2ch" -f 440

# Terminal 2: Record from BlackHole
sox -t coreaudio "BlackHole 2ch" test_output.wav trim 0 5

# Play back the recorded audio
afplay test_output.wav
```

### Verify with System Tools

```bash
# List all audio devices
system_profiler SPAudioDataType

# Check if BlackHole is recognized
system_profiler SPAudioDataType | grep BlackHole
```

## Examples

### Example 1: 440Hz Tone to BlackHole

```bash
./virtual_sine_device -d "BlackHole 2ch" -f 440
```

Now any application using "BlackHole 2ch" as input will receive a 440Hz sine wave.

### Example 2: Custom Frequency and Amplitude

```bash
./virtual_sine_device -d "BlackHole 2ch" -f 880 -a 0.3
```

Generates an 880Hz tone at 30% amplitude.

### Example 3: Test with QuickTime Player

1. Start the virtual device:
   ```bash
   ./virtual_sine_device -d "BlackHole 2ch" -f 440
   ```

2. Open QuickTime Player
3. File → New Audio Recording
4. Click the dropdown next to the record button
5. Select "BlackHole 2ch" as the microphone
6. You should see the audio levels responding to the sine wave

## Troubleshooting

### "Device not found" Error

- List available devices: `./virtual_sine_device -l`
- Make sure BlackHole or your virtual device is installed
- Check the exact device name (case-sensitive)

### No Audio in Applications

- Verify the virtual device is selected as the input source in the application
- Check that the virtual sine device is running
- Try increasing the amplitude: `-a 0.8`

### Permission Issues

macOS may require microphone permissions for some applications. Grant permissions in:
**System Preferences → Security & Privacy → Privacy → Microphone**

## References

- [BlackHole Virtual Audio Driver](https://github.com/ExistentialAudio/BlackHole)
- [CoreAudio Documentation](https://developer.apple.com/documentation/coreaudio)
- [Audio MIDI Setup User Guide](https://support.apple.com/guide/audio-midi-setup)
