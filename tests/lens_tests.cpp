#include "core/lensanalysis.h"
#include <iostream>
#include <sstream>

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<class F> void rejects(F f) {
    bool failed = false;
    try { f(); } catch (const std::exception&) { failed = true; }
    require(failed, "invalid input must be rejected");
}
int main(int argc, char** argv) {
    try {
        require(argc == 2, "sample directory required");
        Lens lens;
        require(lens.empty() && lens.thickness() == 0, "empty lens must be initialized");
        rejects([&] { optics::analyze_lens(lens, 0.002, 0, 0.2); });
        // Symmetric biconvex singlet; no aperture-stop row required.
        std::istringstream singlet("\xEF\xBB\xBF  # UTF-8 BOM and indented comment\n50,5,1.5,50,10\n-50,50,nan,nan,10\n");
        lens.load(singlet);
        auto result = optics::analyze_lens(lens, 0.002, 0.02, 0.08);
        require(result.outgoing.size() == 41, "all small-aperture singlet rays should transmit");
        // Lensmaker thick-lens BFL: f*(1-(n-1)*t/(n*R1)).
        double power = 0.5 * (1 / 0.05 - 1 / -0.05 + 0.5 * 0.005 / (1.5 * 0.05 * -0.05));
        double expected_bfl = (1 - 0.5 * 0.005 / (1.5 * 0.05)) / power;
        require(std::abs(result.focus.parameter - expected_bfl) < 0.0001, "geometric focus should approach thick-lens BFL");
        require(result.focus.value < result.sensor_rms_m, "optimized RMS must improve prescribed sensor RMS");
        require(std::abs(result.last_surface_m - 0.005) < 1e-12, "mm to SI conversion");
        // Compare golden-section result with an independent closed-form bundle variance minimum.
        double mean_y = 0, mean_slope = 0;
        for (auto ray : result.outgoing) { mean_y += ray.height; mean_slope += ray.slope; }
        mean_y /= result.outgoing.size(); mean_slope /= result.outgoing.size();
        double cov = 0, variance = 0;
        for (auto ray : result.outgoing) {
            cov += (ray.height - mean_y) * (ray.slope - mean_slope);
            variance += (ray.slope - mean_slope) * (ray.slope - mean_slope);
        }
        require(std::abs(result.focus.parameter + cov / variance) < 1e-8, "optimizer agrees with analytic RMS minimum");
        for (const auto* text : {"1,2,3,4,5,6\n", "abc,2,1.5,50,1\n", "50,-1,1.5,50,10\n", "# empty\n", "0,2,1.5,50,1\n"}) {
            rejects([&] { std::istringstream invalid(text); lens.load(invalid); });
            require(lens.surfaceCount() == 2 && lens.thickness() == 55, "failed load preserves current prescription");
        }
        std::istringstream planar("inf,2,1.5,50,10\ninf,10,nan,nan,10\n");
        Lens plate; plate.load(planar);
        auto path = plate.raytraceFrontToBack(Ray(glm::vec3(0, 1, -1), glm::vec3(0, 0, 1)));
        require(path.size() == 3 && std::isfinite(path.back().origin().y), "planar surfaces trace without NaNs");
        rejects([&] { optics::analyze_lens(plate, 0.002, 0, 0.2); });
        rejects([&] { optics::analyze_lens(lens, 0.02, 0, 0.2); });
        rejects([&] { optics::analyze_lens(lens, 0.002, 0.2, 0.1); });
        for (const auto* name : {"lens0.txt", "lens1.txt", "DGaussF1.4_7elem.txt"}) {
            lens.load(std::filesystem::path(argv[1]) / name);
            result = optics::analyze_lens(lens, 0.002, 0, 0.3);
            require(result.outgoing.size() >= 3 && std::isfinite(result.focus.value), "sample lens integration");
            std::cout << name << ": focus " << result.focus.parameter * 1e3 << " mm, RMS " << result.focus.value * 1e6 << " um\n";
        }
        std::cout << "Lens parser, geometric tracer and SI focus integration passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << "\n"; return 1; }
}
