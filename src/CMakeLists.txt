
set(SOURCES
    main.cpp
    app/NavOneApp.cpp
    app/services/ServiceManager.cpp
    core/ThreadPool.cpp
    simulator/BaseSimulator.cpp
    simulator/GpsSimulator.cpp
    simulator/WindSimulator.cpp
    simulator/WaterSimulator.cpp
    simulator/AisSimulator.cpp
    parsers/NmeaParser.cpp
    utils/SerialPortUtils.cpp
    utils/ConfigManager.cpp
    gui/MainWindow.cpp
    gui/windows/NmeaMonitorWindow.cpp
    gui/windows/DashboardWindow.cpp
    gui/windows/CommunicationSettingsWindow.cpp
    network/UdpService.cpp
    network/UdpSender.cpp
    network/SerialService.cpp
    app/PluginManager.cpp
)

set(HEADERS
    app/NavOneApp.hpp
    app/DataSourceConfig.hpp
    app/services/ServiceManager.hpp
    app/PluginManager.hpp
    core/ThreadPool.hpp
    core/NavData.hpp
    core/MessageBus.hpp
    simulator/ISimulator.hpp
    simulator/BaseSimulator.hpp
    simulator/SimulatorDecorator.hpp
    simulator/GpsSimulator.hpp
    simulator/WindSimulator.hpp
    simulator/AisSimulator.hpp
    simulator/WaterSimulator.hpp
    simulator/SimulatorConfig.hpp
    parsers/NmeaParser.hpp
    utils/SerialPortUtils.hpp
    utils/ConfigManager.hpp
    gui/MainWindow.hpp
    gui/windows/NmeaMonitorWindow.hpp
    gui/windows/DashboardWindow.hpp
    gui/windows/CommunicationSettingsWindow.hpp
    gui/windows/DisplaySettingsWindow.hpp
    gui/windows/SimulatorWindow.hpp
    network/UdpService.hpp
    network/UdpSender.hpp
    network/SerialService.hpp
    plugin_api/IPlugin.hpp
)

add_executable(NavOne ${SOURCES} ${HEADERS})

# --- Plugins ---

# GPS Plugin
add_library(GpsPlugin SHARED plugins/gps/GpsPlugin.cpp)
target_include_directories(GpsPlugin PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
)
# Link ImGui to plugin (static linking, so it has its own copy of functions, but shares context via pointer)
target_sources(GpsPlugin PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

# GPS Big Plugin
add_library(GpsBigPlugin SHARED plugins/gps_big/GpsBigPlugin.cpp)
target_include_directories(GpsBigPlugin PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
)
target_sources(GpsBigPlugin PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

# Wind Plugin
add_library(WindPlugin SHARED plugins/wind/WindPlugin.cpp)
target_include_directories(WindPlugin PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
)
target_sources(WindPlugin PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

# Water Plugin
add_library(WaterPlugin SHARED plugins/water/WaterPlugin.cpp)
target_include_directories(WaterPlugin PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
)
target_sources(WaterPlugin PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

# Ensure output directory is the same as executable so it can find it easily
# set_target_properties(GpsPlugin PROPERTIES 
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/plugins
# )

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

# --- Installation ---

# Install Executable
install(TARGETS NavOne
    RUNTIME DESTINATION bin
)

# Install Plugins
install(TARGETS GpsPlugin GpsBigPlugin WindPlugin WaterPlugin
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)
