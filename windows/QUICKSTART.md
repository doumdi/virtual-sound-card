# Windows Virtual Sound Card - Quick Start Guide

Get up and running with the Windows virtual sound card in 5 minutes.

## Prerequisites

### Development Tools

Install one of the following:

**Option 1: Visual Studio (Recommended)**
- Visual Studio 2019 or later
- C++ Desktop Development workload
- Windows SDK (10.0.19041.0 or later)

Download from: https://visualstudio.microsoft.com/downloads/

**Option 2: MinGW-w64**
- MinGW-w64 compiler
- CMake 3.15 or later

Download from: https://www.mingw-w64.org/

### For Driver Development (Optional)

For WDM kernel driver development:
- Windows Driver Kit (WDK)
- WDK Visual Studio extensions

Download from: https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk

## Build

Choose one of these methods:

### Method 1: CMake (Recommended for full project)

```cmd
cd C:\path\to\virtual-sound-card
mkdir build
cd build
cmake ..
cmake --build . --config Release

REM Programs are in build\windows\Release\
cd windows\Release
sine_generator_app.exe
```

### Method 2: Makefile with NMAKE (Quick Windows build)

```cmd
cd C:\path\to\virtual-sound-card\windows
nmake

REM Programs are in build\
build\sine_generator_app.exe
```

### Method 3: MinGW

```cmd
cd C:\path\to\virtual-sound-card
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .

REM Programs are in build\windows\
windows\sine_generator_app.exe
```

## Setup

The current implementation uses WASAPI for user-mode audio testing.

### Virtual Audio Cable Setup (Windows Loopback Solution)

Similar to Linux's `snd-aloop` and macOS's BlackHole, Windows requires a virtual audio cable driver for loopback functionality. Choose one of these options:

**Option 1: VB-Cable (Free, Recommended for beginners)**
- Download from: https://vb-audio.com/Cable/
- Run the installer as Administrator
- Install VB-Cable
- Restart if prompted
- Creates: "CABLE Input" (playback) and "CABLE Output" (recording) devices

**Option 2: Voicemeeter (Free, Advanced features)**
- Download from: https://vb-audio.com/Voicemeeter/
- Includes virtual audio cables plus mixing capabilities
- Best for complex audio routing scenarios
- GUI for real-time audio management

**Option 3: Virtual Audio Cable (Commercial, Professional)**
- Download from: https://vac.muzychenko.net/en/
- Supports multiple virtual cables (up to 256)
- Advanced configuration and low latency
- Best for professional audio production

**Option 4: JACK Audio (Free, Professional)**
- Download from: https://jackaudio.org/
- Cross-platform audio routing server
- Best for complex setups requiring MIDI and audio routing

After installation, the virtual audio devices will appear in:
- Sound Control Panel (mmsys.cpl)
- Settings > System > Sound

### How Virtual Audio Cables Work

Virtual audio cables work like physical audio cables but in software:
- **Playback device** (e.g., "CABLE Input"): Applications play audio to this
- **Recording device** (e.g., "CABLE Output"): Applications record audio from this
- Audio flows: App A → CABLE Input → CABLE Output → App B

This is the Windows equivalent of:
- **Linux**: ALSA loopback (`hw:Loopback,0,0` → `hw:Loopback,1,0`)
- **macOS**: BlackHole (write to BlackHole → read from BlackHole)

## Test

### Quick Test

**Terminal 1** - Generate audio:
```cmd
sine_generator_app.exe 440 10
```

This plays a 440 Hz sine wave for 10 seconds through your default audio output.

**Terminal 2** - Verify audio (requires virtual cable):
```cmd
test_loopback_read.exe
```

### With Virtual Cable

1. Set VB-Cable Input as your default playback device:
   ```cmd
   control mmsys.cpl
   ```
   - Right-click "CABLE Input" → Set as Default Device

2. Terminal 1 - Generate audio:
   ```cmd
   sine_generator_app.exe 440 10
   ```

3. Terminal 2 - Record and verify:
   ```cmd
   test_loopback_read.exe
   ```

You should see:
```
=== Analysis Results ===
Signal amplitude OK (RMS: x.xxxx, mean: x.xxxx)
Detected frequency: 440.xx Hz
PASS: Frequency within tolerance (x.xx Hz)

=== TEST PASSED ===
```

## Usage Examples

### Generate Different Frequencies

