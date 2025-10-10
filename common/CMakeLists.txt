# Common library with shared utilities
add_library(vcard_common STATIC
    vcard_common.c
    sine_generator.c
)

target_include_directories(vcard_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Math library (required for sine wave generation)
if(UNIX)
    target_link_libraries(vcard_common m)
endif()

install(TARGETS vcard_common DESTINATION lib)
