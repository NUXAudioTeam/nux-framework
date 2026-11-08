#include <nxf/core/SpscQueue.h>
#include "TestSupport.h"
#include <atomic>
#include <cstdint>
#include <thread>
int main() {
    using nxf::core::SpscQueue;
    SpscQueue<int, 8> q;
    NXF_CHECK(q.empty());
    NXF_CHECK(q.capacity() == 8);
    for (int i = 0; i < 8; ++i) NXF_CHECK(q.tryPush(i));
    NXF_CHECK(!q.tryPush(99));            // full: refuses, never overwrites
    int v = -1;
    for (int i = 0; i < 8; ++i) { NXF_CHECK(q.tryPop(v)); NXF_CHECK(v == i); }  // FIFO
    NXF_CHECK(!q.tryPop(v));              // empty
    NXF_CHECK(q.empty());
    for (int round = 0; round < 100; ++round) {   // wrap-around
        NXF_CHECK(q.tryPush(round));
        NXF_CHECK(q.tryPop(v)); NXF_CHECK(v == round);
    }
    // One producer, one consumer, 200k items: nothing lost, nothing reordered.
    SpscQueue<std::int64_t, 1024> big;
    constexpr std::int64_t n = 200000;
    std::atomic<bool> orderOk{true};
    std::thread producer([&] { for (std::int64_t i = 0; i < n;) if (big.tryPush(i)) ++i; });
    std::thread consumer([&] { std::int64_t expect = 0, got = 0;
        while (expect < n) if (big.tryPop(got)) { if (got != expect) orderOk = false; ++expect; } });
    producer.join(); consumer.join();
    NXF_CHECK(orderOk.load());
    NXF_CHECK(big.empty());
    return nxf::test::report("CoreQueueTests");
}
