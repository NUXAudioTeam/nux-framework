#pragma once

#include <cstddef>
#include <span>
#include <utility>

namespace nxf::core {

/// An owning buffer whose size is fixed at construction - the
/// allocation pattern real-time code needs: allocate once, up front,
/// then hand out spans that never reallocate (docs/
/// REALTIME_AUDIO_RULES.md). Construction may allocate; span access,
/// operator[] and fill() never do. Move-only, like the ownership it
/// represents.
template <typename T>
class FixedBuffer {
public:
    FixedBuffer() = default;
    explicit FixedBuffer(std::size_t size)
        : data_(size == 0 ? nullptr : new T[size]()), size_(size) {}
    ~FixedBuffer() { delete[] data_; }

    FixedBuffer(const FixedBuffer&) = delete;
    FixedBuffer& operator=(const FixedBuffer&) = delete;
    FixedBuffer(FixedBuffer&& other) noexcept : data_(std::exchange(other.data_, nullptr)), size_(std::exchange(other.size_, 0)) {}
    FixedBuffer& operator=(FixedBuffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] T* data() noexcept { return data_; }
    [[nodiscard]] const T* data() const noexcept { return data_; }
    [[nodiscard]] std::span<T> span() noexcept { return {data_, size_}; }
    [[nodiscard]] std::span<const T> span() const noexcept { return {data_, size_}; }
    T& operator[](std::size_t i) noexcept { return data_[i]; }
    const T& operator[](std::size_t i) const noexcept { return data_[i]; }
    void fill(const T& value) { for (auto& element : span()) element = value; }

private:
    T* data_ = nullptr;
    std::size_t size_ = 0;
};

} // namespace nxf::core
