#include "lensanalysis.h"

namespace optics {
LensAnalysis analyze_lens(const Lens& lens, double beam_diameter_m,
                          double lower_distance_m, double upper_distance_m, unsigned ray_count) {
    if (lens.empty()) throw std::invalid_argument("Load a lens prescription first");
    positive(beam_diameter_m, "Beam diameter must be positive");
    finite(lower_distance_m, "Invalid lower focus distance");
    finite(upper_distance_m, "Invalid upper focus distance");
    if (lower_distance_m < 0 || lower_distance_m >= upper_distance_m)
        throw std::invalid_argument("Focus bounds must satisfy 0 <= lower < upper");
    if (ray_count < 3 || ray_count > 10001) throw std::invalid_argument("Ray count must be 3 to 10001");
    LensAnalysis result;
    const auto& first = *lens.cbegin();
    const auto& last = *(lens.cend() - 1);
    // Use the prescription's double thickness sum rather than the float vertex coordinate.
    double last_z_mm = 0;
    for (auto it = lens.cbegin(); it + 1 != lens.cend(); ++it) last_z_mm += it->thickness();
    result.last_surface_m = last_z_mm * 1e-3;
    result.sensor_distance_m = last.thickness() * 1e-3;
    const double beam_radius_mm = beam_diameter_m * 500;
    if (beam_radius_mm > first.aperture() / 2 + 1e-8)
        throw std::invalid_argument("Beam diameter exceeds the first surface aperture");
    double launch_z_mm = -1;
    if (std::isfinite(first.radius()) && first.radius() < 0) {
        const double r = std::abs(first.radius());
        launch_z_mm -= beam_radius_mm * beam_radius_mm / (r + std::sqrt(r * r - beam_radius_mm * beam_radius_mm));
    }
    result.launched = ray_count;
    for (unsigned i = 0; i < ray_count; ++i) {
        const double y = beam_radius_mm * (2.0 * i / (ray_count - 1) - 1);
        auto path = lens.raytraceFrontToBack(::Ray(glm::vec3(0, float(y), float(launch_z_mm)), glm::vec3(0, 0, 1)));
        if (path.size() != lens.surfaceCount() + 1) continue;
        const auto& end = path.back();
        const auto& direction = end.direction();
        if (!std::isfinite(direction.z) || direction.z <= 1e-6 || !std::isfinite(direction.y)) continue;
        const double slope = double(direction.y) / direction.z;
        const double height_mm = end.origin().y + (last_z_mm - end.origin().z) * slope;
        if (!std::isfinite(height_mm)) continue;
        result.outgoing.push_back({height_mm * 1e-3, slope});
        result.paths.push_back(std::move(path));
    }
    if (result.outgoing.size() < 3) throw std::runtime_error("Fewer than three rays transmitted; reduce beam diameter");
    double min_slope = result.outgoing.front().slope, max_slope = min_slope;
    for (const auto& ray : result.outgoing) { min_slope = std::min(min_slope, ray.slope); max_slope = std::max(max_slope, ray.slope); }
    if (max_slope - min_slope < 1e-12) throw std::runtime_error("Transmitted rays are parallel; no unique focus");
    result.sensor_rms_m = rms_spot(result.outgoing, result.sensor_distance_m);
    result.focus = minimize([&](double z) { return rms_spot(result.outgoing, z); }, lower_distance_m, upper_distance_m, 1e-10);
    result.at_bound = std::abs(result.focus.parameter - lower_distance_m) < 1e-9 ||
                      std::abs(result.focus.parameter - upper_distance_m) < 1e-9;
    return result;
}
}
