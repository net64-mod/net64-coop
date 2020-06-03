# Licensed under the ISC license. Please see the LICENSE.md file for details.
# Copyright (c) 2019 Sandro Stikić <https://github.com/opeik>

#[[============================================================================
FindSDL2
---------

Try to find SDL2_net.

This module defines the following IMPORTED targets:
- SDL2::Net — Link against this.

This module defines the following variables:
- SDL2_NET_FOUND
- SDL2_NET_VERSION_STRING
- SDL2_NET_LIBRARIES (deprecated)
- SDL2_NET_INCLUDE_DIRS (deprecated)
#============================================================================]]

# Ensure SDL2 is installed.
find_package(SDL2 REQUIRED QUIET)

if(NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found")
endif()

# Look for SDL2_net.
find_path(SDL2_NET_INCLUDE_DIR SDL_net.h
    PATH_SUFFIXES SDL2 include/SDL2 include
    PATHS ${SDL2_NET_PATH}
)

if (NOT SDL2_NET_LIBRARIES)
    # Determine architecture.
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_SDL2_NET_PATH_SUFFIX lib/x64)
    else()
        set(_SDL2_NET_PATH_SUFFIX lib/x86)
    endif()

    # Look for the release version of SDL2.
    find_library(SDL2_NET_LIBRARY_RELEASE
        NAMES SDL2_net
        PATH_SUFFIXES lib ${_SDL2_NET_PATH_SUFFIX}
        PATHS ${SDL2_PATH}
    )

    # Look for the debug version of SDL2.
    find_library(SDL2_NET_LIBRARY_DEBUG
        NAMES SDL2_netd
        PATH_SUFFIXES lib ${_SDL2_NET_PATH_SUFFIX}
        PATHS ${SDL2_PATH}
    )

    include(SelectLibraryConfigurations)
    select_library_configurations(SDL2_NET)
endif()

# Find the SDL2_net version.
if(SDL2_TTF_INCLUDE_DIR AND EXISTS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h")
    file(STRINGS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h" SDL2_NET_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_NET_MAJOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h" SDL2_NET_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_NET_MINOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h" SDL2_NET_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_NET_PATCHLEVEL[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL_NET_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_NET_VERSION_MAJOR "${SDL2_NET_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_NET_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_NET_VERSION_MINOR "${SDL2_NET_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_NET_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_NET_VERSION_PATCH "${SDL2_NET_VERSION_PATCH_LINE}")
    set(SDL2_NET_VERSION_STRING ${SDL2_NET_VERSION_MAJOR}.${SDL2_NET_VERSION_MINOR}.${SDL2_NET_VERSION_PATCH})
    unset(SDL2_NET_VERSION_MAJOR_LINE)
    unset(SDL2_NET_VERSION_MINOR_LINE)
    unset(SDL2_NET_VERSION_PATCH_LINE)
    unset(SDL2_NET_VERSION_MAJOR)
    unset(SDL2_NET_VERSION_MINOR)
    unset(SDL2_NET_VERSION_PATCH)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_net
    REQUIRED_VARS SDL2_NET_LIBRARIES SDL2_NET_INCLUDE_DIR
    VERSION_VAR SDL2_NET_VERSION_STRING)

if(SDL2_NET_FOUND)
    set(SDL2_NET_LIBRARIES ${SDL2_NET_LIBRARY})
    set(SDL2_NET_INCLUDE_DIR ${SDL2_NET_INCLUDE_DIR})

    # Define the SDL2_net target.
    if(NOT TARGET SDL2::Net)
        add_library(SDL2::Net UNKNOWN IMPORTED)
        set_target_properties(SDL2::Net PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${SDL2_NET_INCLUDE_DIR})

        if(SDL2_NET_LIBRARY_RELEASE)
            set_property(TARGET SDL2::Net APPEND PROPERTY
                IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(SDL2::Net PROPERTIES
                IMPORTED_LOCATION_RELEASE ${SDL2_NET_LIBRARY_RELEASE})
        endif()

        if(SDL2_NET_LIBRARY_DEBUG)
            set_property(TARGET SDL2::Net APPEND PROPERTY
                IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(SDL2::Net PROPERTIES
                IMPORTED_LOCATION_DEBUG ${SDL2_NET_LIBRARY_DEBUG})
        endif()

        if(NOT SDL2_NET_LIBRARY_RELEASE AND NOT SDL2_NET_LIBRARY_DEBUG)
            set_property(TARGET SDL2::Net APPEND PROPERTY
                IMPORTED_LOCATION ${SDL2_NET_LIBRARY})
        endif()
    endif()
endif()
