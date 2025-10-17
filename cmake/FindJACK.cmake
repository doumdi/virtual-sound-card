# FindJACK.cmake
# Finds the JACK Audio Connection Kit library
#
# This will define the following variables:
#   JACK_FOUND        - True if JACK is found
#   JACK_INCLUDE_DIRS - Include directories for JACK
#   JACK_LIBRARIES    - Libraries to link against
#   JACK_VERSION      - Version of JACK found
#
# And the following imported target:
#   JACK::JACK

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_JACK QUIET jack)
endif()

# Find the include directory
find_path(JACK_INCLUDE_DIR
    NAMES jack/jack.h
    HINTS
        ${PC_JACK_INCLUDEDIR}
        ${PC_JACK_INCLUDE_DIRS}
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /opt/homebrew/include
        "$ENV{PROGRAMFILES}/JACK2/include"
        "$ENV{PROGRAMFILES}/JACK/include"
        "C:/Program Files/JACK2/include"
        "C:/Program Files/JACK/include"
)

# Find the library
find_library(JACK_LIBRARY
    NAMES jack libjack jack64 libjack64
    HINTS
        ${PC_JACK_LIBDIR}
        ${PC_JACK_LIBRARY_DIRS}
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /opt/homebrew/lib
        "$ENV{PROGRAMFILES}/JACK2/lib"
        "$ENV{PROGRAMFILES}/JACK/lib"
        "C:/Program Files/JACK2/lib"
        "C:/Program Files/JACK/lib"
)

# Extract version from pkg-config if available
if(PC_JACK_VERSION)
    set(JACK_VERSION ${PC_JACK_VERSION})
endif()

# Handle standard args
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JACK
    REQUIRED_VARS
        JACK_LIBRARY
        JACK_INCLUDE_DIR
    VERSION_VAR JACK_VERSION
)

if(JACK_FOUND)
    set(JACK_LIBRARIES ${JACK_LIBRARY})
    set(JACK_INCLUDE_DIRS ${JACK_INCLUDE_DIR})
    
    # Create imported target
    if(NOT TARGET JACK::JACK)
        add_library(JACK::JACK UNKNOWN IMPORTED)
        set_target_properties(JACK::JACK PROPERTIES
            IMPORTED_LOCATION "${JACK_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${JACK_INCLUDE_DIR}"
        )
    endif()
    
    mark_as_advanced(JACK_INCLUDE_DIR JACK_LIBRARY)
endif()
