#include "optics.h"
#include <iomanip>
#include <iostream>
#include <string>
using namespace optics;

void ray_demo() {
    std::vector<Ray> rays;
    for (int i = -10; i <= 10; ++i) {
        Ray ray{i * 0.0001, 0}; ray.thin_lens(0.05); rays.push_back(ray);
    }
    auto fit = minimize([&](double z) { return rms_spot(rays, z); }, 0.02, 0.08);
    std::cout << "Ray: optimized image plane = " << fit.parameter * 1e3
              << " mm, RMS spot = " << fit.value * 1e6 << " um\n";
}
void wave_demo(bool metalens) {
    Field1D pupil(128, 5e-6, 550e-9); pupil.aperture(100e-6);
    auto intensity = [&](double f) {
        auto field = pupil; field.lens(f, metalens); field.propagate(0.005);
        return std::norm(field.amplitude[field.amplitude.size() / 2]);
    };
    // Narrow search bounds isolate the primary focal peak.
    auto fit = minimize([&](double f) { return -intensity(f); }, 0.0045, 0.0055);
    auto focused = pupil; focused.lens(fit.parameter, metalens); focused.propagate(0.005);
    std::cout << (metalens ? "Ideal metalens" : "Scalar wave lens")
              << ": optimized phase focal parameter = " << fit.parameter * 1e3
              << " mm, on-axis intensity gain = " << -fit.value
              << ", sampled power ratio = " << focused.power() / pupil.power() << "\n";
    if (metalens) {
        auto q = pupil; q.lens(fit.parameter, true); q.quantize_phase(8); q.propagate(0.005);
        std::cout << "  8-level phase sheet intensity gain = " << std::norm(q.amplitude[64]) << "\n";
    }
}
void quantum_demo() {
    SinglePhoton state;
    state.beam_splitter(pi / 4); state.phase_a(pi / 2); state.beam_splitter(pi / 4);
    auto hom = hong_ou_mandel(pi / 4);
    std::cout << "Quantum: Mach-Zehnder P(a) = " << state.probability_a()
              << ", P(b) = " << state.probability_b() << "; HOM coincidence = " << hom.coincidence << "\n";
    auto fit = minimize([](double phase) {
        SinglePhoton p; p.beam_splitter(pi / 4); p.phase_a(phase); p.beam_splitter(pi / 4);
        return -p.probability_a();
    }, 0, 2 * pi);
    std::cout << "  Optimized interferometer phase = " << fit.parameter << " rad, P(a) = " << -fit.value << "\n";
}
int main(int argc, char** argv) {
    std::string mode = argc > 1 ? argv[1] : "all";
    if (argc > 2 || (mode != "all" && mode != "ray" && mode != "wave" && mode != "metalens" && mode != "quantum")) {
        std::cerr << "Usage: optics_workbench [all|ray|wave|metalens|quantum]\n"; return 2;
    }
    try {
        std::cout << std::setprecision(9);
        if (mode == "all" || mode == "ray") ray_demo();
        if (mode == "all" || mode == "wave") wave_demo(false);
        if (mode == "all" || mode == "metalens") wave_demo(true);
        if (mode == "all" || mode == "quantum") quantum_demo();
    } catch (const std::exception& e) { std::cerr << e.what() << "\n"; return 1; }
}
