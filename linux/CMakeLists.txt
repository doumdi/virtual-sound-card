# Linux Virtual Sound Card Implementation
# Uses ALSA loopback (snd-aloop) kernel module

message(STATUS "Configuring Linux implementation with ALSA")

# Find ALSA library
find_package(ALSA)

if(ALSA_FOUND)
    message(STATUS "ALSA found: ${ALSA_VERSION}")
    
    # Sine wave generator application
    add_executable(sine_generator_app userspace/sine_generator_app.c)
    target_link_libraries(sine_generator_app ${ALSA_LIBRARIES} m)
    target_include_directories(sine_generator_app PRIVATE ${ALSA_INCLUDE_DIRS})
    
    # Loopback read test
    add_executable(test_loopback_read tests/test_loopback_read.c)
    target_link_libraries(test_loopback_read ${ALSA_LIBRARIES} m)
    target_include_directories(test_loopback_read PRIVATE ${ALSA_INCLUDE_DIRS})
    
    message(STATUS "Linux ALSA programs configured")
    message(STATUS "  - sine_generator_app: Generates sine wave to loopback device")
    message(STATUS "  - test_loopback_read: Reads and verifies audio from loopback")
    message(STATUS "")
    message(STATUS "To use: sudo modprobe snd-aloop")
else()
    message(WARNING "ALSA not found - Linux implementation will not be built")
    message(WARNING "Install ALSA development libraries:")
    message(WARNING "  Ubuntu/Debian: sudo apt-get install libasound2-dev")
    message(WARNING "  Fedora/RHEL: sudo dnf install alsa-lib-devel")
endif()
