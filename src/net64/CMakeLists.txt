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
        client/msg_queue.cpp
        client/msg_queue.hpp
        emulator/emulator.hpp
        emulator/m64plus.cpp
        emulator/m64plus.hpp
        emulator/m64plus_core.cpp
        emulator/m64plus_plugin.cpp
        client/client_messages.hpp
        client/game_messages.hpp
        client/message_handler.cpp
        client/message_handler.hpp
        client/game_logger.cpp
        client/game_logger.hpp
        client/net64_header.hpp
        memory/pointer.hpp
        client.cpp
        client.hpp
        server.cpp
        server.hpp
        net/protocol.hpp
        net/errors.cpp
        net/errors.hpp
        net/protocol.cpp
        net64.cpp
        net64.hpp
        ../install_dir.cpp
        emulator/m64plus_core.hpp
        emulator/m64plus_plugin.hpp
        emulator/m64plus_config.hpp
        emulator/m64plus_config.cpp
        emulator/m64plus_error.hpp
        emulator/m64plus_error.cpp
        ../common/resource_handle.hpp
        ../common/shared_object.cpp
        ../common/shared_object.hpp
        ../common/double_dispatch.hpp
        ../common/automatic_factory.hpp
        ../common/message_handler.hpp
        ../common/message_interface.hpp
        net/net_message.hpp
        net/message_ids.hpp
        net/messages_client.hpp
        net/messages_server.hpp
        net/messages_shared.hpp
        server/player.hpp
        server/player.cpp
        server/chat_server.hpp
        server/chat_server.cpp
        server/player_manager.hpp
        server/player_manager.cpp
        ../common/id_manager.hpp client/local_player.cpp client/local_player.hpp client/local_player_manager.cpp client/local_player_manager.hpp client/chat_client.cpp client/chat_client.hpp client/remote_player.cpp client/remote_player.hpp client/player_list_manager.cpp client/player_list_manager.hpp net/net_message.cpp)

target_compile_options(net64_core PRIVATE ${ADDITIONAL_CXXFLAGS})

include(ENet)
include_directories(${ENET_INCLUDES})
target_link_libraries(net64_core enet_lib)

include(SDL2_finder)
requires_sdl2()
target_link_libraries(net64_core SDL2::Core)

if(UNIX)
    target_link_libraries(net64_core -ldl)
endif()
