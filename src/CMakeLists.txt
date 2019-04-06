set(GIT_REVISION_FILE "${CMAKE_CURRENT_BINARY_DIR}/build_info.cpp")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/build_info.cpp.in" "${GIT_REVISION_FILE}" @ONLY)

include_directories(. ../externals)

option(BUILD_CORE "Build core library" ON)
option(BUILD_QT_FRONTEND "Build the Qt5 userinterface" ON)

if(BUILD_CORE)
    add_subdirectory(core)
endif()

if(BUILD_QT_FRONTEND)
    add_subdirectory(qt_gui)
endif()
