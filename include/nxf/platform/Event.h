#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace nxf::platform {

/// A manual-reset event: threads wait until another thread signals.
/// For worker/UI coordination (load finished, scan complete, ...).
/// Blocking waits are forbidden in an audio callback; use
/// nxf::core::SpscQueue for anything the audio thread touches.
class Event {
public:
    Event() = default;
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    void set() {
        {
            const std::lock_guard lock(mutex_);
            signaled_ = true;
        }
        condition_.notify_all();
    }
    void reset() {
        const std::lock_guard lock(mutex_);
        signaled_ = false;
    }
    void wait() {
        std::unique_lock lock(mutex_);
        condition_.wait(lock, [this] { return signaled_; });
    }
    /// Returns true if signalled within the timeout.
    template <typename Rep, typename Period>
    [[nodiscard]] bool waitFor(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, timeout, [this] { return signaled_; });
    }

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    bool signaled_ = false;
};

} // namespace nxf::platform
