cmake_minimum_required(VERSION 3.15)
project(VirtualSoundCard VERSION 0.1.0 LANGUAGES C CXX)

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_LINUX "Build Linux implementation" OFF)
option(BUILD_WINDOWS "Build Windows implementation" OFF)
option(BUILD_MACOS "Build macOS implementation" OFF)

# Auto-detect platform if no specific platform is selected
if(NOT BUILD_LINUX AND NOT BUILD_WINDOWS AND NOT BUILD_MACOS)
    if(UNIX AND NOT APPLE)
        set(BUILD_LINUX ON)
    elseif(WIN32)
        set(BUILD_WINDOWS ON)
    elseif(APPLE)
        set(BUILD_MACOS ON)
    endif()
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)

# Common library
add_subdirectory(common)

# Platform-specific implementations
if(BUILD_LINUX)
    message(STATUS "Building Linux implementation")
    add_subdirectory(linux)
endif()

if(BUILD_WINDOWS)
    message(STATUS "Building Windows implementation")
    add_subdirectory(windows)
endif()

if(BUILD_MACOS)
    message(STATUS "Building macOS implementation")
    add_subdirectory(macos)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(FILES common/vcard.h DESTINATION include)

# Print configuration summary
message(STATUS "Virtual Sound Card Build Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Linux: ${BUILD_LINUX}")
message(STATUS "  Build Windows: ${BUILD_WINDOWS}")
message(STATUS "  Build macOS: ${BUILD_MACOS}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
