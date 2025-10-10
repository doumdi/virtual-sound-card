# Contributing to Virtual Sound Card

Thank you for your interest in contributing to the Virtual Sound Card project! This document provides guidelines and information for contributors.

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help create a welcoming environment for all contributors

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/yourusername/virtual-sound-card.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Test thoroughly on relevant platforms
6. Commit with clear messages
7. Push to your fork
8. Open a Pull Request

## Development Setup

### Linux Development

```bash
# Install dependencies
sudo apt-get install build-essential linux-headers-$(uname -r) alsa-utils libasound2-dev

# Build
cd linux
make
```

### Windows Development

- Install Visual Studio 2019+
- Install Windows SDK and WDK
- Open solution in Visual Studio
- Build for x64/ARM64

### macOS Development

```bash
# Install Xcode
xcode-select --install

# Build
cd macos
xcodebuild -project VirtualSoundCard.xcodeproj -scheme VirtualSoundCard
```

## Coding Standards

### General Guidelines

- Write clean, readable, and maintainable code
- Follow platform-specific conventions (see below)
- Add comments for complex logic
- Keep functions focused and small
- Avoid code duplication

### Platform-Specific Standards

#### Linux (C)

- Follow Linux kernel coding style for kernel modules
- Use `checkpatch.pl` for kernel code
- 8-space tabs for indentation (kernel code)
- Keep line length under 80 characters when possible

```c
// Example
static int vcard_probe(struct platform_device *pdev)
{
    struct vcard_data *vcard;
    int ret;
    
    vcard = devm_kzalloc(&pdev->dev, sizeof(*vcard), GFP_KERNEL);
    if (!vcard)
        return -ENOMEM;
    
    // ... implementation
}
```

#### Windows (C/C++)

- Follow Microsoft coding conventions
- Use PascalCase for public functions
- Use camelCase for local variables
- 4-space indentation
- Add SAL annotations for function parameters

```c
// Example
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status;
    
    // ... implementation
    
    return status;
}
```

#### macOS (C/Objective-C/Swift)

- Follow Apple's Swift or Objective-C style guide
- Use 4-space indentation
- camelCase for variables and functions
- PascalCase for types

```swift
// Example
func configureAudioDevice(channels: Int, sampleRate: Double) -> Bool {
    guard channels > 0 && channels <= 32 else {
        return false
    }
    
    // ... implementation
}
```

## Testing

### Required Testing

- All code changes must include appropriate tests
- Test on the target platform before submitting PR
- Include both unit tests and integration tests where applicable
- Document test procedures in PR description

### Platform Testing

- **Linux**: Test with multiple distributions (Ubuntu, Fedora, Arch)
- **Windows**: Test on Windows 10 and 11
- **macOS**: Test on latest and previous macOS versions

## Commit Messages

Use clear and descriptive commit messages:

```
[Platform] Brief description (50 chars or less)

More detailed explanation if needed. Wrap at 72 characters.
Explain what changes were made and why.

- Bullet points are okay
- Reference issues: Fixes #123
```

Examples:
```
[Linux] Add support for 96kHz sample rate

[Windows] Fix buffer overflow in WASAPI endpoint

[macOS] Implement DriverKit audio driver for Big Sur+

[Common] Add MIDI channel configuration API
```

## Pull Request Process

1. **Update Documentation**: Update relevant README files
2. **Add Tests**: Include tests for new features
3. **Test Thoroughly**: Test on target platform(s)
4. **Update Changelog**: Add entry to CHANGELOG.md (if exists)
5. **Request Review**: Tag appropriate reviewers
6. **Address Feedback**: Respond to review comments promptly

### PR Title Format

```
[Platform] Brief description of changes
```

### PR Description Template

```markdown
## Description
Brief description of what this PR does

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Performance improvement
- [ ] Code refactoring

## Platforms Affected
- [ ] Linux
- [ ] Windows
- [ ] macOS
- [ ] Common/All

## Testing
- [ ] Tested on [Platform/OS version]
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Manual testing completed

## Related Issues
Fixes #(issue number)

## Screenshots (if applicable)
```

## Bug Reports

When reporting bugs, include:

- Platform and OS version
- Driver version
- Steps to reproduce
- Expected behavior
- Actual behavior
- Error messages/logs
- System configuration (channels, sample rate, etc.)

## Feature Requests

When requesting features:

- Describe the use case
- Explain why this feature is valuable
- Provide examples of expected behavior
- Consider implementation complexity
- Note platform-specific requirements

## Documentation

- Update documentation for any API changes
- Add inline comments for complex logic
- Update README files for new features
- Include code examples where helpful

## Security

- Never commit credentials, keys, or certificates
- Report security vulnerabilities privately to maintainers
- Follow platform security best practices
- Use secure coding practices

## Questions?

- Open a Discussion on GitHub
- Check existing Issues and PRs
- Review documentation in `docs/`

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (to be determined).

---

Thank you for contributing to Virtual Sound Card! 🎵
