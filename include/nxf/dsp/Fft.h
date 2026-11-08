#pragma once

#include <complex>
#include <cstddef>
#include <span>
#include <vector>

namespace nxf::dsp {

/// An iterative radix-2 FFT over complex float data. The size is
/// fixed at construction (power of two) and the caller owns the
/// buffers, so forward()/inverse() allocate nothing - the same code
/// can run in an analyser thread or, with preallocated buffers, next
/// to the audio path. inverse() includes the 1/N scaling, so
/// forward-then-inverse restores the input.
class Fft {
public:
    explicit Fft(std::size_t size);
    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    void forward(std::span<std::complex<float>> data) const;
    void inverse(std::span<std::complex<float>> data) const;
    [[nodiscard]] static constexpr bool isPowerOfTwo(std::size_t n) noexcept { return n >= 2 && (n & (n - 1)) == 0; }

private:
    std::size_t size_ = 0;
    std::vector<std::size_t> bitReversal_;
    std::vector<std::complex<float>> twiddles_;
};

} // namespace nxf::dsp
