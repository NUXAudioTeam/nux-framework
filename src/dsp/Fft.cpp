#include <nxf/dsp/Fft.h>

#include <cmath>
#include <stdexcept>

namespace nxf::dsp {

Fft::Fft(std::size_t size) : size_(size)
{
    if (!isPowerOfTwo(size)) throw std::invalid_argument("Fft size must be a power of two >= 2");
    std::size_t bits = 0;
    while ((std::size_t{1} << bits) < size) ++bits;
    bitReversal_.resize(size);
    for (std::size_t i = 0; i < size; ++i) {
        std::size_t reversed = 0;
        for (std::size_t b = 0; b < bits; ++b)
            if (i & (std::size_t{1} << b)) reversed |= std::size_t{1} << (bits - 1 - b);
        bitReversal_[i] = reversed;
    }
    twiddles_.resize(size / 2);
    constexpr double pi = 3.14159265358979323846;
    for (std::size_t i = 0; i < size / 2; ++i) {
        const double angle = -2.0 * pi * static_cast<double>(i) / static_cast<double>(size);
        twiddles_[i] = {static_cast<float>(std::cos(angle)), static_cast<float>(std::sin(angle))};
    }
}

namespace {
void transform(std::span<std::complex<float>> data, const std::vector<std::size_t>& bitReversal,
               const std::vector<std::complex<float>>& twiddles, bool inverse)
{
    const std::size_t n = data.size();
    for (std::size_t i = 0; i < n; ++i)
        if (bitReversal[i] > i) std::swap(data[i], data[bitReversal[i]]);

    for (std::size_t length = 2; length <= n; length <<= 1) {
        const std::size_t step = n / length;
        for (std::size_t base = 0; base < n; base += length) {
            for (std::size_t j = 0; j < length / 2; ++j) {
                std::complex<float> w = twiddles[j * step];
                if (inverse) w = std::conj(w);
                const auto even = data[base + j];
                const auto odd = data[base + j + length / 2] * w;
                data[base + j] = even + odd;
                data[base + j + length / 2] = even - odd;
            }
        }
    }
    if (inverse) {
        const float scale = 1.0f / static_cast<float>(n);
        for (auto& value : data) value *= scale;
    }
}
} // namespace

void Fft::forward(std::span<std::complex<float>> data) const
{
    if (data.size() == size_) transform(data, bitReversal_, twiddles_, false);
}

void Fft::inverse(std::span<std::complex<float>> data) const
{
    if (data.size() == size_) transform(data, bitReversal_, twiddles_, true);
}

} // namespace nxf::dsp
