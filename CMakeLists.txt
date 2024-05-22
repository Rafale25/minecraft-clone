cmake_minimum_required(VERSION 3.9)

set(PROJECT_ROOT ./)
set(PROJECT_NAME "Minecraft_Clone")

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)

# find_package(OpenGL REQUIRED)
# include_directories(${OPENGL_INCLUDE_DIRS})

# find_package(glfw3 REQUIRED)
# include_directories(${GLFW_INCLUDE_DIRS})
# link_libraries(${GLFW_LIBRARY_DIRS})

set(SRC_FILES
    ./src/engine/gl.c
    ./src/engine/stb_image.cpp
    ./src/engine/context.cpp
    ./src/game/chunk.cpp
    ./src/game/texture_manager.cpp
    ./src/main.cpp
)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/imgui)
add_library(imgui STATIC)
target_sources(imgui
    PRIVATE
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui
    PUBLIC
        ${IMGUI_DIR}
)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_compile_options(${PROJECT_NAME} PRIVATE -W) # -Wall -Wextra) # -g3
# target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ./include/
        ./src/
        ./src/engine/
        ./src/game/
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)
target_link_libraries(${PROJECT_NAME} PRIVATE -lglfw -lGL -lX11 -lpthread -lXrandr -ldl) # -lXi
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

add_definitions(-D GLM_ENABLE_EXPERIMENTAL) ## for GLM

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./"
)
