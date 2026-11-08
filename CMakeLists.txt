cmake_minimum_required(VERSION 3.22)

project(nxf
    VERSION 0.10.0
    DESCRIPTION "NXF - NUX Framework: an original, JUCE-independent C++ framework for audio applications"
    LANGUAGES CXX)

# NXF is C++20. No exceptions: modules may not quietly require a newer
# standard, because the SDK promise (Phase 12) is a C++20 consumer build.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NXF_BUILD_TESTS "Build NXF tests" ON)
option(NXF_BUILD_EXAMPLES "Build NXF examples" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(NxfWarnings)

# --- Targets -----------------------------------------------------------------
# Phase 0 implements nxf_core only. Other modules exist as documented
# boundaries (docs/ARCHITECTURE.md), not as empty libraries pretending
# to be finished work.

add_library(nxf_core
    src/core/Version.cpp
    src/core/Time.cpp
    src/core/ConsoleLogger.cpp)
add_library(nxf::core ALIAS nxf_core)
target_include_directories(nxf_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(nxf_core PUBLIC cxx_std_20)
nxf_set_warnings(nxf_core)

add_library(nxf_platform
    src/platform/FileSystem.cpp
    src/platform/Timing.cpp)
if(WIN32)
    target_sources(nxf_platform PRIVATE src/platform/windows/SystemInfo.cpp)
else()
    target_sources(nxf_platform PRIVATE src/platform/posix/SystemInfo.cpp)
endif()
add_library(nxf::platform ALIAS nxf_platform)
target_include_directories(nxf_platform PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(nxf_platform PUBLIC cxx_std_20)
target_link_libraries(nxf_platform PUBLIC nxf::core)
nxf_set_warnings(nxf_platform)

add_library(nxf_audio
    src/audio/AudioDevice.cpp
    src/audio/AudioNode.cpp
    src/audio/AudioGraph.cpp
    src/audio/OfflineAudioBackend.cpp)
if(WIN32)
    target_sources(nxf_audio PRIVATE src/audio/windows/WasapiAudioBackend.cpp)
    target_link_libraries(nxf_audio PRIVATE ole32)
else()
    target_sources(nxf_audio PRIVATE src/audio/SystemBackendFallback.cpp)
endif()
add_library(nxf::audio ALIAS nxf_audio)
target_include_directories(nxf_audio PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(nxf_audio PUBLIC cxx_std_20)
target_link_libraries(nxf_audio PUBLIC nxf::core nxf::platform)
nxf_set_warnings(nxf_audio)

add_library(nxf_dsp
    src/dsp/Fft.cpp
    src/dsp/Biquad.cpp
    src/dsp/Compressor.cpp
    src/dsp/DelayLine.cpp
    src/dsp/Reverb.cpp
    src/dsp/Resampler.cpp)
add_library(nxf::dsp ALIAS nxf_dsp)
target_include_directories(nxf_dsp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(nxf_dsp PUBLIC cxx_std_20)
target_link_libraries(nxf_dsp PUBLIC nxf::core)
nxf_set_warnings(nxf_dsp)

add_library(nxf_midi
    src/midi/MidiMessage.cpp
    src/midi/MidiStreamParser.cpp
    src/midi/MidiRouter.cpp
    src/midi/OfflineMidiBackend.cpp)
if(WIN32)
    target_sources(nxf_midi PRIVATE src/midi/windows/WinMmMidiBackend.cpp)
    target_link_libraries(nxf_midi PRIVATE winmm)
else()
    target_sources(nxf_midi PRIVATE src/midi/SystemMidiBackendFallback.cpp)
endif()
add_library(nxf::midi ALIAS nxf_midi)
target_include_directories(nxf_midi PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(nxf_midi PUBLIC cxx_std_20)
target_link_libraries(nxf_midi PUBLIC nxf::core)
nxf_set_warnings(nxf_midi)

add_library(nxf_gui
    src/gui/Component.cpp
    src/gui/Layout.cpp)
if(WIN32)
    target_sources(nxf_gui PRIVATE src/gui/windows/Win32Window.cpp)
    target_link_libraries(nxf_gui PRIVATE user32 gdi32)
else()
    target_sources(nxf_gui PRIVATE src/gui/WindowFactoryFallback.cpp)
endif()
add_library(nxf::gui ALIAS nxf_gui)
target_include_directories(nxf_gui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(nxf_gui PUBLIC cxx_std_20)
target_link_libraries(nxf_gui PUBLIC nxf::core)
nxf_set_warnings(nxf_gui)

add_library(nxf_graphics
    src/graphics/SoftwareRenderer.cpp
    src/graphics/Waveform.cpp)
add_library(nxf::graphics ALIAS nxf_graphics)
target_include_directories(nxf_graphics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(nxf_graphics PUBLIC cxx_std_20)
target_link_libraries(nxf_graphics PUBLIC nxf::core)
nxf_set_warnings(nxf_graphics)

add_library(nxf_project
    src/project/Value.cpp
    src/project/ProjectDocument.cpp
    src/project/ProjectFile.cpp)
add_library(nxf::project ALIAS nxf_project)
target_include_directories(nxf_project PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(nxf_project PUBLIC cxx_std_20)
target_link_libraries(nxf_project PUBLIC nxf::core nxf::platform)
nxf_set_warnings(nxf_project)

if(NXF_BUILD_EXAMPLES)
    add_subdirectory(examples/hello_nxf)
endif()

if(NXF_BUILD_TESTS)
    enable_testing()
    find_package(Threads REQUIRED)
    set(NXF_UNIT_TESTS
        "CoreVersionTests:nxf.unit.core_version"
        "CoreErrorTests:nxf.unit.core_error"
        "CoreQueueTests:nxf.unit.core_queue"
        "CoreMemoryTests:nxf.unit.core_memory"
        "CoreLoggingTests:nxf.unit.core_logging"
        "CoreTimeTests:nxf.unit.core_time"
        "PlatformFileSystemTests:nxf.platform.filesystem"
        "PlatformSystemInfoTests:nxf.platform.system_info"
        "PlatformTimingTests:nxf.platform.timing"
        "PlatformEventTests:nxf.platform.event"
        "AudioBufferTests:nxf.audio.buffer"
        "AudioStreamTests:nxf.audio.stream"
        "AudioBackendTests:nxf.audio.backend"
        "AudioGraphTests:nxf.audio.graph"
        "TransportTests:nxf.audio.transport"
        "AudioEngineTests:nxf.audio.engine"
        "DspFftTests:nxf.dsp.fft"
        "DspFilterTests:nxf.dsp.filters"
        "DspDynamicsTests:nxf.dsp.dynamics"
        "DspEffectsTests:nxf.dsp.effects"
        "DspResampleTests:nxf.dsp.resample"
        "MidiMessageTests:nxf.midi.message"
        "MidiParserTests:nxf.midi.parser"
        "MidiClockTests:nxf.midi.clock"
        "MidiRouterTests:nxf.midi.router"
        "MidiDeviceTests:nxf.midi.devices"
        "GuiGeometryTests:nxf.gui.geometry"
        "GuiComponentTests:nxf.gui.component"
        "GuiLayoutTests:nxf.gui.layout"
        "GuiControlTests:nxf.gui.controls"
        "GuiWindowTests:nxf.gui.window"
        "GraphicsRendererTests:nxf.graphics.renderer"
        "GraphicsTextImageTests:nxf.graphics.text_image"
        "ProjectValueTests:nxf.project.value"
        "ProjectDocumentTests:nxf.project.document"
        "ProjectFileTests:nxf.project.file")
    foreach(entry IN LISTS NXF_UNIT_TESTS)
        string(REPLACE ":" ";" parts "${entry}")
        list(GET parts 0 source_name)
        list(GET parts 1 test_label)
        add_executable(${source_name} tests/unit/${source_name}.cpp)
        target_link_libraries(${source_name} PRIVATE nxf::core nxf::platform nxf::audio nxf::dsp nxf::midi nxf::gui nxf::graphics nxf::project Threads::Threads)
        target_include_directories(${source_name} PRIVATE tests/unit)
        nxf_set_warnings(${source_name})
        add_test(NAME ${test_label} COMMAND ${source_name})
    endforeach()
endif()

message(STATUS "NXF ${PROJECT_VERSION} configured (C++20). Implemented modules: nxf_core (Phase 1), nxf_platform (Phase 2), nxf_audio (Phase 3-4, hardware unverified), nxf_dsp (Phase 5), nxf_midi (Phase 6, hardware unverified), nxf_gui (Phase 7, display unverified), nxf_graphics (Phase 8), nxf_project (Phase 9). All other modules are planned - see docs/ROADMAP.md.")
