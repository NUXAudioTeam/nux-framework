#pragma once

#include <cstdint>

namespace nxf::platform {

/// Worker/UI-thread timing helpers. Sleeping and yielding are
/// forbidden in an audio callback; these exist for background work,
/// polling loops and tests. For measuring intervals use
/// nxf::core::monotonicNanoseconds().

void sleepMilliseconds(std::uint32_t milliseconds);
void yieldThread() noexcept;

} // namespace nxf::platform
