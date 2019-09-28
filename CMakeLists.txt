cmake_minimum_required(VERSION 3.10)
project(net64-coop)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/externals/cmake-modules/")


option(USE_JSON_LIBRARY "Use json library" ON)

include(CompilerWarnings)

# Generate revision information
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REF_SPEC GIT_REV)
git_describe(GIT_DESC --always --long --dirty)
git_branch_name(GIT_BRANCH)

set(GIT_REVISION_FILE "${CMAKE_CURRENT_BINARY_DIR}/src/build_info.cpp")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/build_info.cpp.in" "${GIT_REVISION_FILE}" @ONLY)

include_directories(src externals)

option(BUILD_CORE "Build core library" ON)
option(BUILD_QT_FRONTEND "Build the Qt5 userinterface" ON)
option(INSTALL_MUPEN64PLUS "Wether to include a Mupen64Plus installation with the build" OFF)
set(MUPEN64PLUS_DIR "" CACHE PATH "The Mupen64Plus installation to include")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(BUILD_CORE)
    add_subdirectory(src/core)
endif()

if(BUILD_QT_FRONTEND)
    add_subdirectory(src/qt_gui)
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    set(JSON_MultipleHeaders ON)
    add_subdirectory(externals/json)
endif()


# Installation

if(INSTALL_MUPEN64PLUS)
    install(DIRECTORY ${MUPEN64PLUS_DIR}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/usr/share/mupen64plus)
endif()
