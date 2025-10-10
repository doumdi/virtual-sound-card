# macOS Virtual Sound Card Implementation
# Uses CoreAudio framework

message(STATUS "Configuring macOS implementation with CoreAudio")

if(APPLE)
    message(STATUS "Building macOS CoreAudio programs")
    
    # Sine wave generator application
    add_executable(sine_generator_app userspace/sine_generator_app.c)
    target_link_libraries(sine_generator_app "-framework CoreAudio" "-framework AudioToolbox")
    
    # Loopback read test
    add_executable(test_loopback_read tests/test_loopback_read.c)
    target_link_libraries(test_loopback_read "-framework CoreAudio" "-framework AudioToolbox")
    
    message(STATUS "macOS CoreAudio programs configured")
    message(STATUS "  - sine_generator_app: Generates sine wave to default audio output")
    message(STATUS "  - test_loopback_read: Reads and verifies audio from default input")
    message(STATUS "")
    message(STATUS "For loopback testing, set up a virtual audio device (e.g., BlackHole)")
else()
    message(WARNING "Not building on macOS - skipping CoreAudio implementation")
endif()
