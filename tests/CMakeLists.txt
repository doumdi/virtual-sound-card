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

# JACK connection test (if JACK is available)
if(BUILD_JACK AND JACK_FOUND)
    add_executable(test_jack_connection test_jack_connection.c)
    target_link_libraries(test_jack_connection ${JACK_LIBRARIES})
    target_include_directories(test_jack_connection PRIVATE ${JACK_INCLUDE_DIRS})
    add_test(NAME test_jack_connection COMMAND test_jack_connection)
    message(STATUS "JACK test configured: test_jack_connection")
endif()

message(STATUS "Tests configured")
