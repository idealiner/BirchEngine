cmake_minimum_required(VERSION 3.16)
project(BirchEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BIRCHENGINE_BUILD_WEB "Build BirchEngine for web with Emscripten" OFF)

set(BIRCHENGINE_SOURCES
    Src/main.cpp
    Src/Game.cpp
    Src/GameObject.cpp
    Src/TextureManager.cpp
)

add_executable(birchengine ${BIRCHENGINE_SOURCES})

set(BIRCHENGINE_IS_WEB OFF)
if (EMSCRIPTEN)
    set(BIRCHENGINE_IS_WEB ON)
endif()

if (BIRCHENGINE_BUILD_WEB)
    set(BIRCHENGINE_IS_WEB ON)
endif()

if (BIRCHENGINE_IS_WEB AND NOT EMSCRIPTEN)
    message(FATAL_ERROR "Web builds require Emscripten toolchain. Use: emcmake cmake -S . -B build-web -DBIRCHENGINE_BUILD_WEB=ON")
endif()

if (BIRCHENGINE_IS_WEB)
    target_compile_definitions(birchengine PRIVATE BIRCHENGINE_WEB=1)
    set_target_properties(birchengine PROPERTIES SUFFIX ".html")
    target_link_options(birchengine PRIVATE
        "SHELL:-s USE_SDL=2"
        "SHELL:-s USE_SDL_IMAGE=2"
        "SHELL:-s SDL2_IMAGE_FORMATS=['png']"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s ASSERTIONS=1"
    )
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)

    target_link_libraries(birchengine
        PRIVATE
        PkgConfig::SDL2
        PkgConfig::SDL2_IMAGE
    )
endif()
