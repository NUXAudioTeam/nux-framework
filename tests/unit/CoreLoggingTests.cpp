#include <nxf/core/Logger.h>
#include "TestSupport.h"
#include <string>
#include <vector>
namespace {
struct CaptureLogger final : nxf::core::ILogger {
    std::vector<nxf::core::LogLevel> levels;
    std::vector<std::string> messages;
    void log(nxf::core::LogLevel level, std::string_view message) override {
        levels.push_back(level); messages.emplace_back(message);
    }
};
}
int main() {
    using namespace nxf::core;
    NXF_CHECK(toString(LogLevel::Info) == "info");
    NXF_CHECK(toString(LogLevel::Error) == "error");
    CaptureLogger cap;
    cap.log(LogLevel::Warning, "careful");
    NXF_CHECK(cap.levels.size() == 1);
    NXF_CHECK(cap.levels[0] == LogLevel::Warning);
    NXF_CHECK(cap.messages[0] == "careful");
    NullLogger nullLog;
    nullLog.log(LogLevel::Debug, "goes nowhere");   // must simply not crash
    ConsoleLogger consoleLog;
    consoleLog.log(LogLevel::Info, "CoreLoggingTests console smoke");
    ILogger& asInterface = cap;
    asInterface.log(LogLevel::Error, "via interface");
    NXF_CHECK(cap.messages.size() == 2);
    return nxf::test::report("CoreLoggingTests");
}
