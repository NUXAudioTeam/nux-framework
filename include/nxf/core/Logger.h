#pragma once

#include <mutex>
#include <string_view>

namespace nxf::core {

enum class LogLevel { Debug, Info, Warning, Error };

[[nodiscard]] constexpr std::string_view toString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug: return "debug";
        case LogLevel::Info: return "info";
        case LogLevel::Warning: return "warning";
        case LogLevel::Error: return "error";
    }
    return "unknown";
}

/// Logging interface. Implementations define where messages go.
/// Logging performs I/O: it must never be called from an audio
/// callback (docs/REALTIME_AUDIO_RULES.md). Implementations used from
/// several non-audio threads must be safe for concurrent log() calls;
/// both loggers below are.
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(LogLevel level, std::string_view message) = 0;
};

/// A logger that discards everything. Useful as a default and in tests.
class NullLogger final : public ILogger {
public:
    void log(LogLevel, std::string_view) override {}
};

/// Writes "[level] message" lines to std::clog, serialised with a
/// mutex. Non-audio threads only.
class ConsoleLogger final : public ILogger {
public:
    void log(LogLevel level, std::string_view message) override;
private:
    std::mutex mutex_;
};

} // namespace nxf::core
