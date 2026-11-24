#include "mainwindow.h"
#include <QApplication>
#include <QDoubleSpinBox>
#include <QFile>
#include <QLabel>
#include <QTemporaryDir>
#include <iostream>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    auto require = [](bool ok, const char* message) { if (!ok) throw std::runtime_error(message); };
    try {
        require(argc == 2, "sample required");
        MainWindow window;
        require(window.loadPrescription(QString::fromLocal8Bit(argv[1])), "load through GUI controller");
        window.show(); app.processEvents();
        auto results = window.findChild<QLabel*>("analysisResults");
        require(results && results->text().contains("Optimized RMS"), "core results visible in viewer");
        const auto originalResults = results->text();
        auto beam = window.findChild<QDoubleSpinBox*>("beamDiameter");
        require(beam != nullptr, "beam control exists"); beam->setValue(4);
        require(results->text().contains("Parameters changed"), "parameter change invalidates old result");
        require(window.analyze(), "reanalyze from controls");
        require(results->text() != originalResults, "beam change reaches geometric tracer");
        auto lower = window.findChild<QDoubleSpinBox*>("focusLower");
        auto upper = window.findChild<QDoubleSpinBox*>("focusUpper");
        lower->setValue(upper->value());
        require(!window.analyze() && results->text().contains("unavailable"), "bad bounds are surfaced");
        lower->setValue(0); require(window.analyze(), "recover after invalid bounds");
        require(!window.loadPrescription("does-not-exist.txt"), "missing file reported");
        require(window.analyze(), "previous prescription survives failed load");
        QTemporaryDir output; auto svgPath = output.path() + "/lens.svg";
        require(window.exportSvg(svgPath), "SVG export succeeds");
        QFile svg(svgPath); require(svg.open(QIODevice::ReadOnly), "SVG file readable");
        auto content = svg.readAll();
        require(content.contains("<svg") && content.contains("<path"), "SVG contains actual rendered optics");
        auto view = window.findChild<LensProfileView*>("lensProfileView");
        require(view && !view->grab().isNull(), "viewer renders without OpenGL");
        std::cout << "Qt load, core analysis, controls, validation, recovery and SVG passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << "\n"; return 1; }
}
