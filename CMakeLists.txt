cmake_minimum_required(VERSION 3.25.0)

set(PROJECT_NAME "Minecraft_Clone")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
project(${PROJECT_NAME})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) // slower compilation, more optimization

# GLFW #
set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/glfw)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include_directories("submodules/glfw/include")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/glfw)
# -- #


# IMGUI #
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
# -- #


# GLM #
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY ON CACHE BOOL "" FORCE)
set(DGLM_BUILD_INSTALL ON CACHE BOOL "" FORCE)

add_definitions(-D GLM_ENABLE_EXPERIMENTAL) ## for GLM
add_subdirectory("./submodules/glm")
# include_directories("submodules/glm/glm")
# -- #


file(GLOB_RECURSE SRC_C CONFIGURE_DEPENDS ./src/*.c)
file(GLOB_RECURSE SRC_CPP CONFIGURE_DEPENDS ./src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_C})

# target_compile_options(${PROJECT_NAME} PRIVATE
# -fsanitize=address
# -fsanitize=leak
# )
# target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
# target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)

# target_compile_definitions(${PROJECT_NAME} PRIVATE RESSOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
target_compile_definitions(${PROJECT_NAME} PRIVATE
    RESSOURCE_PATH="./assets/"
    GLFW_INCLUDE_NONE
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ./include/
        ./src/
        ./src/engine/
        ./src/engine/camera/
        ./src/game/
        ./src/game/renderer
        ./src/game/world/
        ./src/game/scenes/
        ./src/game/network/Client/
        ./src/game/network/Client/Packets
        ./src/utils/
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${./submodules/glm/glm}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    glfw
    glm
)

IF (WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN) # Removes a lot of includes from windows.h and less namespace pollution
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
ENDIF()

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./"
)
