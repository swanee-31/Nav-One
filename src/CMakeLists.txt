
set(SOURCES
    main.cpp
    app/NavOneApp.cpp
    app/services/ServiceManager.cpp
    core/ThreadPool.cpp
    parsers/NmeaParser.cpp
    utils/SerialPortUtils.cpp
    utils/ConfigManager.cpp
    gui/MainWindow.cpp
    gui/windows/NmeaMonitorWindow.cpp
    gui/windows/DashboardWindow.cpp
    gui/windows/CommunicationSettingsWindow.cpp
    network/UdpService.cpp
    network/SerialService.cpp
)

set(HEADERS
    app/NavOneApp.hpp
    app/DataSourceConfig.hpp
    app/services/ServiceManager.hpp
    core/ThreadPool.hpp
    core/NavData.hpp
    core/MessageBus.hpp
    parsers/NmeaParser.hpp
    utils/SerialPortUtils.hpp
    utils/ConfigManager.hpp
    gui/MainWindow.hpp
    gui/windows/NmeaMonitorWindow.hpp
    gui/windows/DashboardWindow.hpp
    gui/windows/CommunicationSettingsWindow.hpp
    network/UdpService.hpp
    network/SerialService.hpp
)

add_executable(NavOne ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(NavOne PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${asio_SOURCE_DIR}/asio/include
)

# Link libraries
target_link_libraries(NavOne PRIVATE glfw tinyxml2)

# Platform specific linking
if(WIN32)
    target_link_libraries(NavOne PRIVATE opengl32 ws2_32 Advapi32) # ws2_32 for Asio, Advapi32 for Registry
elseif(UNIX AND NOT APPLE)
    target_link_libraries(NavOne PRIVATE GL pthread)
endif()

# Add ImGui sources (core + backends)
target_sources(NavOne PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
