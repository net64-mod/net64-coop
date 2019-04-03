set(GIT_REVISION_FILE "${CMAKE_CURRENT_BINARY_DIR}/build_info.cpp")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/build_info.cpp.in" "${GIT_REVISION_FILE}" @ONLY)

include_directories(. ../externals)

add_subdirectory(core)

add_subdirectory(qt_gui)
