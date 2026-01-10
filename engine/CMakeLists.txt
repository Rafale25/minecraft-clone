cmake_minimum_required(VERSION 3.25.0)

set(PROJECT_NAME "myEngine")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)


file(GLOB_RECURSE SRC_C CONFIGURE_DEPENDS ./*.c)
file(GLOB_RECURSE SRC_CPP CONFIGURE_DEPENDS ./*.cpp)


set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../submodules/)

# GLFW #
set(GLFW_DIR ${THIRD_PARTY_DIR}/glfw)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
include_directories(${GLFW_DIR}/include)
add_subdirectory(${GLFW_DIR} ${CMAKE_CURRENT_BINARY_DIR}/glfw)
# -- #

# GLM #
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY ON CACHE BOOL "" FORCE)
set(DGLM_BUILD_INSTALL ON CACHE BOOL "" FORCE)
# add_definitions(-D GLM_ENABLE_EXPERIMENTAL) ## for GLM
add_subdirectory(${THIRD_PARTY_DIR}/glm ${CMAKE_CURRENT_BINARY_DIR}/glm)
# include_directories("submodules/glm/glm")
# -- #


# IMGUI #
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
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

add_library(${PROJECT_NAME} STATIC ${SRC_C} ${SRC_CPP})

target_compile_definitions(${PROJECT_NAME} PUBLIC
    ENGINE_RESSOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    GLFW_INCLUDE_NONE
    GLM_ENABLE_EXPERIMENTAL
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/camera/
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/
        ${CMAKE_CURRENT_SOURCE_DIR}/bounds/
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    glfw
    glm
    imgui
)

IF ((WIN32 AND MSVC))
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN) # Removes a lot of includes from windows.h and less namespace pollution
ENDIF()


IF (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32 # networking
    )

    IF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${PROJECT_NAME} PRIVATE
            -lstdc++exp # std::print, std::format
        )
    ENDIF()
ENDIF()

IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_definitions(${PROJECT_NAME} PUBLIC NO_CHRONO_CURRENT_ZONE)
ENDIF()
