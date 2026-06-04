#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/PrincessOwliviaCB"
BUILD_DIR="$PROJECT_DIR/build"

cmake -S "$PROJECT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -j

cd "$PROJECT_DIR"
"$BUILD_DIR/PrincessOwliviaCB"
