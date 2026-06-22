#pragma once

#include <vector>
#include <complex>

namespace vocal_advance {

class Fft {
public:
    using Complex = std::complex<float>;

    // Performs an in-place Forward Fast Fourier Transform
    // The size of data must be a power of 2
    static void forward(std::vector<Complex>& data);

    // Performs an in-place Inverse Fast Fourier Transform
    // The size of data must be a power of 2
    static void inverse(std::vector<Complex>& data);

private:
    static void fftCore(std::vector<Complex>& a, bool invert);
};

} // namespace vocal_advance
