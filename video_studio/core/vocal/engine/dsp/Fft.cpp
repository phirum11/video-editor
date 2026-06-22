#include "Fft.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace vocal_advance {

void Fft::forward(std::vector<Complex>& data) {
    fftCore(data, false);
}

void Fft::inverse(std::vector<Complex>& data) {
    fftCore(data, true);
}

void Fft::fftCore(std::vector<Complex>& a, bool invert) {
    int n = a.size();

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;

        if (i < j)
            std::swap(a[i], a[j]);
    }

    // Cooley-Tukey FFT
    for (int len = 2; len <= n; len <<= 1) {
        float angle = 2.0f * M_PI / len * (invert ? -1 : 1);
        Complex wlen(std::cos(angle), std::sin(angle));
        for (int i = 0; i < n; i += len) {
            Complex w(1);
            for (int j = 0; j < len / 2; j++) {
                Complex u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    // Scale for inverse FFT
    if (invert) {
        for (Complex& x : a)
            x /= n;
    }
}

} // namespace vocal_advance
