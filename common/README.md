# Common Library

This directory contains shared code and utilities used across all platform implementations.

## Contents

- **vcard.h**: Common API definitions and data structures
- **utils/**: Utility functions (to be implemented)
- **audio/**: Common audio processing code (to be implemented)
- **midi/**: Common MIDI handling code (to be implemented)

## Purpose

The common library provides:
- Cross-platform API definitions
- Shared data structures
- Common utility functions
- Platform-independent audio/MIDI processing logic

## Building

The common library is built as part of each platform-specific driver. Include files from this directory when building Linux, Windows, or macOS implementations.

## API Documentation

See [docs/API.md](../docs/API.md) for detailed API documentation.
