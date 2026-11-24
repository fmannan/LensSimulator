#pragma once
#include "lens.h"
#include "optics.h"

namespace optics {
struct LensAnalysis {
    RayBundle paths;             // Legacy coordinates in millimetres, for the viewer.
    std::vector<Ray> outgoing;   // SI rays referred to the last surface vertex plane.
    double last_surface_m = 0;
    double sensor_distance_m = 0;
    double sensor_rms_m = 0;
    Optimum focus{};             // Distance measured from the last surface vertex.
    unsigned launched = 0;
    bool at_bound = false;
};

// On-axis collimated meridional rays, constant prescription indices (no dispersion).
// Aperture and search distances are SI; clipping occurs in the geometric tracer.
LensAnalysis analyze_lens(const Lens& lens, double beam_diameter_m,
                          double lower_distance_m, double upper_distance_m,
                          unsigned ray_count = 41);
}
