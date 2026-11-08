# NXF — NUX Framework

An original, modular, **JUCE-independent** C++20 framework for audio
applications and desktop software — built for the NUX family
(NUX DAW, and in future NUX Synth, NUX Sampler and other tools),
but not limited to them.

| | |
|---|---|
| **Developer** | Hirusha Nethsara |
| **Team** | NUX Audio Team |
| **Country** | Sri Lanka 🇱🇰 |
| **Copyright** | © 2026 Hirusha Nethsara / NUX Audio Team |
| **License** | MIT (see `LICENSE`) |
| **Language / build** | C++20 · CMake 3.22+ |
| **First target** | Windows x64 (Linux used for build verification) |

> **Status: Phase 2 — Platform Layer.**
> `nxf_core` provides the framework foundation (errors/results,
> logging, preallocated buffers, an SPSC event queue, time) and
> `nxf_platform` the OS services (filesystem, timing, synchronization,
> system information, with Windows and POSIX backends). Each piece is
> covered by unit tests. The `hello_nxf` example still builds and runs. Everything else in the module
> list below is **planned, not implemented** — `docs/ROADMAP.md`
> tracks each phase honestly. Do not mistake the directory layout
> for finished features.

## Why a separate framework?

NUX DAW currently builds on JUCE. NXF is being developed in its own
repository so the working DAW is never put at risk: NXF modules will
be connected to NUX DAW one at a time, only when each module is
production-ready (Roadmap Phase 13). No JUCE source code, headers or
APIs are copied or adapted — NXF is an original design with its own
namespaces, interfaces and documentation.

## Modules

| Module | Responsibility | Phase 0 status |
|---|---|---|
| `nxf_core` | Foundational types, errors, logging interfaces, memory utilities, event primitives | ✅ Phase 1 implemented (tested) |
| `nxf_platform` | OS abstraction, filesystem, clocks, synchronization, platform services | ✅ Phase 2 implemented (tested) |
| `nxf_audio` | Audio devices, buffers, streams, device abstraction, engine contracts | ⬜ planned (Phase 3–4) |
| `nxf_dsp` | Reusable signal processing (FFT, filters, dynamics, reverb, delay) | ⬜ planned (Phase 5) |
| `nxf_midi` | MIDI messages, input/output, event routing | ⬜ planned (Phase 6) |
| `nxf_gui` | Windows, components, controls, layout, input events | ⬜ planned (Phase 7) |
| `nxf_graphics` | 2D rendering, text, images, GPU abstraction | ⬜ planned (Phase 8) |
| `nxf_plugin` | Plugin format adapters and hosting interfaces (VST3 first) | ⬜ planned (Phase 10) |
| `nxf_project` | Project persistence, serialization, undo/redo, saved-state | ⬜ planned (Phase 9) |
| `nxf_testing` | Test utilities, mock devices, audio fixtures, benchmarks | ⬜ planned |

Full contracts: `docs/ARCHITECTURE.md` · Threading: `docs/THREADING_MODEL.md` ·
Real-time rules: `docs/REALTIME_AUDIO_RULES.md` · API rules: `docs/API_DESIGN.md`

## Build

**Requirements:** a C++20 compiler (MSVC 2022 / GCC 11+ / Clang 14+) and CMake 3.22+.

Windows x64 (primary target):

```bat
cmake --preset windows-x64
cmake --build --preset windows-x64-release
ctest --preset windows-x64-release
build\windows-x64\examples\hello_nxf\Release\hello_nxf.exe
```

Linux (build verification):

```bash
cmake --preset linux-dev
cmake --build --preset linux-dev
ctest --preset linux-dev
./build/linux-dev/examples/hello_nxf/hello_nxf
```

Expected output of the example:

```text
NXF - NUX Framework 0.3.0
JUCE-independent C++20 framework for NUX audio applications.
```

## Rules that are not negotiable

- **No JUCE dependency** — in source, headers, CMake or binaries.
- **The audio callback never allocates, blocks, locks, logs or touches files.**
  See `docs/REALTIME_AUDIO_RULES.md`.
- **No third-party dependency without a documented license decision**
  (`docs/DEPENDENCIES_AND_LICENSING.md`).
- Modules depend in one direction only; no circular dependencies.

## Contributing

See `CONTRIBUTING.md`. Developed in Sri Lanka 🇱🇰 by Hirusha Nethsara and the
NUX Audio Team.
