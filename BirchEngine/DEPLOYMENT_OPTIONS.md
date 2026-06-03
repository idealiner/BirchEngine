# BirchEngine deployment options

This project is portable C++/SDL2 and can target desktop, web, Android, and iOS without rewriting the core game into Objective-C.

## Recommended order

1. Web demo (fastest feedback loop)
2. Android package (largest testing audience)
3. iOS App Store package (more signing/process overhead)

## Web (Emscripten/WASM)

### Build

```bash
emcmake cmake -S . -B build-web -DBIRCHENGINE_BUILD_WEB=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-web -j
```

Output should include `birchengine.html`, `.js`, `.wasm`, and preloaded assets.

### Host

Use static hosting (itch.io HTML5 project, GitHub Pages, Netlify, Cloudflare Pages).

### Web checklist

- Ensure first user interaction happens early (audio autoplay rules)
- Keep total download size reasonable for first load
- Test keyboard plus touch interaction
- Verify performance on low-power laptops and mobile browsers

## Android (Google Play)

### Practical route

Use SDL2 Android project + Android NDK and link your game sources.

### Android checklist

- Add touch controls (virtual left/right/jump)
- Handle pause/resume lifecycle correctly
- Build arm64-v8a (required), optional x86_64 for testing
- Add app icon, privacy policy, age/content declarations
- Test multiple aspect ratios and notch/safe-area behavior

## iOS (App Store)

### Practical route

Use SDL2 iOS template in Xcode and include the same C++ sources.

### iOS checklist

- Keep C++ gameplay code; do not rewrite to Objective-C
- Add touch controls and orientation handling
- Configure signing, bundle identifier, and provisioning profile
- Add App Store metadata, screenshots, and review notes
- Test on real devices (not simulator only)

## Control abstraction roadmap

Current code already routes input through one input state. Expand this layer to include:

- Multi-touch tracking (left + jump simultaneously)
- On-screen visual controls for mobile
- Optional gamepad mapping

## Suggested release model

- Ship desktop and web in parallel for player validation
- Use web as instant-play funnel and desktop/mobile as premium builds
- Move to stores after controls and lifecycle are polished
