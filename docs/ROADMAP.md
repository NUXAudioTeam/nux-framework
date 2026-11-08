# NXF Roadmap

One phase at a time. A phase is **done** only when its acceptance
criteria pass with tests actually executed — not when the code merely
exists. Future phases are planned, not implemented.

| Phase | Scope | Status |
|---|---|---|
| 0 | Architecture & Repository Foundation — repo, CMake layout, coding standards, module contracts, licensing inventory | 🟡 **in progress** (this phase) |
| 1 | NXF Core — types, memory, logging, errors, threading, events, utilities | ⬜ planned |
| 2 | Platform Layer — Windows filesystem, timing, synchronization, system information | ⬜ planned |
| 3 | Audio Foundation — device abstraction, buffers, streams, WASAPI input/output | ⬜ planned |
| 4 | Audio Engine — graph, nodes, routing, mixing, transport, real-time processing | ⬜ planned |
| 5 | DSP Engine — FFT, filters, EQ, dynamics, reverb, delay, resampling | ⬜ planned |
| 6 | MIDI System — messages, devices, clocks, routing, event queues | ⬜ planned |
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
