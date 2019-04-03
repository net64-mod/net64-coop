add_library(64coop_core STATIC
            emulator/handle_config.hpp
            emulator/emulator_handle.cpp
            emulator/emulator_handle.hpp
            emulator/process_handle.hpp
            emulator/process_handle_base.hpp
            emulator/process_linux.cpp
            emulator/process_linux.hpp
            emulator/process_windows.hpp
            emulator/process_windows.cpp
            memory/memory_handle.cpp
            memory/memory_handle.hpp
            ../common/is_instantiation.hpp
            memory/util.hpp
            memory/conversion.hpp
            ${GIT_REVISION_FILE}
            logging.hpp
            logging.cpp ../common/operator_proxy.hpp error_codes.hpp error_codes.cpp ../common/crtp.hpp memory/mem_ptr.hpp memory/mem_ref.hpp)
