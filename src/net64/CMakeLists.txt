add_library(net64_core STATIC
        ../filesystem.hpp
        memory/memory_handle.cpp
        memory/memory_handle.hpp
        ../common/is_instantiation.hpp
        memory/util.hpp
        memory/conversion.hpp
        ${GIT_REVISION_FILE}
        logging.hpp
        logging.cpp
        ../common/operator_proxy.hpp
        error_codes.hpp
        error_codes.cpp
        ../common/crtp.hpp
        game/msg_queue.cpp
        game/msg_queue.hpp
        emulator/emulator.hpp
        emulator/m64plus.cpp
        emulator/m64plus.hpp
        emulator/shared_library.cpp
        emulator/shared_library.hpp
        emulator/m64plus_core.cpp
        emulator/m64plus_plugin.cpp
        game/client_messages.hpp
        game/game_messages.hpp
        game/message_listener.cpp
        game/message_listener.hpp
        game/game_logger.cpp
        game/game_logger.hpp
        game/net64_header.hpp
        memory/pointer.hpp
        client.cpp
        client.hpp
        server.cpp
        server.hpp
        net/protocol.hpp
        net/errors.cpp
        net/errors.hpp
        net/protocol.cpp)

include(ENet)
include_directories(${ENET_INCLUDES})
target_link_libraries(net64_core enet_lib)

if(UNIX)
    target_link_libraries(net64_core -ldl)
endif()
