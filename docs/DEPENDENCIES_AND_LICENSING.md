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

## Adopted optional dependencies

The Steinberg VST3 SDK row below records an adopted, optional
dependency (off by default). Everything else in the table is
still only planned.

## Planned dependencies (not yet adopted — each needs its own decision)

| Candidate | For phase | License position |
|---|---|---|
| Steinberg VST3 SDK | 10 (Plugin Framework) | **Adopted (2026-11-08), optional.** SDK 3.8 is distributed by Steinberg under the MIT License (the older GPLv3/proprietary dual licensing is no longer offered; `LICENSE.txt` in the SDK). NXF pins release `v3.8.1_build_84` (commit `3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96`). It is used only when configuring with `-DNXF_ENABLE_VST3=ON` (default OFF, so the base build stays dependency-light); the SDK is fetched at configure time or supplied with `-DNXF_VST3_SDK_DIR=...`, never vendored into this repository. SDK types stay behind `src/plugin/vst3/Vst3Adapter.cpp`; no SDK headers appear in public NXF headers. The VST trademark and logo rules are Steinberg's separate branding terms and are not a code license. |
| ASIO SDK (Steinberg) | 3–4 (optional adapter) | Proprietary SDK terms; ASIO stays an optional adapter, never a core dependency. Decision required before use. |
| WASAPI / Direct2D / DirectWrite | 3, 7–8 | Windows SDK system components; no separate redistribution license for linking the platform APIs. |
| A unit-test framework (TBD) | 1+ | Must be permissively licensed and listed here before adoption. |
