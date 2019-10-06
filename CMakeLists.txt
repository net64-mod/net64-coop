cmake_minimum_required(VERSION 3.10)
project(net64-coop)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/externals/cmake-modules/")


add_compile_definitions(NOMINMAX)
include(CompilerWarnings)

# Generate revision information
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REF_SPEC GIT_REV)
git_describe(GIT_DESC --always --long --dirty)
git_branch_name(GIT_BRANCH)

set(GIT_REVISION_FILE "${CMAKE_CURRENT_BINARY_DIR}/src/build_info.cpp")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/build_info.cpp.in" "${GIT_REVISION_FILE}" @ONLY)

include_directories(src externals externals/mem64/include)

option(BUILD_CORE "Build core library" ON)
option(BUILD_QT_FRONTEND "Build the Qt5 userinterface" ON)
option(INSTALL_MUPEN64PLUS "Wether to include a Mupen64Plus installation with the build" OFF)

if(WIN32)
    set(MUPEN64PLUS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/mupen-builds/win32" CACHE PATH "The Mupen64Plus installation to include")
elseif(UNIX)
    set(MUPEN64PLUS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/mupen-builds/linux-amd64" CACHE PATH "The Mupen64Plus installation to include")
else()
    set(MUPEN64PLUS_DIR "MUPEN64PLUS_DIR-NOT-SPECIFIED" CACHE PATH "The Mupen64Plus installation to include")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(BUILD_CORE)
    add_subdirectory(src/net64)
endif()

if(BUILD_QT_FRONTEND)
    add_subdirectory(src/qt_gui)
    # Make the Qt GUI the startup project for Visual Studio
    if(MSVC_IDE)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT net64_qt_gui)
    endif()
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    set(JSON_MultipleHeaders ON)
    add_subdirectory(externals/json EXCLUDE_FROM_ALL)
endif()


# Installation

if(INSTALL_MUPEN64PLUS)
    install(DIRECTORY "${MUPEN64PLUS_DIR}/"
            DESTINATION ${CMAKE_INSTALL_PREFIX}/usr/share/mupen64plus)
endif()
