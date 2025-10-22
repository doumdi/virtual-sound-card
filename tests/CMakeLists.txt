# Test suite for Virtual Sound Card

# Test for sine wave generator
add_executable(test_sine_generator test_sine_generator.c)
target_link_libraries(test_sine_generator vcard_common)
add_test(NAME test_sine_generator COMMAND test_sine_generator)

# Test for API initialization
add_executable(test_api_init test_api_init.c)
target_link_libraries(test_api_init vcard_common)
add_test(NAME test_api_init COMMAND test_api_init)

# Test for sine wave file generation
add_executable(test_sine_wave_file test_sine_wave_file.c)
target_link_libraries(test_sine_wave_file vcard_common)
add_test(NAME test_sine_wave_file COMMAND test_sine_wave_file)

# Test for JACK2 availability (if JACK is found)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(JACK jack)
    if(JACK_FOUND)
        add_executable(test_jack_availability test_jack_availability.c)
        target_compile_definitions(test_jack_availability PRIVATE HAVE_JACK)
        target_link_libraries(test_jack_availability ${JACK_LIBRARIES})
        target_include_directories(test_jack_availability PRIVATE ${JACK_INCLUDE_DIRS})
        add_test(NAME test_jack_availability COMMAND test_jack_availability)
        message(STATUS "JACK2 availability test configured")
    else()
        # Build test without JACK support to show it's optional
        add_executable(test_jack_availability test_jack_availability.c)
        add_test(NAME test_jack_availability COMMAND test_jack_availability)
        message(STATUS "JACK2 availability test configured (JACK not found - test will show it's optional)")
    endif()
elseif(WIN32)
    # Windows: Try to find JACK manually
    find_path(JACK_INCLUDE_DIR jack/jack.h)
    find_library(JACK_LIBRARY NAMES libjack64 libjack jack)
    if(JACK_INCLUDE_DIR AND JACK_LIBRARY)
        add_executable(test_jack_availability test_jack_availability.c)
        target_compile_definitions(test_jack_availability PRIVATE HAVE_JACK)
        target_include_directories(test_jack_availability PRIVATE ${JACK_INCLUDE_DIR})
        target_link_libraries(test_jack_availability ${JACK_LIBRARY})
        add_test(NAME test_jack_availability COMMAND test_jack_availability)
        message(STATUS "JACK2 availability test configured (Windows)")
    else()
        # Build test without JACK support
        add_executable(test_jack_availability test_jack_availability.c)
        add_test(NAME test_jack_availability COMMAND test_jack_availability)
        message(STATUS "JACK2 availability test configured (JACK not found - test will show it's optional)")
    endif()
else()
    # Build test without JACK support
    add_executable(test_jack_availability test_jack_availability.c)
    add_test(NAME test_jack_availability COMMAND test_jack_availability)
    message(STATUS "JACK2 availability test configured (pkg-config not found)")
endif()

message(STATUS "Tests configured")
