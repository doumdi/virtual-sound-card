# GitHub Actions Workflows

This directory contains the CI/CD workflows for the Virtual Sound Card project.

## Workflows

### ci.yml
Main continuous integration workflow that runs on every push and pull request.

**Features:**
- Builds on Linux (Ubuntu), macOS, and Windows
- Runs all tests with `ctest`
- Uploads test artifacts (including generated WAV files)
- Provides a build summary across all platforms

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` or `develop` branches
- Manual workflow dispatch

### build-matrix.yml
Extended build matrix for testing different compilers and configurations.

**Features:**
- Tests with GCC and Clang on Linux
- Tests with Clang on macOS
- Tests with MSVC on Windows
- Comprehensive compatibility testing

**Triggers:**
- Weekly schedule (Mondays at 00:00 UTC)
- Manual workflow dispatch

## Build Status

Add this badge to your README.md to show the build status:

```markdown
[![CI](https://github.com/doumdi/virtual-sound-card/actions/workflows/ci.yml/badge.svg)](https://github.com/doumdi/virtual-sound-card/actions/workflows/ci.yml)
```

## Local Testing

Before pushing, you can test builds locally:

```bash
# Linux
mkdir build && cd build
cmake -DBUILD_LINUX=ON -DBUILD_TESTS=ON ..
cmake --build .
ctest --output-on-failure

# macOS
mkdir build && cd build
cmake -DBUILD_MACOS=ON -DBUILD_TESTS=ON ..
cmake --build .
ctest --output-on-failure

# Windows (in Developer Command Prompt)
mkdir build && cd build
cmake -DBUILD_WINDOWS=ON -DBUILD_TESTS=ON ..
cmake --build . --config Release
ctest -C Release --output-on-failure
```

## Troubleshooting

### Linux builds fail
- Ensure ALSA development libraries are installed: `sudo apt-get install libasound2-dev`

### macOS builds fail
- CoreAudio framework should be available by default
- Ensure Xcode command line tools are installed: `xcode-select --install`

### Windows builds fail
- Ensure Visual Studio with C++ tools is installed
- WASAPI is part of Windows SDK and should be available by default

## Maintenance

- Workflows use `actions/checkout@v4` and `actions/upload-artifact@v4`
- Update action versions periodically
- Monitor deprecation warnings in GitHub Actions logs
