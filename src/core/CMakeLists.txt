add_library(64coop_core STATIC
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
        memory/mem_ptr.hpp
        memory/mem_ref.hpp
        game/msg_queue.cpp
        game/msg_queue.hpp emulator/emulator.cpp emulator/emulator.hpp emulator/m64plus.cpp emulator/m64plus.hpp emulator/shared_library.cpp emulator/shared_library.hpp)
