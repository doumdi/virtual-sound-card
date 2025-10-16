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
    
    message(STATUS "Windows WASAPI programs configured")
    message(STATUS "  - sine_generator_app: Generates sine wave to default audio output")
    message(STATUS "  - virtual_sine_device: Continuous sine wave to specific device (e.g., VB-Cable)")
    message(STATUS "  - test_loopback_read: Reads and verifies audio from default capture device")
    message(STATUS "")
    message(STATUS "Note: For virtual loopback, install a virtual audio cable driver")
    message(STATUS "      (e.g., VB-Cable from https://vb-audio.com/Cable/)")
else()
    message(STATUS "Not building on Windows - creating stub programs")
    
    # Create stub programs that inform users to build on Windows
    add_executable(sine_generator_app userspace/sine_generator_app.c)
    add_executable(virtual_sine_device userspace/virtual_sine_device.c)
    add_executable(test_loopback_read tests/test_loopback_read.c)
    
    message(STATUS "Stub programs created for cross-platform compatibility")
    message(STATUS "These programs require Windows to run properly")
endif()
