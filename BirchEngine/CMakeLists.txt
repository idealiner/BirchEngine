cmake_minimum_required(VERSION 3.16)
project(BirchEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BIRCHENGINE_BUILD_WEB "Build BirchEngine for web with Emscripten" OFF)
set(BIRCHENGINE_GAME_NAME "PrincessOwliviaCB")

set(BIRCHENGINE_SOURCES
    Src/main.cpp
    Src/Game.cpp
    Src/GameObject.cpp
    Src/TextureManager.cpp
)

add_executable(${BIRCHENGINE_GAME_NAME} ${BIRCHENGINE_SOURCES})

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
    target_compile_definitions(${BIRCHENGINE_GAME_NAME} PRIVATE BIRCHENGINE_WEB=1)
    target_compile_options(${BIRCHENGINE_GAME_NAME} PRIVATE
        "SHELL:-sUSE_SDL=2"
        "SHELL:-sUSE_SDL_IMAGE=2"
        "SHELL:-sSDL2_IMAGE_FORMATS=['png']"
    )
    set_target_properties(${BIRCHENGINE_GAME_NAME} PROPERTIES SUFFIX ".html")
    target_link_options(${BIRCHENGINE_GAME_NAME} PRIVATE
        "SHELL:-sUSE_SDL=2"
        "SHELL:-sUSE_SDL_IMAGE=2"
        "SHELL:-s SDL2_IMAGE_FORMATS=['png']"
        "SHELL:--shell-file ${CMAKE_SOURCE_DIR}/web/shell.html"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s ASSERTIONS=1"
    )
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)

    target_link_libraries(${BIRCHENGINE_GAME_NAME}
        PRIVATE
        PkgConfig::SDL2
        PkgConfig::SDL2_IMAGE
    )
endif()
