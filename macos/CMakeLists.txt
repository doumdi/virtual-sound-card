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
    
    # Find JACK2 library (optional)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(JACK jack)
        if(JACK_FOUND)
            message(STATUS "JACK2 found: ${JACK_VERSION}")
            
            # JACK2 sine wave generator
            add_executable(jack_sine_generator userspace/jack_sine_generator.c)
            target_link_libraries(jack_sine_generator ${JACK_LIBRARIES})
            target_include_directories(jack_sine_generator PRIVATE ${JACK_INCLUDE_DIRS})
            target_compile_options(jack_sine_generator PRIVATE ${JACK_CFLAGS_OTHER})
            
            message(STATUS "macOS JACK2 programs configured")
            message(STATUS "  - jack_sine_generator: Generates sine wave via JACK2")
            message(STATUS "")
            message(STATUS "To use: brew install jack && start JACK server")
        else()
            message(STATUS "JACK2 not found - JACK programs will not be built")
            message(STATUS "Install JACK2 with: brew install jack (optional)")
        endif()
    endif()
else()
    message(WARNING "Not building on macOS - skipping CoreAudio implementation")
endif()
