#include "optics.h"
#include <iostream>
#include <string>
using namespace optics;
int checks = 0;
void check(bool ok, const std::string& message) {
    ++checks; if (!ok) throw std::runtime_error(message);
}
void near(double a, double b, double tolerance, const std::string& message) {
    check(std::abs(a - b) <= tolerance, message);
}
template<class F> void rejects(F f, const std::string& message) {
    bool rejected = false;
    try { f(); } catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, message);
}
int main() {
    try {
        Ray ray{0.001, 0}; ray.thin_lens(0.05); ray.propagate(0.05);
        near(ray.height, 0, 1e-15, "collimated ray focuses at focal distance");
        Ray r{0.001, 0.02}; r.refract(0.03, 1, 1.5); r.refract(0.03, 1.5, 1);
        near(r.slope, 0.02, 1e-14, "refraction reversibility");
        Ray planar{0.001, 0.03}; planar.refract(std::numeric_limits<double>::infinity(), 1, 1.5);
        near(planar.slope, 0.02, 1e-14, "planar Snell paraxial limit");
        Field1D plane(32, 1e-6, 550e-9); plane.propagate(1e-5);
        auto phase = std::polar(1.0, 2 * pi * 1e-5 / plane.wavelength);
        for (auto a : plane.amplitude) near(std::abs(a - phase), 0, 1e-12, "plane wave analytical phase");
        Field1D pupil(64, 2e-6, 550e-9); pupil.aperture(30e-6); pupil.lens(0.001, true);
        auto input = pupil.amplitude; double power = pupil.power(); pupil.propagate(0);
        for (std::size_t j = 0; j < input.size(); ++j)
            near(std::abs(input[j] - pupil.amplitude[j]), 0, 1e-12, "zero distance identity");
        pupil.propagate(0.001);
        near(pupil.power() / power, 1, 1e-12, "propagating spectrum power conservation");
        Field1D ev(16, 550e-9 / 4, 550e-9);
        for (std::size_t i = 0; i < 16; ++i) ev.amplitude[i] = i % 2 ? -1.0 : 1.0;
        power = ev.power(); ev.propagate(550e-9);
        check(ev.power() < power * 1e-6, "evanescent decay");
        Field1D flat(128, 5e-6, 550e-9); flat.aperture(100e-6);
        auto focus = flat; focus.lens(0.005, true); focus.propagate(0.005); flat.propagate(0.005);
        check(std::norm(focus.amplitude[64]) > 2 * std::norm(flat.amplitude[64]), "phase sheet focuses");
        Field1D q(32, 2e-6, 550e-9); q.lens(0.001, true); power = q.power(); q.quantize_phase(8);
        near(q.power(), power, 1e-15, "quantization preserves pupil power");
        SinglePhoton p; p.beam_splitter(pi / 4);
        near(p.probability_a(), 0.5, 1e-14, "balanced splitter");
        p.phase_a(pi); p.beam_splitter(pi / 4);
        near(p.probability_a(), 1, 1e-14, "Mach-Zehnder interference");
        near(p.probability_a() + p.probability_b(), 1, 1e-14, "photon normalization");
        auto hom = hong_ou_mandel(pi / 4);
        near(hom.coincidence, 0, 1e-14, "HOM suppression");
        near(hom.both_a + hom.both_b + hom.coincidence, 1, 1e-14, "two photon normalization");
        near(hong_ou_mandel(0).coincidence, 1, 1e-14, "transparent splitter");
        auto fit = minimize([](double x) { return (x - 0.037) * (x - 0.037); }, 0, 0.1);
        near(fit.parameter, 0.037, 1e-8, "known interior minimum");
        fit = minimize([](double x) { return x; }, 0, 1);
        near(fit.parameter, 0, 0, "endpoint minimum");
        rejects([] { Field1D f(1, 1, 1); }, "reject small grid");
        rejects([] { Field1D f(8, -1, 1); }, "reject invalid pitch");
        rejects([] { Field1D f(8, 1, 1); f.propagate(-1); }, "reject negative propagation");
        rejects([] { minimize([](double) { return std::numeric_limits<double>::quiet_NaN(); }, 0, 1); }, "reject nonfinite objective");
        std::cout << "Passed " << checks << " physics and validation checks\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << "\n"; return 1; }
}
