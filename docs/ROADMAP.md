# NXF Roadmap

One phase at a time. A phase is **done** only when its acceptance
criteria pass with tests actually executed — not when the code merely
exists. Future phases are planned, not implemented.

| Phase | Scope | Status |
|---|---|---|
| 0 | Architecture & Repository Foundation — repo, CMake layout, coding standards, module contracts, licensing inventory | ✅ done |
| 1 | NXF Core — types, memory, logging, errors, threading, events, utilities | ✅ done |
| 2 | Platform Layer — Windows filesystem, timing, synchronization, system information | ✅ done |
| 3 | Audio Foundation — device abstraction, buffers, streams, WASAPI input/output | 🟡 code complete: abstraction + offline backend tested, WASAPI compiles in CI; **opening a real device is NOT PERFORMED (no hardware)** |
| 4 | Audio Engine — graph, nodes, routing, mixing, transport, real-time processing | ✅ done (offline-verified; device path still hardware-unverified) |
| 5 | DSP Engine — FFT, filters, EQ, dynamics, reverb, delay, resampling | ✅ done (known-answer tests; resampler is a linear baseline, not final quality) |
| 6 | MIDI System — messages, devices, clocks, routing, event queues | 🟡 code complete: pure logic + offline backend tested, WinMM compiles in CI; **real MIDI hardware NOT PERFORMED**. WinMM sysex output not yet supported |
| 7 | GUI Framework — windows, components, controls, event handling, layout engine | ⬜ planned |
| 8 | Graphics Engine — 2D renderer, text, images, waveform drawing, GPU acceleration | ⬜ planned |
| 9 | Project & State System — serialization, project files, undo/redo, saved-state | ⬜ planned |
| 10 | Plugin Framework — VST3 scanning, hosting, processing, plugin windows, lifecycle | ⬜ planned |
| 11 | Media & File I/O — WAV, audio import/export, metadata, file streams | ⬜ planned |
| 12 | Developer SDK — public headers, CMake package, examples, documentation, API stability | ⬜ planned |
| 13 | NUX DAW Integration — connect NXF modules to NUX DAW one by one; remove JUCE from the DAW | ⬜ planned |
| 14 | Production Readiness — stress tests, sanitizers, benchmarks, compatibility, release | ⬜ planned |

## Phase gates

- Phase 1 does not start until Phase 0 is reviewed and accepted.
- NUX DAW (Phase 13) is touched only when the replacing NXF module
  is production-ready and tested. The working JUCE-based DAW is
  never put at risk for framework work.
- Anything needing physical audio hardware is reported as
  **not performed** until it has actually run on hardware. A green
  CI build proves code, not sound.

## Phase 3 verification note

Verified: buffer/view behaviour, stream configuration validation,
the offline stream lifecycle and callback pump, backend enumeration
logic (offline), and that the WASAPI backend **compiles and links**
under MSVC in CI (Debug + Release).

Not performed: enumerating or opening a physical device, duplex
capture/render, latency, glitch behaviour. CI runners have no audio
hardware, so device-level claims wait for a run on a real Windows
PC - the same honesty rule as NUX DAW's hardware checklist.
