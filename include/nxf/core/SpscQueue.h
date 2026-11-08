#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace nxf::core {

/// A bounded single-producer / single-consumer queue for handing
/// values between two threads - the standard event channel between
/// the UI/worker side and real-time code (docs/THREADING_MODEL.md).
///
/// Real-time contract:
/// - all storage is allocated at construction; tryPush/tryPop never
///   allocate, lock or wait;
/// - exactly ONE thread may call tryPush and exactly ONE (other)
///   thread may call tryPop - SPSC is in the name on purpose;
/// - a full queue refuses the push (returns false); it never
///   overwrites unread data and never blocks the producer.
/// T must be default-constructible and movable. Capacity must be a
/// power of two so indices wrap with a mask instead of a division.
/// Whether std::atomic<std::size_t> is lock-free on a given target
/// is a platform property; this header does not claim it (see
/// docs/REALTIME_AUDIO_RULES.md) - verify per target if it matters.
template <typename T, std::size_t Capacity>
class SpscQueue {
    static_assert(Capacity >= 2 && (Capacity & (Capacity - 1)) == 0,
                  "SpscQueue capacity must be a power of two >= 2");
    static_assert(std::is_default_constructible_v<T>, "SpscQueue element must be default-constructible");
public:
    SpscQueue() = default;
    SpscQueue(const SpscQueue&) = delete;
    SpscQueue& operator=(const SpscQueue&) = delete;

    [[nodiscard]] static constexpr std::size_t capacity() noexcept { return Capacity; }

    /// Producer thread only. Returns false when the queue is full.
    bool tryPush(T value) {
        const auto write = writeIndex_.load(std::memory_order_relaxed);
        const auto read = readIndex_.load(std::memory_order_acquire);
        if (write - read >= Capacity) return false;
        buffer_[write & kMask] = std::move(value);
        writeIndex_.store(write + 1, std::memory_order_release);
        return true;
    }

    /// Consumer thread only. Returns false when the queue is empty.
    bool tryPop(T& out) {
        const auto read = readIndex_.load(std::memory_order_relaxed);
        const auto write = writeIndex_.load(std::memory_order_acquire);
        if (write == read) return false;
        out = std::move(buffer_[read & kMask]);
        readIndex_.store(read + 1, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool empty() const {
        return writeIndex_.load(std::memory_order_acquire) == readIndex_.load(std::memory_order_acquire);
    }
    [[nodiscard]] std::size_t size() const {
        const auto write = writeIndex_.load(std::memory_order_acquire);
        const auto read = readIndex_.load(std::memory_order_acquire);
        return write >= read ? write - read : 0;
    }

private:
    static constexpr std::size_t kMask = Capacity - 1;
    std::array<T, Capacity> buffer_{};
    std::atomic<std::size_t> writeIndex_{0};
    std::atomic<std::size_t> readIndex_{0};
};

} // namespace nxf::core
