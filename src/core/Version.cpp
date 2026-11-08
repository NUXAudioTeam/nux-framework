#include <nxf/core/Version.h>

namespace nxf::core {

std::string_view frameworkName() noexcept
{
    return "NXF - NUX Framework";
}

std::string_view versionString() noexcept
{
    return "0.9.0";
}

} // namespace nxf::core
