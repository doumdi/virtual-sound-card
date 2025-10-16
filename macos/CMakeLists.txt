# macOS Virtual Sound Card Implementation
# Uses CoreAudio framework

message(STATUS "Configuring macOS implementation with CoreAudio")

if(APPLE)
    message(STATUS "Building macOS CoreAudio programs")
    
    # Sine wave generator application
    add_executable(sine_generator_app userspace/sine_generator_app.c)
    target_link_libraries(sine_generator_app "-framework CoreAudio" "-framework AudioToolbox")
    
    # Virtual sine wave device (generates sine wave to virtual device)
    add_executable(virtual_sine_device userspace/virtual_sine_device.c)
    target_link_libraries(virtual_sine_device "-framework CoreAudio" "-framework AudioToolbox" "-framework CoreFoundation")
    
    # Loopback read test
    add_executable(test_loopback_read tests/test_loopback_read.c)
    target_link_libraries(test_loopback_read "-framework CoreAudio" "-framework AudioToolbox")
    
    # Virtual sine device configuration test
    add_executable(test_virtual_sine_device tests/test_virtual_sine_device.c)
    target_compile_options(test_virtual_sine_device PRIVATE -Wno-unused-parameter)
    add_test(NAME test_virtual_sine_device COMMAND test_virtual_sine_device)
    
    # Install setup script
    install(PROGRAMS userspace/setup_virtual_device.sh
            DESTINATION bin
            RENAME vcard-setup-macos)
    
    # Install demo script
    install(PROGRAMS demo.sh
            DESTINATION bin
            RENAME vcard-demo-macos)
    
    message(STATUS "macOS CoreAudio programs configured")
    message(STATUS "  - sine_generator_app: Generates sine wave to default audio output")
    message(STATUS "  - virtual_sine_device: Virtual sine wave device for loopback")
    message(STATUS "  - test_loopback_read: Reads and verifies audio from default input")
    message(STATUS "  - setup_virtual_device.sh: Setup helper for virtual audio devices")
    message(STATUS "")
    message(STATUS "For loopback testing, set up a virtual audio device (e.g., BlackHole)")
    message(STATUS "or use the setup_virtual_device.sh script for instructions")
else()
    message(WARNING "Not building on macOS - skipping CoreAudio implementation")
endif()
