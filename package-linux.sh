#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/BirchEngine"
BUILD_DIR="$PROJECT_DIR/build"
DIST_DIR="$SCRIPT_DIR/dist/BirchEngine-Linux-x86_64"
APP_DIR="$DIST_DIR/AppDir"
LIB_DIR="$APP_DIR/lib"

cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j

rm -rf "$DIST_DIR"
mkdir -p "$APP_DIR/usr/bin" "$APP_DIR/usr/share/BirchEngine" "$LIB_DIR"

cp "$BUILD_DIR/birchengine" "$APP_DIR/usr/bin/"
cp -r "$PROJECT_DIR/assets" "$APP_DIR/usr/share/BirchEngine/"

cat > "$APP_DIR/AppRun" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="$APP_DIR/lib:${LD_LIBRARY_PATH:-}"
cd "$APP_DIR/usr/share/BirchEngine"
exec "$APP_DIR/usr/bin/birchengine" "$@"
EOF
chmod +x "$APP_DIR/AppRun"

cat > "$APP_DIR/BirchEngine.desktop" <<'EOF'
[Desktop Entry]
Type=Application
Name=BirchEngine
Exec=BirchEngine
Icon=BirchEngine
Categories=Game;
EOF

cp "$PROJECT_DIR/assets/cha11.png" "$APP_DIR/BirchEngine.png"

ldd "$BUILD_DIR/birchengine" \
  | awk '/=> \/|\/lib/ { print $(NF-1) }' \
  | while read -r libPath; do
	if [[ -f "$libPath" ]]; then
		cp -f "$libPath" "$LIB_DIR/"
	fi
	done

tar -C "$DIST_DIR" -czf "$SCRIPT_DIR/dist/BirchEngine-Linux-x86_64.tar.gz" AppDir

echo "Created portable bundle at $SCRIPT_DIR/dist/BirchEngine-Linux-x86_64"
echo "Created archive at $SCRIPT_DIR/dist/BirchEngine-Linux-x86_64.tar.gz"