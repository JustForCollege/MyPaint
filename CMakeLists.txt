cmake_minimum_required(VERSION 3.30)
set(PROJECT_NAME mypaint)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(${PROJECT_NAME} C CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../deps")

set(IMGUI_DIR "${THIRDPARTY_DIR}/imgui")
set(RLIMGUI_DIR "${THIRDPARTY_DIR}/rlImGui")
set(RAYLIB_DIR "${THIRDPARTY_DIR}/raylib")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${RLIMGUI_DIR}/rlImGui.cpp
)

include_directories(${IMGUI_DIR} ${RLIMGUI_DIR} "${ENET_DIR}/include")
add_subdirectory("${RAYLIB_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/raylib")

add_executable(${PROJECT_NAME} ${IMGUI_SOURCES} main.cpp paint.cpp)
target_link_libraries(${PROJECT_NAME} raylib)
