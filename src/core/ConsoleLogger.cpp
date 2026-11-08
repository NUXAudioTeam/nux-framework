#include <nxf/core/Logger.h>

#include <iostream>

namespace nxf::core {

void ConsoleLogger::log(LogLevel level, std::string_view message)
{
    const std::lock_guard lock(mutex_);
    std::clog << '[' << toString(level) << "] " << message << '\n';
}

} // namespace nxf::core
