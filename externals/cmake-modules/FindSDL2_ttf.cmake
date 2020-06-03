# Licensed under the ISC license. Please see the LICENSE.md file for details.
# Copyright (c) 2019 Sandro Stikić <https://github.com/opeik>

#[[============================================================================
FindSDL2
---------

Try to find SDL2_ttf.

This module defines the following IMPORTED targets:
- SDL2::TTF — Link against this.

This module defines the following variables:
- SDL2_TTF_FOUND
- SDL2_TTF_VERSION_STRING
- SDL2_TTF_LIBRARIES (deprecated)
- SDL2_TTF_INCLUDE_DIRS (deprecated)
#============================================================================]]

# Ensure SDL2 is installed.
find_package(SDL2 REQUIRED QUIET)

if(NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found")
endif()

# Look for SDL2_ttf.
find_path(SDL2_TTF_INCLUDE_DIR SDL_ttf.h
    PATH_SUFFIXES SDL2 include/SDL2 include
    PATHS ${SDL2_TTF_PATH}
)

if (NOT SDL2_TTF_LIBRARIES)
    # Determine architecture.
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_SDL2_TTF_PATH_SUFFIX lib/x64)
    else()
        set(_SDL2_TTF_PATH_SUFFIX lib/x86)
    endif()

    # Look for the release version of SDL2.
    find_library(SDL2_TTF_LIBRARY_RELEASE
        NAMES SDL2_ttf
        PATH_SUFFIXES lib ${_SDL2_TTF_PATH_SUFFIX}
        PATHS ${SDL2_PATH}
    )

    # Look for the debug version of SDL2.
    find_library(SDL2_TTF_LIBRARY_DEBUG
        NAMES SDL2_ttfd
        PATH_SUFFIXES lib ${_SDL2_TTF_PATH_SUFFIX}
        PATHS ${SDL2_PATH}
    )

    include(SelectLibraryConfigurations)
    select_library_configurations(SDL2_TTF)
endif()

# Find the SDL2_ttf version.
if(SDL2_TTF_INCLUDE_DIR AND EXISTS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h")
    file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_TTF_MAJOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_TTF_MINOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_TTF_PATCHLEVEL[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL_TTF_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_TTF_VERSION_MAJOR "${SDL2_TTF_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_TTF_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_TTF_VERSION_MINOR "${SDL2_TTF_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_TTF_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_TTF_VERSION_PATCH "${SDL2_TTF_VERSION_PATCH_LINE}")
    set(SDL2_TTF_VERSION_STRING ${SDL2_TTF_VERSION_MAJOR}.${SDL2_TTF_VERSION_MINOR}.${SDL2_TTF_VERSION_PATCH})
    unset(SDL2_TTF_VERSION_MAJOR_LINE)
    unset(SDL2_TTF_VERSION_MINOR_LINE)
    unset(SDL2_TTF_VERSION_PATCH_LINE)
    unset(SDL2_TTF_VERSION_MAJOR)
    unset(SDL2_TTF_VERSION_MINOR)
    unset(SDL2_TTF_VERSION_PATCH)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_ttf
    REQUIRED_VARS SDL2_TTF_LIBRARIES SDL2_TTF_INCLUDE_DIR
    VERSION_VAR SDL2_TTF_VERSION_STRING)

if(SDL2_TTF_FOUND)
    set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
    set(SDL2_TTF_INCLUDE_DIR ${SDL2_TTF_INCLUDE_DIR})

    # Define the SDL2_ttf target.
    if(NOT TARGET SDL2::TTF)
        add_library(SDL2::TTF UNKNOWN IMPORTED)
        set_target_properties(SDL2::TTF PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${SDL2_TTF_INCLUDE_DIR})

        if(SDL2_TTF_LIBRARY_RELEASE)
            set_property(TARGET SDL2::TTF APPEND PROPERTY
                IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(SDL2::TTF PROPERTIES
                IMPORTED_LOCATION_RELEASE ${SDL2_TTF_LIBRARY_RELEASE})
        endif()

        if(SDL2_TTF_LIBRARY_DEBUG)
            set_property(TARGET SDL2::TTF APPEND PROPERTY
                IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(SDL2::TTF PROPERTIES
                IMPORTED_LOCATION_DEBUG ${SDL2_TTF_LIBRARY_DEBUG})
        endif()

        if(NOT SDL2_TTF_LIBRARY_RELEASE AND NOT SDL2_TTF_LIBRARY_DEBUG)
            set_property(TARGET SDL2::TTF APPEND PROPERTY
                IMPORTED_LOCATION ${SDL2_TTF_LIBRARY})
        endif()
    endif()
endif()
