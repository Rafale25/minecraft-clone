# Minecraft-clone
![minecraft_01](https://github.com/user-attachments/assets/f7678698-494a-4145-8fef-843b9e357d24)
![minecraft_02](https://github.com/user-attachments/assets/f1ae1527-ca33-4218-a70b-4d029efd5950)
![minecraft_03](https://github.com/user-attachments/assets/9169c142-90ef-4fbe-8b11-d8829edd5dfe)

## Installing
    git clone https://github.com/Rafale25/minecraft-clone
    git submodule update --init --recursive

# How to build

## External dependencies
* C++ Compiler - C++23 standard (MSVC, GCC, Clang)
* CMake v3.25+ - https://cmake.org/
* Ninja generator - https://ninja-build.org/

#### Linux - Debian/Ubuntu - Install required tools & libraries
    sudo apt install build-essential git ninja-build cmake
    sudo apt install libxrandr-dev libx11-dev libxkbcommon-dev libwayland-dev libxinerama-dev libxcursor-dev libxi-dev mesa-common-dev

### Building the project (Windows & Linux)
    mkdir -p build
    cmake --preset <preset-of-your-choice>
    cmake --build build -j

### Presets
    gcc-debug     - GCC Debug
    gcc-release   - GCC Release
    clang-debug   - Clang Debug
    clang-release - Clang Release
    windows"      - MSVC