```cmd
REM 440 Hz (A4) for 5 seconds
sine_generator_app.exe 440 5

REM 880 Hz (A5) for 10 seconds
sine_generator_app.exe 880 10

REM 261.63 Hz (Middle C) for 3 seconds
sine_generator_app.exe 261.63 3
```

### Route Audio Between Applications

Using virtual audio cable:

1. Set VB-Cable Input as default playback device
2. Application A plays to default output (goes to VB-Cable Input)
3. Application B records from VB-Cable Output
4. Audio flows: App A → VB-Cable → App B

### Test with Windows Audio Tools

```cmd
REM List audio devices
powershell -Command "Get-AudioDevice -List"

REM Record audio (requires PowerShell AudioDeviceCmdlets)
powershell -Command "Start-AudioRecording -Path test.wav -Seconds 5"
```

## Troubleshooting

### "Failed to initialize COM"

Run from a normal command prompt (not PowerShell ISE or certain IDEs).

### "Failed to get default audio endpoint"

1. Check that audio devices are enabled:
   ```cmd
   control mmsys.cpl
   ```

2. Ensure Windows Audio service is running:
   ```cmd
   sc query Audiosrv
   ```

3. If stopped, start it:
   ```cmd
   sc start Audiosrv
   ```

### "This program is designed for Windows only"

You're trying to run the program on Linux. The stub executable provides this message.
Build on Windows with Visual Studio or MinGW for the actual implementation.

### No audio output

1. Check volume mixer:
   ```cmd
   sndvol
   ```

2. Verify default device:
   ```cmd
   control mmsys.cpl
   ```

3. Test system audio:
   ```cmd
   explorer shell:AppsFolder\Microsoft.ZuneMusic_8wekyb3d8bbwe!Microsoft.ZuneMusic
   ```

## Configuration

### Change Default Audio Device

```cmd
REM Open Sound settings
control mmsys.cpl

REM Or via Settings
start ms-settings:sound
```

Select your desired device in the Playback or Recording tab.

## Development Notes

### Current Implementation

The current implementation provides:
- **User-mode WASAPI applications**: For testing and demonstration
- **Cross-platform build support**: CMake and Makefile
- **Stub programs**: For non-Windows platforms

### Future Implementation

The full Windows driver implementation will include:
- **WDM Kernel Driver**: Core audio driver using AVStream/PortCls
- **User-mode Driver**: WASAPI endpoint configuration
- **Control Panel Application**: Configuration GUI
- **Installation Package**: MSI installer
- **Code Signing**: For Windows 10/11 installation

### Building the Driver (Future)

When the WDM driver is implemented:

```cmd
REM Open driver solution
start VirtualSoundCard.sln

REM Build in Visual Studio
msbuild VirtualSoundCard.sln /p:Configuration=Release /p:Platform=x64

REM Enable test signing
bcdedit /set testsigning on

REM Install driver
cd build\Release\x64
pnputil /add-driver VirtualSoundCard.inf /install
```

## Next Steps

- Read [README.md](README.md) for detailed documentation
- See [../docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md) for system architecture
- Explore the [API](../common/vcard.h) for integration
- Review [CONTRIBUTING.md](../CONTRIBUTING.md) for contribution guidelines

## Getting Help

- Check [README.md](README.md) for detailed troubleshooting
- Open an issue: https://github.com/doumdi/virtual-sound-card/issues
- Read WASAPI docs: https://docs.microsoft.com/en-us/windows/win32/coreaudio/wasapi
- WDK documentation: https://docs.microsoft.com/en-us/windows-hardware/drivers/

## Resources

### Virtual Audio Cables (Windows Loopback Solutions)

These are the Windows equivalents to Linux's ALSA loopback and macOS's BlackHole:

- **VB-Cable (Free)**: https://vb-audio.com/Cable/
- **Voicemeeter (Free)**: https://vb-audio.com/Voicemeeter/
- **Virtual Audio Cable (Commercial)**: https://vac.muzychenko.net/en/
- **JACK Audio (Free)**: https://jackaudio.org/

### Windows Audio Development

- WASAPI: https://docs.microsoft.com/en-us/windows/win32/coreaudio/wasapi
- Core Audio APIs: https://docs.microsoft.com/en-us/windows/win32/coreaudio/core-audio-apis
- WDK: https://docs.microsoft.com/en-us/windows-hardware/drivers/
