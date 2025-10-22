# Windows Virtual Sound Card Implementation
# Uses WASAPI for user-mode audio applications

message(STATUS "Configuring Windows implementation with WASAPI")

if(WIN32)
    message(STATUS "Building Windows WASAPI programs")
    
    # Sine wave generator application
    add_executable(sine_generator_app userspace/sine_generator_app.c)
    target_link_libraries(sine_generator_app ole32 uuid)
    
    # Virtual sine wave device (continuous, device-specific)
    add_executable(virtual_sine_device userspace/virtual_sine_device.c)
    target_link_libraries(virtual_sine_device ole32 uuid)
    
    # Loopback read test
    add_executable(test_loopback_read tests/test_loopback_read.c)
    target_link_libraries(test_loopback_read ole32 uuid)
    
    # Format handling test
    add_executable(test_format_handling tests/test_format_handling.c)
    target_link_libraries(test_format_handling ole32 uuid)
    add_test(NAME windows_test_format_handling COMMAND test_format_handling)
    
    message(STATUS "Windows WASAPI programs configured")
    message(STATUS "  - sine_generator_app: Generates sine wave to default audio output")
    message(STATUS "  - virtual_sine_device: Continuous sine wave to specific device (e.g., VB-Cable)")
    message(STATUS "  - test_loopback_read: Reads and verifies audio from default capture device")
    message(STATUS "  - test_format_handling: Tests format detection and conversion logic")
    message(STATUS "")
    message(STATUS "Note: For virtual loopback, install a virtual audio cable driver")
    message(STATUS "      (e.g., VB-Cable from https://vb-audio.com/Cable/)")
    
    # Find JACK2 library (optional)
    find_path(JACK_INCLUDE_DIR jack/jack.h)
    find_library(JACK_LIBRARY NAMES libjack64 libjack jack)
    
    if(JACK_INCLUDE_DIR AND JACK_LIBRARY)
        message(STATUS "JACK2 found on Windows")
        
        # JACK2 sine wave generator
        add_executable(jack_sine_generator userspace/jack_sine_generator.c)
        target_include_directories(jack_sine_generator PRIVATE ${JACK_INCLUDE_DIR})
        target_link_libraries(jack_sine_generator ${JACK_LIBRARY})
        
        message(STATUS "Windows JACK2 programs configured")
        message(STATUS "  - jack_sine_generator: Generates sine wave via JACK2")
        message(STATUS "")
        message(STATUS "To use: Install JACK2 from https://jackaudio.org/")
    else()
        message(STATUS "JACK2 not found - JACK programs will not be built")
        message(STATUS "Install JACK2 from https://jackaudio.org/ (optional)")
    endif()
else()
    message(STATUS "Not building on Windows - creating stub programs")
    
    # Create stub programs that inform users to build on Windows
    add_executable(sine_generator_app userspace/sine_generator_app.c)
    add_executable(virtual_sine_device userspace/virtual_sine_device.c)
    add_executable(test_loopback_read tests/test_loopback_read.c)
    
    message(STATUS "Stub programs created for cross-platform compatibility")
    message(STATUS "These programs require Windows to run properly")
endif()
