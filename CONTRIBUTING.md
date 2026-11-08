# Contributing to NXF

NXF is developed by **Hirusha Nethsara** and the **NUX Audio Team**,
Sri Lanka 🇱🇰. Contributions are welcome under the rules below —
they exist to keep the framework original, real-time-safe and
maintainable.

## Ground rules

1. **Original code only.** Do not copy or adapt JUCE (or any other
   framework's) source code, headers or API shape. Design NXF
   interfaces from NXF's own requirements.
2. **C++20, CMake.** Every target must configure with the presets in
   `CMakePresets.json` and build warning-clean with `nxf_set_warnings`.
3. **Real-time rules are law.** Anything reachable from an audio
   callback follows `docs/REALTIME_AUDIO_RULES.md` — no allocation,
   no blocking locks, no I/O. If a change cannot prove that, it does
   not merge.
4. **Dependencies need a license entry first.** Add the dependency,
   its version and its license to `docs/DEPENDENCIES_AND_LICENSING.md`
   in the same pull request that introduces it — or earlier.
5. **Tests before claims.** Executable behaviour ships with tests.
   A test that was not run did not pass.

## Workflow

- Work on a feature branch; never commit directly to `main`.
- One logical change per pull request.
- Run configure + build + `ctest` locally and say so in the PR.
- Update the docs in the same PR when behaviour or boundaries change.
  In particular: `docs/ROADMAP.md` phase status must stay honest —
  "planned" until it is implemented *and* tested.

## Style

- Namespaces: `nxf`, then per module (`nxf::core`, `nxf::audio`, …).
- Public headers live under `include/nxf/<module>/`; implementation
  details stay in `src/`. See `docs/API_DESIGN.md`.
- Document thread-safety on every public API that crosses a thread.

## Copyright

By contributing you agree your contribution is licensed under the
project's MIT License. Copyright: © 2026 Hirusha Nethsara / NUX Audio Team.
