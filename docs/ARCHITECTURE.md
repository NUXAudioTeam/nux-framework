# NXF Architecture

NXF is a modular C++20 framework for audio applications. It is an
original design: no JUCE dependency, no JUCE API compatibility, no
adapted JUCE source. This document defines module boundaries and the
dependency rules between them. Phase 1 implemented `nxf_core`; Phase 2 added `nxf_platform`
(filesystem, timing, events, system information with Windows/POSIX
backend files selected by CMake). The rest are contracts for later phases.

## Layering (dependencies point down only)

```text
Applications        NUX DAW · NUX Synth · NUX Sampler · third-party apps
      |
nxf_gui ──────────► nxf_graphics ──► nxf_platform ──► nxf_core
nxf_plugin ───────► nxf_audio ─────► nxf_platform
nxf_project ──────► nxf_core, nxf_platform
nxf_dsp ──────────► nxf_core          (pure algorithms, no OS, no threads)
nxf_midi ─────────► nxf_core, nxf_platform
nxf_audio ────────► nxf_dsp (optional, for built-in processing nodes)
nxf_testing ──────► any module it verifies (test-only, never shipped in a product target)
```

Rules:

- **No circular dependencies.** If two modules seem to need each
  other, the shared concept moves down into the lower module.
- **Public headers** live in `include/nxf/<module>/` and may only
  include public headers of modules below them.
- **Implementation files** live in `src/<module>/`. Platform backends
  are selected by CMake, never by `#ifdef` scattered through logic.
- `nxf_dsp` stays pure: no devices, no threads, no allocation policy
  surprises — the same code must run in tests, offline renders and
  the audio callback.

## Modules

| Module | Owns | Must never own |
|---|---|---|
| `nxf_core` | foundational types, error representation, logging interfaces, memory utilities, event primitives | OS calls, devices, UI |
| `nxf_platform` | filesystem, clocks/timing, synchronization primitives, system info, windowing primitives | audio policy, DSP maths |
| `nxf_audio` | device abstraction, buffers, streams, engine/graph contracts, routing | GUI, file formats |
| `nxf_dsp` | FFT, filters, EQ, dynamics, reverb, delay, resampling | threads, devices, files |
| `nxf_midi` | MIDI messages, device I/O contracts, clocks, routing, event queues | audio rendering |
| `nxf_gui` | windows, components, controls, layout, input events | DSP, device ownership |
| `nxf_graphics` | 2D renderer contract, text, images, waveform drawing, GPU backend abstraction | widget policy |
| `nxf_plugin` | plugin scanning/hosting contracts, lifecycle, adapter isolation (VST3 SDK stays behind the adapter) | leaking SDK types into NXF public headers |
| `nxf_project` | serialization, project files, undo/redo, saved-state tracking | rendering, device state |
| `nxf_testing` | test utilities, mock devices, fixtures, benchmark helpers | production code paths |

## Platform & adapter strategy

Windows x64 is the first target. Native backends planned:

- Audio: WASAPI (native backend); ASIO as a **separate optional
  adapter**, enabled only when its SDK/licensing conditions are met.
- GUI/graphics: Direct2D/DirectWrite backend first; other platform
  backends later behind the same `nxf_graphics` contract.
- Plugins: Steinberg VST3 SDK as a separately documented dependency,
  isolated behind `nxf_plugin` adapter code.

External SDK types never appear in NXF public headers, so an SDK can
be replaced (or a platform added) without breaking application code.
