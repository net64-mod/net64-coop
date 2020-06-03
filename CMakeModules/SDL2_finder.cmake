if(WIN32)
    include(FetchContent)
    FetchContent_Declare(
            sdl2_bin_win32
            URL "https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip")
endif()

function(requires_sdl2)
    find_package(SDL2)
    if(NOT SDL_FOUND)
        if(WIN32)
            # Download SDL2 binaries
            include(FetchContent)
            FetchContent_Declare(
                    sdl2_bin_win32
                    URL "https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip")
            FetchContent_GetProperties(sdl2_bin_win32)
            if(NOT ${sdl2_bin_win32_POPULATED})
                FetchContent_Populate(sdl2_bin_win32)
            endif()
            # Manually set paths
            message("Using downloaded SDL2 win32 binaries")
            set(SDL2_INCLUDE_DIR "${sdl2_bin_win32_SOURCE_DIR}/include")
            set(SDL2_LIBRARY_RELEASE "${sdl2_bin_win32_SOURCE_DIR}/lib/x86/SDL2.lib")
            set(SDL2main_LIBRARIES "${sdl2_bin_win32_SOURCE_DIR}/lib/x86/SDL2main.lib")
        endif()
    endif()
    find_package(SDL2 REQUIRED)
endfunction()
