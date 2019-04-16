SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(net64_qt_gui
               main.cpp
               mainframe.cpp
               mainframe.hpp ../common/operator_proxy.hpp ../common/crtp.hpp)

target_link_libraries(net64_qt_gui 64coop_core)

# Qt libs
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
target_link_libraries(net64_qt_gui Qt5::Widgets)
include_directories(${Qt5_Widgtes_INCLUDES})

if(UNIX)
    target_link_libraries(net64_qt_gui -lpthread -lstdc++fs )
endif()
