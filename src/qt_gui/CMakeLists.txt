SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(net64_qt_gui
                resources.qrc
                main.cpp
                m64p_settings_window.cpp
                m64p_settings_window.hpp
                app_settings.hpp
                app_settings.cpp
                multiplayer_settings_window.cpp
                multiplayer_settings_window.hpp
                main_window.cpp
                main_window.hpp
                vcdiff_patcher.cpp
                vcdiff_patcher.hpp
                patch_dialog.hpp
                patch_dialog.cpp
                net64_thread.hpp
                net64_thread.cpp
                emulatorsettings.cpp
                emulatorsettings.hpp
                emulator_audio_configuration.hpp
                emulator_audio_configuration.cpp
                emulator_video_configuration.hpp
                emulator_video_configuration.cpp
                emulator_controller_configuration.cpp
                emulator_controller_configuration.hpp
                ../common/sdl_event_handler.cpp
                ../common/sdl_event_handler.hpp
                sdl_bind_button.cpp
                sdl_bind_button.hpp
        )

target_compile_options(net64_qt_gui PRIVATE ${ADDITIONAL_CXXFLAGS})

target_link_libraries(net64_qt_gui net64_core)

# Qt libs
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
target_link_libraries(net64_qt_gui Qt5::Widgets)
include_directories(${Qt5_Widgtes_INCLUDES})

# json library
include(nlohmann_json)
target_include_directories(net64_qt_gui PRIVATE "${JSON_INCLUDES}")

# vcdiff library
target_link_libraries(net64_qt_gui vcddec vcdenc)
target_include_directories(net64_qt_gui PRIVATE "${PROJECT_SOURCE_DIR}/externals/open-vcdiff/src")

include(SDL2_finder)
requires_sdl2()
target_link_libraries(net64_qt_gui SDL2::SDL2)

if(UNIX)
    target_link_libraries(net64_qt_gui -lpthread -lstdc++fs)
endif()

set_target_properties(net64_qt_gui PROPERTIES RUNTIME_OUTPUT_NAME "net64")

if(WIN32)
    # Include MSVC libs
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION usr/bin)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)
    # Run windeployqt
    get_target_property(MOC_EXECUTABLE_LOCATION Qt5::moc IMPORTED_LOCATION)
    get_filename_component(QT_BINARY_DIRECTORY "${MOC_EXECUTABLE_LOCATION}" DIRECTORY)
    find_program(WINDEPLOYQT_EXE windeployqt HINTS "${QT_BINARY_DIRECTORY}")

    add_custom_command(TARGET net64_qt_gui POST_BUILD
        COMMAND "${WINDEPLOYQT_EXE}" --no-translations --no-compiler-runtime "$<TARGET_FILE:net64_qt_gui>"
        WORKING_DIRECTORY "${QT_BINARY_DIRECTORY}"
    )
endif()


# Installation

# We're copying the whole cmake binary dir into /usr/bin to bundle the Qt environment on Windows.
# Unfortunately this does not make the program executable.
# As such we install the executable separately as target and ignore it when copying the directory later.

get_target_property(QT_GUI_EXECUTABLE_NAME_BASE net64_qt_gui RUNTIME_OUTPUT_NAME)

# Install net64 executable
install(TARGETS net64_qt_gui
        DESTINATION ${CMAKE_INSTALL_PREFIX}/usr/bin
        RENAME "net64")

# Install everything else in the runtime output dir
if(CMAKE_CONFIGURATION_TYPES)
    install(DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
            DESTINATION ${CMAKE_INSTALL_PREFIX}/usr/bin
            PATTERN "net64*" EXCLUDE)
else()
    install(DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
            DESTINATION ${CMAKE_INSTALL_PREFIX}/usr/bin
            PATTERN "${QT_GUI_EXECUTABLE_NAME_BASE}*" EXCLUDE)
endif()
