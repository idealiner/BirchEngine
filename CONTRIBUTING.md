# Contributing

Thanks for your interest in improving PrincessOwliviaCB.

## Development Setup

1. Install build dependencies listed in README.md.
2. Run native build with:

```bash
./run-linux.sh
```

3. For web changes, build with Emscripten and test in a local HTTP server.

## Pull Request Guidelines

- Keep changes focused and small.
- Build and test locally before opening a PR.
- Include a short summary and test steps in the PR description.
- Update changelog/release notes when behavior changes.

## Style

- C++17
- Keep gameplay behavior deterministic and simple.
- Avoid unrelated refactors in feature/fix PRs.
