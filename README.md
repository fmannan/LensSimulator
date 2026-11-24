# LensSimulator

A C++17 lens prescription viewer with geometric ray tracing and bounded RMS
focus optimization. The engine builds without Qt; the optional desktop viewer
supports Qt 5.9+ or Qt 6. CMake is the canonical build system.

This repository focuses on lens simulation. The broader ray/wave/quantum optics
application belongs in the separate `optics_sim` project. The initial exploratory
wave, ideal-metalens and quantum examples remain available in `optics_workbench`.

## Build

Requires a C++17 compiler and CMake 3.21+ for presets (3.16+ for manual configure).
Use a Qt installation compatible with your compiler/architecture for desktop.

```powershell
cmake --preset desktop -DCMAKE_PREFIX_PATH="C:/path/to/Qt/6.x/msvc2022_64"
cmake --build --preset desktop
# Windows: copy the Qt libraries and plugins beside the application.
cmake --build --preset desktop --target deploy_qt
ctest --preset desktop
.\build\desktop\Release\QtLensSimulator.exe
```

On Linux/macOS, the same configure/build/test presets work; omit the Windows-only
`deploy_qt` target. Set `CMAKE_PREFIX_PATH` to your Qt prefix if it is not already
discoverable. Single-configuration generators place the executable directly in
`build/desktop`; macOS produces `QtLensSimulator.app`. Linux needs Widgets and SVG
development packages (for example `qtbase5-dev` and `libqt5svg5-dev` on Ubuntu).

On the current Windows machine, PATH points to an older CMake. A compatible
version is bundled with Visual Studio at:
`C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin`.
The tested Qt prefix is `C:/ProgramData/Anaconda3/Library` (Qt 5.9.7).
Use these tool paths per shell; no global settings need changing.

For an engine-only build with **no Qt dependency**:

```powershell
cmake --preset headless
cmake --build --preset headless
ctest --preset headless
```

Manual configuration: `cmake -S . -B build/native -DLENS_BUILD_QT=OFF`.
Options: `LENS_BUILD_QT`, `LENS_BUILD_EXAMPLES`, `BUILD_TESTING`.
The headless CI matrix covers Windows, macOS and Linux; an additional Linux job
builds and tests the Qt viewer. These workflow definitions need a CI run to
establish results on those hosts. Local validation used Windows/MSVC and Qt 5;
Qt 6 and other operating systems have not been exercised locally.

`deploy_qt` requires `windeployqt` from the Qt installation. It bundles Qt runtime
dependencies, not an installer or the MSVC redistributable. macOS deployment,
Linux distribution packaging, signing, and installers are release work still
to be added. Source portability and packaged releases are separate milestones.

## Desktop viewer

The viewer starts with the bundled `lens0.txt`, when available, or accepts a path:

```powershell
.\build\desktop\Release\QtLensSimulator.exe .\LensPrescription\lens1.txt
```

- Open a five-column prescription: radius, thickness, following-medium index,
  Abbe number, aperture diameter. Prescription lengths are **millimetres**.
- Inspect surfaces and prescription values. The table is read-only.
- Set incident beam diameter and focus bounds, then select **Optimize image
  plane**. The beam is collimated, on-axis, and sampled with 41 meridional rays.
- Cyan rays come from the existing geometric tracer. The purple dashed line is
  the prescribed sensor, and orange is the optimized plane. Search distances
  are measured from the last surface vertex, not the first surface or a principal
  plane. The final thickness entry specifies the prescribed sensor distance.
- Results show transmitted ray count and RMS spot at prescribed/optimized planes.
  A solution at a bound is marked so you can widen or revise the search interval.
- Scroll to zoom, middle-drag to pan, use Reset view to fit, and export SVG.

Rendering uses QWidget/QPainter; no OpenGL or GLU dependency is needed. SVG uses
the same rendering path. Parameter changes clear stale analysis. Failed file
loads preserve the previously loaded prescription.

## Engine and web boundary

- `optics_core`: header-only SI numerical primitives, independent of Qt.
- `lens_model`: prescription parsing, existing geometric tracer, and analysis.
- `core/lensanalysis.h`: bridges geometric output rays in mm to SI rays for the
  core RMS objective and bounded optimizer. Results contain paths and metrics,
  without Qt types or rendering calls.
- `lens_viewer`: optional desktop interface; only this layer depends on Qt.

`Lens::load(std::istream&)` allows a future browser binding to supply uploaded
prescription text without native file dialogs or filesystem access. A future
Emscripten/Embind module can expose this engine to JavaScript; the native file
path overload is for desktop/CLI use. No WebAssembly binary or web UI is included
yet. Emscripten compatibility must still be compiled and tested.

For a web interface shared with a packaged desktop app, a browser frontend can
use the C++ engine through WebAssembly, or call a native/server worker for larger
jobs. Qt remains a separate desktop adapter rather than a requirement for those
interfaces. Optiland is also a candidate Python backend for the broader project:
evaluate overlap before rebuilding its classical design features. A Python
backend would need its own local worker or server integration.

## Physics limits and tests

See [the validation record](docs/validation.md) for the tested environment,
passed checks, and remaining platform validation.

The desktop analysis uses the geometric tracer's actual spherical/planar
intersections and refraction, not the core's separate paraxial thin-lens demo.
It is a meridional, on-axis geometric RMS calculation with fixed prescription
indices. Dispersion, diffraction, polarization, coatings, 2D spots, and
prescription-variable optimization are not part of the desktop integration.
Clipped/TIR rays are excluded and the transmitted count is reported. The legacy
tracer uses float geometry, which limits small-spot numerical precision.

Tests cover analytical numerical models, singlet focus against lensmaker BFL,
an independent closed-form RMS focus, supplied prescriptions, SI conversion,
transactional parsing, planar surfaces, GUI control updates, invalid bounds,
failed-load recovery, painting, and actual SVG geometry.

The exploratory CLI offers `all`, `ray`, `wave`, `metalens`, and `quantum` demos.
Its wave model is 1D scalar propagation on a periodic grid with an O(N^2) DFT;
sampling convergence is not established by power conservation. Its metalens is
an ideal phase sheet, without electromagnetic nanostructure modeling. Quantum
support is limited to ideal two-mode one-photon interference and specified
two-photon Hong–Ou–Mandel probabilities. These are prototypes, not full solvers.

## Older build files

`QtLensSimulator.pro` is a lightweight compatibility qmake build for the updated
viewer. Use CMake for development and tests. The checked-in historical `.sln`
and `.vcxproj` files are obsolete and are not maintained; generate a fresh Visual
Studio solution in a build directory with CMake. `mainwindow.ui` is historical;
the current interface is assembled in `mainwindow.cpp`.
