// hello_nxf - the smallest possible NXF program.
// It exists to prove the build, the public include layout and the
// nxf_core target all work. It is not a demo of framework features:
// most NXF modules are planned, not implemented (see docs/ROADMAP.md).

#include <nxf/core/Version.h>

#include <iostream>

int main()
{
    std::cout << nxf::core::frameworkName() << ' ' << nxf::core::versionString() << '\n';
    std::cout << "JUCE-independent C++20 framework for NUX audio applications.\n";
    return 0;
}
