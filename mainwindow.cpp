#include "mainwindow.h"
#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenuBar>
#include <QPainter>
#include <QPushButton>
#include <QStatusBar>
#include <QSvgGenerator>
#include <QTableWidget>
#include <QToolBar>
#include <QVBoxLayout>

namespace {
QDoubleSpinBox* distanceControl(const char* name, double minimum, double maximum, double value, QWidget* parent) {
    auto box = new QDoubleSpinBox(parent);
    box->setObjectName(name); box->setRange(minimum, maximum); box->setDecimals(4);
    box->setSuffix(" mm"); box->setValue(value); return box;
}
std::filesystem::path nativePath(const QString& path) {
#ifdef _WIN32
    return std::filesystem::path(path.toStdWString());
#else
    return std::filesystem::path(path.toUtf8().constData());
#endif
}
}
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(tr("Lens Simulator")); resize(1280, 820);
    mView = new LensProfileView(this); setCentralWidget(mView);
    auto file = menuBar()->addMenu(tr("&File"));
    auto open = file->addAction(tr("&Open prescription...")); open->setShortcut(QKeySequence::Open);
    connect(open, &QAction::triggered, this, [this] {
        auto path = QFileDialog::getOpenFileName(this, tr("Open lens prescription"),
            QApplication::applicationDirPath() + "/LensPrescription", tr("Lens prescriptions (*.txt *.TXT);;All files (*)"));
        if (!path.isEmpty()) loadPrescription(path);
    });
    auto save = file->addAction(tr("Export &SVG..."));
    connect(save, &QAction::triggered, this, [this] {
        auto path = QFileDialog::getSaveFileName(this, tr("Export lens view"), "lens.svg", tr("SVG files (*.svg)"));
        if (!path.isEmpty()) exportSvg(path);
    });
    auto toolbar = addToolBar(tr("Lens tools")); toolbar->addAction(open); toolbar->addAction(save);
    auto reset = toolbar->addAction(tr("Reset view"));
    connect(reset, &QAction::triggered, mView, &LensProfileView::resetView);

    auto dock = new QDockWidget(tr("Focus analysis"), this); dock->setObjectName("analysisDock");
    auto panel = new QWidget(dock); auto layout = new QVBoxLayout(panel);
    auto description = new QLabel(tr("Collimated on-axis beam\nGeometric tracing + core RMS optimizer"), panel);
    layout->addWidget(description);
    auto form = new QFormLayout;
    mBeam = distanceControl("beamDiameter", 0.01, 10000, 2, panel);
    mLower = distanceControl("focusLower", 0, 1e6, 1, panel);
    mUpper = distanceControl("focusUpper", 0, 1e6, 200, panel);
    form->addRow(tr("Beam diameter"), mBeam);
    form->addRow(tr("Focus lower bound"), mLower);
    form->addRow(tr("Focus upper bound"), mUpper);
    layout->addLayout(form);
    auto boundsNote = new QLabel(tr("Focus distances are measured from the last surface vertex.\nIndices are prescription values; dispersion is not modeled."), panel);
    boundsNote->setWordWrap(true); layout->addWidget(boundsNote);
    mAnalyze = new QPushButton(tr("Optimize image plane"), panel);
    mAnalyze->setObjectName("analyzeButton"); mAnalyze->setEnabled(false);
    connect(mAnalyze, &QPushButton::clicked, this, [this] { analyze(); }); layout->addWidget(mAnalyze);
    mResults = new QLabel(tr("Open a prescription to analyze its focus."), panel);
    mResults->setObjectName("analysisResults"); mResults->setWordWrap(true);
    mResults->setTextInteractionFlags(Qt::TextSelectableByMouse); layout->addWidget(mResults);
    auto invalidate = [this] {
        if (mLens.empty()) return;
        mResults->setText(tr("Parameters changed. Optimize the image plane to update results."));
        mView->setRays({}, std::numeric_limits<double>::quiet_NaN());
    };
    for (auto control : {mBeam, mLower, mUpper})
        connect(control, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, invalidate);
    layout->addStretch(); dock->setWidget(panel); addDockWidget(Qt::RightDockWidgetArea, dock);

    auto tableDock = new QDockWidget(tr("Prescription (mm)"), this);
    tableDock->setObjectName("prescriptionDock");
    mTable = new QTableWidget(0, 5, tableDock);
    mTable->setHorizontalHeaderLabels({tr("Radius"), tr("Thickness"), tr("Index"), tr("Abbe"), tr("Aperture")});
    mTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableDock->setWidget(mTable); addDockWidget(Qt::BottomDockWidgetArea, tableDock);
    auto viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(dock->toggleViewAction()); viewMenu->addAction(tableDock->toggleViewAction());
    statusBar()->showMessage(tr("Ready — open a lens prescription"));
}
bool MainWindow::loadPrescription(const QString& path) {
    try {
        mLens.load(nativePath(path));
        mView->loadLens(&mLens);
        mTable->setRowCount(static_cast<int>(mLens.surfaceCount()));
        int row = 0;
        for (auto it = mLens.cbegin(); it != mLens.cend(); ++it, ++row) {
            const double values[] = {it->radius(), it->thickness(), it->nd(), it->Vno(), it->aperture()};
            for (int column = 0; column < 5; ++column)
                mTable->setItem(row, column, new QTableWidgetItem(std::isnan(values[column]) ? "—" : QString::number(values[column], 'g', 8)));
        }
        mBeam->setMaximum(mLens.cbegin()->aperture());
        mBeam->setValue(std::min(2.0, mLens.cbegin()->aperture()));
        const auto& last = *(mLens.cend() - 1);
        mLower->setValue(0); mUpper->setValue(std::max(200.0, last.thickness() * 3));
        mAnalyze->setEnabled(true);
        setWindowTitle(tr("%1 — Lens Simulator").arg(QFileInfo(path).fileName()));
        analyze(); // Loading succeeds even when the selected system has no forward focus.
        return true;
    } catch (const std::exception& e) {
        statusBar()->showMessage(tr("Unable to load: %1").arg(QString::fromUtf8(e.what())));
        return false;
    }
}
bool MainWindow::analyze() {
    try {
        const auto result = optics::analyze_lens(mLens, mBeam->value() * 1e-3,
            mLower->value() * 1e-3, mUpper->value() * 1e-3);
        mView->setRays(result.paths, (result.last_surface_m + result.focus.parameter) * 1e3);
        QString text = tr("Transmitted rays: %1 / %2\nPrescribed sensor distance: %3 mm\nSensor RMS: %4 µm\n\nOptimized distance: %5 mm\nOptimized RMS: %6 µm")
            .arg(static_cast<unsigned>(result.outgoing.size())).arg(result.launched)
            .arg(result.sensor_distance_m * 1e3, 0, 'f', 4).arg(result.sensor_rms_m * 1e6, 0, 'g', 6)
            .arg(result.focus.parameter * 1e3, 0, 'f', 4).arg(result.focus.value * 1e6, 0, 'g', 6);
        if (result.at_bound) text += tr("\n\nBest plane is at a search bound; adjust bounds to investigate focus.");
        mResults->setText(text); statusBar()->showMessage(tr("Focus analysis complete")); return true;
    } catch (const std::exception& e) {
        mView->setRays({}, std::numeric_limits<double>::quiet_NaN());
        mResults->setText(tr("Analysis unavailable: %1").arg(QString::fromUtf8(e.what())));
        statusBar()->showMessage(tr("Focus analysis unavailable")); return false;
    }
}
bool MainWindow::exportSvg(const QString& path) {
    if (mLens.empty()) { statusBar()->showMessage(tr("Load a lens before exporting")); return false; }
    QSvgGenerator generator; generator.setFileName(path);
    generator.setSize(mView->size()); generator.setViewBox(mView->rect());
    generator.setTitle(tr("Lens Simulator — lens and focus analysis"));
    QPainter painter;
    if (!painter.begin(&generator)) { statusBar()->showMessage(tr("Unable to write SVG")); return false; }
    mView->paint(painter); painter.end(); statusBar()->showMessage(tr("SVG exported")); return true;
}
