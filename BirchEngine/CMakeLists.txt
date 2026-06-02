cmake_minimum_required(VERSION 3.16)
project(BirchEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)

add_executable(birchengine
    Src/main.cpp
    Src/Game.cpp
    Src/GameObject.cpp
    Src/TextureManager.cpp
)

target_link_libraries(birchengine
    PRIVATE
    PkgConfig::SDL2
    PkgConfig::SDL2_IMAGE
)
