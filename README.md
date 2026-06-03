# PrincessOwliviaCB

PrincessOwliviaCB is a fast arcade platformer built with C++17, SDL2, and SDL2_image.

## Features

- Four-stage progression with a final clear screen
- Splash art, score tracking, and high-score name entry
- Native Linux build and WebAssembly build
- GitHub Pages deployment workflow for browser play

## Play Online

When GitHub Pages is enabled for this repository, the web game is available at:

- https://idealiner.github.io/BirchEngine/

## Build Requirements

### Native Linux

- CMake 3.16+
- C++17 compiler (g++, clang++)
- SDL2 and SDL2_image development packages

Example (Debian/Ubuntu):

```bash
sudo apt install -y build-essential cmake libsdl2-dev libsdl2-image-dev
```

### Web Build

- Emscripten SDK (emsdk)

## Build and Run

### Native

```bash
./run-linux.sh
```

### Web

```bash
source ./emsdk/emsdk_env.sh
emcmake cmake -S BirchEngine -B build-web -DBIRCHENGINE_BUILD_WEB=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-web -j
python3 -m http.server 8091 -d build-web
```

Then open:

- http://127.0.0.1:8091/PrincessOwliviaCB.html

## Packaging

### Portable Linux Bundle

```bash
./package-linux.sh
```

This creates:

- dist/PrincessOwliviaCB-Linux-x86_64.tar.gz

The archive contains an AppDir with bundled runtime libraries and assets.

## Is a Single Self-Contained Desktop Executable Possible?

Yes, with caveats:

- Linux: a true single executable is uncommon for SDL apps because dynamic system compatibility varies. The current AppDir/tar.gz bundle is the safest portable option.
- Linux single-file option: AppImage can produce one downloadable file while still bundling dependencies.
- Windows: one .exe is possible with static linking or by bundling DLLs beside the executable.

## Release v0.1.0

See:

- CHANGELOG.md
- RELEASE_NOTES_v0.1.0.md

## License

This project is licensed under the MIT License. See LICENSE.
