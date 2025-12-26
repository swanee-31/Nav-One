cmake_minimum_required(VERSION 3.16)

project(NavOne VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Get Git Version
find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

if(NOT GIT_VERSION)
    set(GIT_VERSION "unknown")
endif()

add_compile_definitions(NAVONE_GIT_VERSION="${GIT_VERSION}")

# --- Dependencies ---
include(FetchContent)

# 1. GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# 2. Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

# 3. Asio (Standalone - No Boost required)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-28-0
)
FetchContent_MakeAvailable(asio)

# 4. TinyXML2
FetchContent_Declare(
    tinyxml2
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
    GIT_TAG        9.0.0
)
FetchContent_MakeAvailable(tinyxml2)

# --- Sources ---
add_subdirectory(src)

# --- Packaging (CPack) ---
set(CPACK_PACKAGE_NAME "NavOne")
set(CPACK_PACKAGE_VENDOR "Fabrice Meynckens")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NavOne - Navigation Hub Application")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "NavOne")
set(CPACK_PACKAGE_CONTACT "Fabrice Meynckens")

# NSIS specific (Windows Installer)
set(CPACK_NSIS_DISPLAY_NAME "NavOne Navigation Hub")
set(CPACK_NSIS_PACKAGE_NAME "NavOne")
set(CPACK_NSIS_MODIFY_PATH ON)
# Icon for installer (optional, commented out for now)
# set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.ico")
# set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.ico")

# Executable to run after install
set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
set(CPACK_NSIS_MENU_LINKS "bin/NavOne.exe" "NavOne")

# Generators
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
