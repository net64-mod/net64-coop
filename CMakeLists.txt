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

if(BUILD_CORE)
    add_subdirectory(src/core)
endif()

if(BUILD_QT_FRONTEND)
    add_subdirectory(src/qt_gui)
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    add_subdirectory(externals/json)
endif()
