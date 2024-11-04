cmake_minimum_required(VERSION 3.9)

set(PROJECT_ROOT ./)
set(PROJECT_NAME "Minecraft_Clone")

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

# find_package(OpenGL REQUIRED)
# include_directories(${OPENGL_INCLUDE_DIRS})

# find_package(glfw3 REQUIRED)
# include_directories(${GLFW_INCLUDE_DIRS})
# link_libraries(${GLFW_LIBRARY_DIRS})

file(GLOB_RECURSE SRC_C ./src/*.c)
file(GLOB_RECURSE SRC_CPP ./src/*.cpp)


set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/glfw)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include_directories("submodules/glfw/include")

add_subdirectory(${GLFW_DIR})


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
target_include_directories(imgui PUBLIC ${IMGUI_DIR})

# add_executable(${PROJECT_NAME} ${SRC_FILES})
add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_C})
target_compile_options(${PROJECT_NAME} PRIVATE
    -W -Wall -Wextra
    -Wno-unused-variable
    -Wno-unused-parameter
    -Wno-strict-aliasing
    -g3
    # -g
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ./include/
        ./src/
        ./src/engine/
        ./src/engine/camera/
        ./src/game/
        ./src/game/world/
        ./src/game/scenes/
        ./src/game/network/Client/
        ./src/game/network/Client/Packets
        ./src/utils/
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        # ${GLFW_DIR}
)

# target_link_libraries(${PROJECT_NAME} PRIVATE -lGL -lX11 -lpthread -lXrandr -ldl) # -lXi
target_link_libraries(${PROJECT_NAME} PRIVATE imgui glfw)

add_definitions(-D GLM_ENABLE_EXPERIMENTAL) ## for GLM

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./"
)
