#pragma once
// Minimal shared test support for the framework-free NXF test runners.
// A failing check prints file/line and bumps a counter the runner's
// main() turns into a non-zero exit code (that is what CTest sees).
#include <iostream>
#include <string_view>
namespace nxf::test {
inline int failures = 0;
inline void check(bool ok, std::string_view expr, const char* file, int line) {
    if (!ok) { ++failures; std::cerr << "FAIL " << file << ':' << line << ": " << expr << '\n'; }
}
inline int report(const char* suite) {
    if (failures != 0) { std::cerr << suite << ": " << failures << " check(s) failed\n"; return 1; }
    std::cout << suite << ": all checks passed\n"; return 0;
}
} // namespace nxf::test
#define NXF_CHECK(expr) ::nxf::test::check(static_cast<bool>(expr), #expr, __FILE__, __LINE__)
