# Windows Virtual Sound Card - Makefile
# For use with MSVC (cl.exe) or MinGW

# Compiler detection
!IF "$(CC)" == ""
CC = cl
!ENDIF

# Common flags
CFLAGS = /W3 /O2 /D_CRT_SECURE_NO_WARNINGS
LDFLAGS = ole32.lib

# Directories
USERSPACE_DIR = userspace
TESTS_DIR = tests
BUILD_DIR = build

# Targets
SINE_APP = $(BUILD_DIR)\sine_generator_app.exe
LOOPBACK_TEST = $(BUILD_DIR)\test_loopback_read.exe

# Source files
SINE_SRC = $(USERSPACE_DIR)\sine_generator_app.c
LOOPBACK_SRC = $(TESTS_DIR)\test_loopback_read.c

.PHONY: all clean help

all: $(BUILD_DIR) $(SINE_APP) $(LOOPBACK_TEST)
	@echo Build complete!
	@echo Run programs from the $(BUILD_DIR) directory

$(BUILD_DIR):
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

$(SINE_APP): $(SINE_SRC)
	$(CC) $(CFLAGS) /Fe:$@ $** $(LDFLAGS)

$(LOOPBACK_TEST): $(LOOPBACK_SRC)
	$(CC) $(CFLAGS) /Fe:$@ $** $(LDFLAGS)

clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@if exist *.obj del *.obj
	@echo Clean complete!

help:
	@echo Windows Virtual Sound Card - Build Instructions
	@echo.
	@echo Targets:
	@echo   all              - Build all programs (default)
	@echo   clean            - Remove build artifacts
	@echo   help             - Show this help message
	@echo.
	@echo Usage:
	@echo   nmake            - Build using NMAKE (Visual Studio)
	@echo   mingw32-make     - Build using MinGW
	@echo.
	@echo Requirements:
	@echo   - Visual Studio 2019 or later (for cl.exe)
	@echo   - OR MinGW-w64 (for gcc)
	@echo   - Windows SDK
