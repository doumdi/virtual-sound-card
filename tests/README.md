# Virtual Sound Card Test Suite

This directory contains the test suite for the Virtual Sound Card project.

## Test Overview

### test_sine_generator
Tests the sine wave generator functionality including:
- Initialization with frequency, sample rate, and amplitude
- 32-bit float sample generation
- 16-bit integer sample generation
- 32-bit integer sample generation
- Frequency and amplitude changes
- Phase reset functionality

### test_api_init
Tests basic API initialization and version management:
- Library version retrieval
- Initialization and cleanup
- Double initialization handling
- Device listing (stub implementation)

### test_sine_wave_file
Generates a test WAV file containing a sine wave:
- Frequency: 440 Hz (A4 note)
- Duration: 2 seconds
- Sample rate: 48000 Hz
- Format: 16-bit PCM mono
- Output: `test_sine_440hz.wav`

This test is useful for manual verification of audio output. You can play the generated file to verify the sine wave generator works correctly.

## Running Tests

### Run All Tests

```bash
cd build
ctest --output-on-failure
```

### Run Individual Tests

```bash
cd build/tests

# Test sine wave generator
./test_sine_generator

# Test API initialization
./test_api_init

# Generate WAV file
./test_sine_wave_file
```

### Verify Generated Audio

After running `test_sine_wave_file`, you can play the generated WAV file:

**Linux:**
```bash
aplay test_sine_440hz.wav
```

**macOS:**
```bash
afplay test_sine_440hz.wav
```

**Windows:**
```cmd
start test_sine_440hz.wav
```

You should hear a 440 Hz sine wave (A4 musical note) for 2 seconds.

## Test Results

Expected output for successful test run:

```
Test project /path/to/build
    Start 1: test_sine_generator
1/3 Test #1: test_sine_generator ..............   Passed    0.00 sec
    Start 2: test_api_init
2/3 Test #2: test_api_init ....................   Passed    0.00 sec
    Start 3: test_sine_wave_file
3/3 Test #3: test_sine_wave_file ..............   Passed    0.00 sec

100% tests passed, 0 tests failed out of 3
```

## Adding New Tests

To add a new test:

1. Create a new test file in this directory (e.g., `test_new_feature.c`)
2. Add the test to `CMakeLists.txt`:
   ```cmake
   add_executable(test_new_feature test_new_feature.c)
   target_link_libraries(test_new_feature vcard_common)
   add_test(NAME test_new_feature COMMAND test_new_feature)
   ```
3. Rebuild and run tests

## Test Conventions

- Tests should return 0 on success, non-zero on failure
- Use descriptive output messages
- Print "PASS" or "FAIL" for each test case
- Include a summary at the end
