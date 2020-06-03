# Licensed under the ISC license. Please see the LICENSE.md file for details.
# Copyright (c) 2019 Sandro Stikić <https://github.com/opeik>

#[[============================================================================
FindSDL2
---------

Try to find SDL2_gfx.

This module defines the following IMPORTED targets:
- SDL2::Gfx — Link against this.

This module defines the following variables:
- SDL2_GFX_FOUND
- SDL2_GFX_VERSION_STRING
- SDL2_GFX_LIBRARIES (deprecated)
- SDL2_GFX_INCLUDE_DIRS (deprecated)
#============================================================================]]

# Ensure SDL2 is installed.
find_package(SDL2 REQUIRED QUIET)

if(NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found")
endif()

# Look for SDL2_gfx.
find_path(SDL2_GFX_INCLUDE_DIR SDL2_gfxPrimitives.h
    PATH_SUFFIXES SDL2 include/SDL2 include
    PATHS ${SDL2_GFX_PATH}
)

if (NOT SDL2_GFX_LIBRARIES)
    # Determine architecture.
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_SDL2_GFX_PATH_SUFFIX lib/x64)
    else()
        set(_SDL2_GFX_PATH_SUFFIX lib/x86)
    endif()

    # Look for the release version of SDL2.
    find_library(SDL2_GFX_LIBRARY_RELEASE
        NAMES SDL2_gfx
        PATH_SUFFIXES lib ${_SDL2_GFX_PATH_SUFFIX}
        PATHS ${SDL2_PATH}
    )

    # Look for the debug version of SDL2.
    find_library(SDL2_GFX_LIBRARY_DEBUG
        NAMES SDL2_gfxd
        PATH_SUFFIXES lib ${_SDL2_GFX_PATH_SUFFIX}
        PATHS ${SDL2_PATH}
    )

    include(SelectLibraryConfigurations)
    select_library_configurations(SDL2_GFX)
endif()

# Find the SDL2_gfx version.
if(SDL2_GFX_INCLUDE_DIR AND EXISTS "${SDL2_GFX_INCLUDE_DIR}/SDL2_gfxPrimitives.h")
    file(STRINGS "${SDL2_GFX_INCLUDE_DIR}/SDL2_gfxPrimitives.h" SDL2_GFX_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL2_GFXPRIMITIVES_MAJOR[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_GFX_INCLUDE_DIR}/SDL2_gfxPrimitives.h" SDL2_GFX_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL2_GFXPRIMITIVES_MINOR[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_GFX_INCLUDE_DIR}/SDL2_gfxPrimitives.h" SDL2_GFX_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL2_GFXPRIMITIVES_MICRO[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL2_GFXPRIMITIVES_MAJOR[ \t]+([0-9]+)$" "\\1" SDL2_GFX_VERSION_MAJOR "${SDL2_GFX_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL2_GFXPRIMITIVES_MINOR[ \t]+([0-9]+)$" "\\1" SDL2_GFX_VERSION_MINOR "${SDL2_GFX_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL2_GFXPRIMITIVES_MICRO[ \t]+([0-9]+)$" "\\1" SDL2_GFX_VERSION_PATCH "${SDL2_GFX_VERSION_PATCH_LINE}")
    set(SDL2_GFX_VERSION_STRING ${SDL2_GFX_VERSION_MAJOR}.${SDL2_GFX_VERSION_MINOR}.${SDL2_GFX_VERSION_PATCH})
    unset(SDL2_GFX_VERSION_MAJOR_LINE)
    unset(SDL2_GFX_VERSION_MINOR_LINE)
    unset(SDL2_GFX_VERSION_PATCH_LINE)
    unset(SDL2_GFX_VERSION_MAJOR)
    unset(SDL2_GFX_VERSION_MINOR)
    unset(SDL2_GFX_VERSION_PATCH)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_gfx
    REQUIRED_VARS SDL2_GFX_LIBRARIES SDL2_GFX_INCLUDE_DIR
    VERSION_VAR SDL2_GFX_VERSION_STRING)

if(SDL2_GFX_FOUND)
    set(SDL2_GFX_LIBRARIES ${SDL2_GFX_LIBRARY})
    set(SDL2_GFX_INCLUDE_DIR ${SDL2_GFX_INCLUDE_DIR})

    # Define the SDL2_gfx target.
    if(NOT TARGET SDL2::Gfx)
        add_library(SDL2::Gfx UNKNOWN IMPORTED)
        set_target_properties(SDL2::Gfx PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${SDL2_GFX_INCLUDE_DIR})

        if(SDL2_GFX_LIBRARY_RELEASE)
            set_property(TARGET SDL2::Gfx APPEND PROPERTY
                IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(SDL2::Gfx PROPERTIES
                IMPORTED_LOCATION_RELEASE ${SDL2_GFX_LIBRARY_RELEASE})
        endif()

        if(SDL2_GFX_LIBRARY_DEBUG)
            set_property(TARGET SDL2::Gfx APPEND PROPERTY
                IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(SDL2::Gfx PROPERTIES
                IMPORTED_LOCATION_DEBUG ${SDL2_GFX_LIBRARY_DEBUG})
        endif()

        if(NOT SDL2_GFX_LIBRARY_RELEASE AND NOT SDL2_GFX_LIBRARY_DEBUG)
            set_property(TARGET SDL2::Gfx APPEND PROPERTY
                IMPORTED_LOCATION ${SDL2_GFX_LIBRARY})
        endif()
    endif()
endif()
