# NXF API Design

Conventions every NXF module follows. Phase 0 fixes the rules and a
minimal example (`nxf::core` version API); it does not pre-invent
APIs for modules that do not exist yet.

## Naming and layout

- Root namespace: `nxf`; one sub-namespace per module
  (`nxf::core`, `nxf::audio`, `nxf::dsp`, …).
- Public headers: `include/nxf/<module>/<Header>.h`, included as
  `#include <nxf/core/Version.h>`. Internal headers never appear
  under `include/`.
- Types are `PascalCase`, functions and values `camelCase`,
  constants `camelCase` with `constexpr`. No Hungarian notation, no
  macros for logic, `#pragma once` in every header.

## Errors, ownership, lifetime

- Expected failures are values, not exceptions, on any path the
  audio thread can reach; exceptions must never cross a device
  callback or module ABI boundary.
- Ownership is expressed in the type: values and `std::unique_ptr`
  by default; raw pointers/references are non-owning views whose
  lifetime requirement is documented at the declaration.
- Every public API states its thread contract ("UI thread only",
  "audio-thread safe", "any thread") and its real-time constraints
  where relevant.

## Compatibility and versioning

- NXF uses semantic versioning for releases. Before 1.0 (Phase 12),
  public APIs may change, but every change is recorded in the
  changelog of the release that makes it.
- API stability is a Phase 12 promise, not a Phase 0 one: internal
  structure must stay free to improve until the SDK is declared.
- ABI stability is not promised across compiler/CMake changes until
  a release explicitly says so.

## Module dependency rules

- Dependencies point down the layering in `docs/ARCHITECTURE.md`
  only. Circular dependencies are a design bug, fixed by moving the
  shared concept down a layer.
- External SDK types (WASAPI, ASIO, VST3, Direct2D) never appear in
  public NXF headers; they live behind adapters in `src/`.
- A new third-party dependency requires an entry in
  `docs/DEPENDENCIES_AND_LICENSING.md` before, or in the same change
  as, its first use.
