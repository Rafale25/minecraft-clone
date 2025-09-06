cmake_minimum_required(VERSION 3.25.0)

set(PROJECT_NAME "Minecraft_Clone")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
project(${PROJECT_NAME})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) // slower compilation, more optimization

file(GLOB_RECURSE SRC_C CONFIGURE_DEPENDS ./src/*.c)
file(GLOB_RECURSE SRC_CPP CONFIGURE_DEPENDS ./src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_C})

# target_compile_options(${PROJECT_NAME} PRIVATE
# -fsanitize=address
# -fsanitize=leak
# )
# target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
# target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ./src/game/
        ./src/game/renderer
        ./src/game/world/
        ./src/game/scenes/
        ./src/game/network/Client/
        ./src/game/network/Client/Packets
)

add_subdirectory(engine)

target_link_libraries(${PROJECT_NAME} PRIVATE
    myEngine
)

target_compile_definitions(myEngine PUBLIC
    RESSOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./"
)
