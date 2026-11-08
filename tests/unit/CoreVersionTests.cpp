// CoreVersionTests - Phase 0 smoke tests for nxf_core.
//
// Deliberately framework-free: a new test dependency needs a documented
// license decision first (docs/DEPENDENCIES_AND_LICENSING.md), and these
// checks do not need one. A failing check prints its line and the
// process exits non-zero, which is what CTest reports.

#include <nxf/core/Version.h>

#include <iostream>
#include <string_view>

namespace {

int failures = 0;

void check(bool condition, std::string_view expression, int line)
{
    if (!condition) {
        ++failures;
        std::cerr << "FAIL line " << line << ": " << expression << '\n';
    }
}

#define NXF_CHECK(expr) check(static_cast<bool>(expr), #expr, __LINE__)

} // namespace

int main()
{
    NXF_CHECK(nxf::core::versionMajor() == 0);
    NXF_CHECK(nxf::core::versionMinor() == 3);
    NXF_CHECK(nxf::core::versionPatch() == 0);
    NXF_CHECK(nxf::core::versionString() == "0.3.0");
    NXF_CHECK(nxf::core::frameworkName() == "NXF - NUX Framework");
    NXF_CHECK(!nxf::core::frameworkName().empty());

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All NXF core version tests passed\n";
    return 0;
}
