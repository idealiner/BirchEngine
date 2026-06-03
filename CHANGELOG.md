# Changelog

All notable changes to this project are documented in this file.

## [0.1.0] - 2026-06-03

### Added

- Web build support with Emscripten and SDL2/SDL2_image configuration
- GitHub Pages deployment workflow for browser publishing
- Splash screen with cover art and launcher/window icon integration
- Four-stage completion state with final clear screen
- High-score system with player name entry and XML persistence on native builds
- Improved asset path resolution for more reliable texture loading

### Changed

- Game/project naming updated to PrincessOwliviaCB
- Linux run and packaging scripts updated for PrincessOwliviaCB outputs
- Sprite/font glyph support expanded for UI text coverage

### Fixed

- Web startup/rendering issues caused by unsupported SDL init flags in browser
- Missing sprite loads due to working-directory asset path mismatches
- Final completion screen rendering path and restart handling
