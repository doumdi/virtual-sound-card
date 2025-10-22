# WAVEFORMATEXTENSIBLE Support

## Overview

This document describes the changes made to support `WAVEFORMATEXTENSIBLE` audio format in the Windows virtual sine device implementation.

## Problem Statement

When using virtual audio devices like VB-Cable on Windows, the device reports its format as `WAVEFORMATEXTENSIBLE` (0xFFFE) instead of the simpler `WAVE_FORMAT_IEEE_FLOAT` or `WAVE_FORMAT_PCM` formats. The original implementation only checked for `WAVE_FORMAT_IEEE_FLOAT`, causing it to write silence instead of the sine wave to VB-Cable devices.

## Solution

### 1. Format Detection

Added helper functions to properly detect audio formats that can be either simple or extensible:

- `is_format_ieee_float()`: Detects IEEE Float format (both simple and extensible)
- `is_format_pcm()`: Detects PCM format (both simple and extensible)
- `guid_equals()`: Helper to compare GUIDs for SubFormat detection
- `get_format_description()`: Returns human-readable format description

### 2. PCM Support

Added `sine_generator_process_pcm()` function to generate sine waves in PCM formats:
- Supports 16-bit signed PCM (-32768 to 32767)
- Supports 24-bit signed PCM (-8388608 to 8388607)
- Supports 32-bit signed PCM (-2147483648 to 2147483647)

### 3. Updated Files

#### windows/userspace/virtual_sine_device.c
- Added WAVE_FORMAT_EXTENSIBLE definition
- Added GUID definitions for IEEE Float and PCM SubFormats
- Added format detection helper functions
- Added PCM conversion function
- Updated audio generation logic to use helper functions
- Updated format display to show extensible formats

#### windows/tests/test_loopback_read.c
- Added WAVE_FORMAT_EXTENSIBLE support
- Updated to use `is_format_ieee_float()` helper

#### windows/tests/test_format_handling.c (NEW)
- Comprehensive tests for format detection
- Tests for simple and extensible IEEE Float formats
- Tests for simple and extensible PCM formats
- Tests for float sample generation
- Tests for PCM sample generation (16-bit, 24-bit, 32-bit)

### 4. Technical Details

#### WAVEFORMATEXTENSIBLE Structure

```c
typedef struct {
    WAVEFORMATEX Format;       // Base format
    union {
        WORD wValidBitsPerSample;
        WORD wSamplesPerBlock;
        WORD wReserved;
    } Samples;
    DWORD dwChannelMask;       // Speaker positions
    GUID SubFormat;            // Actual format GUID
} WAVEFORMATEXTENSIBLE;
```

The key is checking the `SubFormat` GUID to determine the actual audio format:
- `KSDATAFORMAT_SUBTYPE_IEEE_FLOAT`: IEEE Float samples
- `KSDATAFORMAT_SUBTYPE_PCM`: PCM samples

#### Format Detection Logic

```c
if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
    WAVEFORMATEXTENSIBLE *pwfex = (WAVEFORMATEXTENSIBLE*)pwfx;
    if (guid_equals(&pwfex->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
        // Handle IEEE Float
    } else if (guid_equals(&pwfex->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM)) {
        // Handle PCM
    }
}
```

## Testing

The new `test_format_handling` test validates:
1. Simple format detection (WAVE_FORMAT_IEEE_FLOAT, WAVE_FORMAT_PCM)
2. Extensible format detection (IEEE Float and PCM SubFormats)
3. Float sample generation
4. PCM sample generation (16-bit, 24-bit, 32-bit)

All tests pass successfully.

## Usage

The virtual_sine_device now works seamlessly with VB-Cable and other virtual audio devices that use WAVEFORMATEXTENSIBLE:

```bash
# Use with VB-Cable
virtual_sine_device.exe -d "CABLE Input" -f 440

# The device will now correctly:
# 1. Detect EXTENSIBLE format
# 2. Check the SubFormat GUID
# 3. Generate appropriate samples (float or PCM)
```

## Security

CodeQL analysis was run on all changes with zero security alerts found.

## Compatibility

- Maintains backward compatibility with simple formats
- No changes to API or command-line interface
- Works with all existing virtual audio device solutions on Windows
