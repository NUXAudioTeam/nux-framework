#pragma once

#include <nxf/core/Error.h>
#include <nxf/core/Result.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace nxf::platform {

/// Filesystem services for worker/UI threads. All paths are UTF-8
/// strings; NXF deliberately does not leak std::filesystem types
/// through this public API. File I/O is forbidden in an audio
/// callback (docs/REALTIME_AUDIO_RULES.md); failures are returned
/// as Error values, never thrown across this boundary.

[[nodiscard]] bool exists(std::string_view path);
[[nodiscard]] nxf::core::Result<std::uint64_t> fileSizeBytes(std::string_view path);
[[nodiscard]] nxf::core::Result<std::string> readTextFile(std::string_view path);
[[nodiscard]] nxf::core::Error writeTextFile(std::string_view path, std::string_view contents);
/// Entry names (not full paths) inside a directory.
[[nodiscard]] nxf::core::Result<std::vector<std::string>> listDirectory(std::string_view path);
/// Creates the directory and any missing parents.
[[nodiscard]] nxf::core::Error createDirectory(std::string_view path);
[[nodiscard]] nxf::core::Error removeFile(std::string_view path);
/// The operating system's temporary directory (always exists on success).
[[nodiscard]] nxf::core::Result<std::string> tempDirectory();

} // namespace nxf::platform
