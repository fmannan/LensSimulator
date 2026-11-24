#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <functional>
#include <limits>
#include <stdexcept>
#include <vector>

// SI units throughout: metres, radians, vacuum wavelengths in metres.
namespace optics {
constexpr double pi = 3.14159265358979323846;
using Complex = std::complex<double>;
inline void positive(double x, const char* name) {
    if (!std::isfinite(x) || x <= 0) throw std::invalid_argument(name);
}
inline void finite(double x, const char* name) {
    if (!std::isfinite(x)) throw std::invalid_argument(name);
}

// Meridional paraxial rays. Angle is dy/dz, restricted to small angles.
struct Ray {
    double height;
    double slope;
    void propagate(double distance) {
        finite(distance, "distance must be finite");
        height += distance * slope;
    }
    void thin_lens(double focal_length) {
        finite(focal_length, "focal length must be finite");
        if (focal_length == 0) throw std::invalid_argument("zero focal length");
        slope -= height / focal_length;
    }
    void refract(double radius, double n_before, double n_after) {
        positive(n_before, "invalid incident index");
        positive(n_after, "invalid transmitted index");
        if (std::isnan(radius) || radius == 0) throw std::invalid_argument("invalid radius");
        slope = (n_before * slope - (n_after - n_before) * height / radius) / n_after;
    }
};

inline double rms_spot(const std::vector<Ray>& rays, double distance) {
    if (rays.empty()) throw std::invalid_argument("empty ray bundle");
    finite(distance, "invalid image distance");
    double mean = 0, sum = 0;
    for (const auto& r : rays) mean += r.height + distance * r.slope;
    mean /= rays.size();
    for (const auto& r : rays) {
        double d = r.height + distance * r.slope - mean;
        sum += d * d;
    }
    return std::sqrt(sum / rays.size());
}

// Sampled coherent scalar field in one transverse dimension, periodic boundary.
// The reference DFT is O(N^2); replace with an FFT backend for large grids.
class Field1D {
public:
    double pitch;
    double wavelength;
    std::vector<Complex> amplitude;

    Field1D(std::size_t count, double dx, double lambda)
        : pitch(dx), wavelength(lambda), amplitude(count, 1.0) {
        if (count < 2) throw std::invalid_argument("grid needs at least two samples");
        positive(dx, "invalid sample pitch");
        positive(lambda, "invalid wavelength");
    }
    double x(std::size_t i) const { return (double(i) - double(amplitude.size() / 2)) * pitch; }
    double power() const {
        double p = 0;
        for (auto a : amplitude) p += std::norm(a);
        return p * pitch;
    }
    void aperture(double half_width) {
        positive(half_width, "invalid aperture half width");
        for (std::size_t i = 0; i < amplitude.size(); ++i)
            if (std::abs(x(i)) > half_width) amplitude[i] = 0;
    }
    void lens(double focal_length, bool hyperbolic = false) {
        finite(focal_length, "invalid focal length");
        if (focal_length == 0) throw std::invalid_argument("zero focal length");
        const double k = 2 * pi / wavelength;
        for (std::size_t i = 0; i < amplitude.size(); ++i) {
            const double xx = x(i);
            // Hyperbolic phase for an ideal focusing phase sheet in air.
            const double opd = hyperbolic
                ? std::copysign(xx * xx / (std::hypot(focal_length, xx) + std::abs(focal_length)), focal_length)
                : xx * xx / (2 * focal_length);
            amplitude[i] *= std::polar(1.0, -k * opd);
        }
    }
    void quantize_phase(unsigned levels) {
        if (levels < 2) throw std::invalid_argument("need at least two phase levels");
        const double step = 2 * pi / levels;
        for (auto& a : amplitude) a = std::polar(std::abs(a), step * std::round(std::arg(a) / step));
    }
    void propagate(double distance) {
        finite(distance, "invalid propagation distance");
        if (distance < 0) throw std::invalid_argument("forward propagation only");
        const auto count = amplitude.size();
        const double k = 2 * pi / wavelength;
        std::vector<Complex> spectrum(count), result(count);
        for (std::size_t m = 0; m < count; ++m) {
            for (std::size_t j = 0; j < count; ++j)
                spectrum[m] += amplitude[j] * std::polar(1.0, -2 * pi * double(m) * double(j) / double(count));
            const auto signed_m = m <= count / 2 ? double(m) : double(m) - double(count);
            const double kx = 2 * pi * signed_m / (count * pitch);
            const double kz2 = k * k - kx * kx;
            spectrum[m] *= kz2 >= 0 ? std::polar(1.0, distance * std::sqrt(kz2))
                                   : Complex(std::exp(-distance * std::sqrt(-kz2)), 0);
        }
        for (std::size_t j = 0; j < count; ++j) {
            for (std::size_t m = 0; m < count; ++m)
                result[j] += spectrum[m] * std::polar(1.0, 2 * pi * double(m) * double(j) / double(count));
            result[j] /= double(count);
        }
        amplitude = std::move(result);
    }
};

// Lossless two-mode interferometer restricted to the one-photon sector.
// [a', b'] = [cos(theta), i sin(theta); i sin(theta), cos(theta)] [a, b].
struct SinglePhoton {
    Complex a = 1.0;
    Complex b = 0.0;
    void beam_splitter(double theta) {
        finite(theta, "invalid splitter angle");
        auto old_a = a;
        a = std::cos(theta) * a + Complex(0, std::sin(theta)) * b;
        b = Complex(0, std::sin(theta)) * old_a + std::cos(theta) * b;
    }
    void phase_a(double phase) {
        finite(phase, "invalid phase");
        a *= std::polar(1.0, phase);
    }
    double probability_a() const { return std::norm(a); }
    double probability_b() const { return std::norm(b); }
};

struct TwoPhotonProbabilities { double both_a, coincidence, both_b; };
// Input |1,1>, indistinguishable photons, ideal lossless beam splitter.
inline TwoPhotonProbabilities hong_ou_mandel(double theta) {
    finite(theta, "invalid splitter angle");
    const double bunch = 0.5 * std::pow(std::sin(2 * theta), 2);
    return {bunch, std::pow(std::cos(2 * theta), 2), bunch};
}

struct Optimum { double parameter, value; unsigned evaluations; };
// Bounded scalar search for a unimodal objective; endpoints included.
inline Optimum minimize(const std::function<double(double)>& objective,
                        double lower, double upper, double tolerance = 1e-9,
                        unsigned max_iterations = 128) {
    finite(lower, "invalid lower bound"); finite(upper, "invalid upper bound");
    positive(tolerance, "invalid tolerance");
    if (lower >= upper || max_iterations == 0) throw std::invalid_argument("invalid search bounds or budget");
    Optimum best{lower, std::numeric_limits<double>::infinity(), 0};
    auto evaluate = [&](double x) {
        double y = objective(x);
        ++best.evaluations;
        finite(y, "objective returned nonfinite value");
        if (y < best.value) { best.parameter = x; best.value = y; }
        return y;
    };
    evaluate(lower); evaluate(upper);
    constexpr double ratio = 0.6180339887498948482;
    double c = upper - ratio * (upper - lower), d = lower + ratio * (upper - lower);
    double fc = evaluate(c), fd = evaluate(d);
    for (unsigned i = 0; i < max_iterations && upper - lower > tolerance; ++i) {
        if (fc < fd) {
            upper = d; d = c; fd = fc;
            c = upper - ratio * (upper - lower); fc = evaluate(c);
        } else {
            lower = c; c = d; fc = fd;
            d = lower + ratio * (upper - lower); fd = evaluate(d);
        }
    }
    return best;
}
} // namespace optics
