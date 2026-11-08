#include <nxf/platform/Event.h>
#include "TestSupport.h"
#include <atomic>
#include <chrono>
#include <thread>
int main() {
    using namespace nxf::platform;
    using namespace std::chrono_literals;
    Event event;
    NXF_CHECK(!event.waitFor(10ms));           // not signalled yet
    event.set();
    NXF_CHECK(event.waitFor(10ms));
    event.wait();                              // already set: returns
    event.reset();
    NXF_CHECK(!event.waitFor(10ms));
    Event gate; std::atomic<bool> passed{false};   // cross-thread signal
    std::thread worker([&] { gate.wait(); passed = true; });
    std::this_thread::sleep_for(20ms);
    NXF_CHECK(!passed.load());
    gate.set();
    worker.join();
    NXF_CHECK(passed.load());
    return nxf::test::report("PlatformEventTests");
}
