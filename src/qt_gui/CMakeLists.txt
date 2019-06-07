SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(net64_qt_gui
                main.cpp
                mainframe.cpp
                mainframe.hpp
                m64p_settings_window.cpp
                m64p_settings_window.hpp app_settings.hpp app_settings.cpp)

target_link_libraries(net64_qt_gui 64coop_core)

# Qt libs
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
target_link_libraries(net64_qt_gui Qt5::Widgets)
include_directories(${Qt5_Widgtes_INCLUDES})

# json library
target_link_libraries(net64_qt_gui nlohmann_json::nlohmann_json)

if(UNIX)
    target_link_libraries(net64_qt_gui -lpthread -lstdc++fs )
endif()
