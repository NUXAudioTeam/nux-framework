# NXF Dependencies and Licensing Inventory

**Project license:** MIT — © 2026 Hirusha Nethsara / NUX Audio Team
(see `LICENSE` at the repository root). Developer: Hirusha Nethsara,
NUX Audio Team, Sri Lanka.

Rule: nothing is a dependency of NXF until it is listed here with
its purpose, version and license. An unclear license stops the work
that needs it (Phase 0 stop conditions).

## Current dependencies (Phase 0)

| Dependency | Purpose | Version | License | How it is used |
|---|---|---|---|---|
| C++ standard library | language runtime | C++20 (compiler-provided) | compiler-vendor terms | the only runtime dependency of `nxf_core` |
| CMake | build system | ≥ 3.22 | BSD-3-Clause | build-time only; not linked or shipped |

That is the complete list. In particular:

- **JUCE: not a dependency.** No JUCE headers, modules, CMake
  packages, binaries or source code are used, copied or adapted,
  and NXF makes no claim of JUCE API compatibility.
- No test framework dependency: Phase 0 tests are a minimal
  self-contained runner (see `tests/unit/`), so no licensing
  decision was needed yet. Choosing one (e.g. for Phase 1) requires
  an entry here first.

## Planned dependencies (not yet adopted — each needs its own decision)

| Candidate | For phase | License position |
|---|---|---|
| Steinberg VST3 SDK | 10 (Plugin Framework) | Steinberg VST3 licensing terms apply; NXF code will keep SDK types behind the `nxf_plugin` adapter. Decision required before Phase 10 starts. |
| ASIO SDK (Steinberg) | 3–4 (optional adapter) | Proprietary SDK terms; ASIO stays an optional adapter, never a core dependency. Decision required before use. |
| WASAPI / Direct2D / DirectWrite | 3, 7–8 | Windows SDK system components; no separate redistribution license for linking the platform APIs. |
| A unit-test framework (TBD) | 1+ | Must be permissively licensed and listed here before adoption. |
