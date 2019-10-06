set(ENET_INCLUDES "${PROJECT_SOURCE_DIR}/externals/enet-lib/include")
set(ENET_SOURCE_DIR "${PROJECT_SOURCE_DIR}/externals/enet-lib")

add_library(enet_lib STATIC
        "${ENET_SOURCE_DIR}/callbacks.c"
        "${ENET_SOURCE_DIR}/compress.c"
        "${ENET_SOURCE_DIR}/host.c"
        "${ENET_SOURCE_DIR}/list.c"
        "${ENET_SOURCE_DIR}/packet.c"
        "${ENET_SOURCE_DIR}/peer.c"
        "${ENET_SOURCE_DIR}/protocol.c"
        "${ENET_SOURCE_DIR}/unix.c"
        "${ENET_SOURCE_DIR}/win32.c")

if(WIN32)
    target_link_libraries(enet_lib PUBLIC winmm ws2_32)
endif()

target_include_directories(enet_lib PUBLIC ${ENET_INCLUDES})
add_compile_definitions(HAS_SOCKLEN_T)
