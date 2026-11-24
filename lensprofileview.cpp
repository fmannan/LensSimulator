#include "lensprofileview.h"
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <QMouseEvent>
#include <algorithm>
#include <cmath>

namespace {
QPointF mousePosition(QMouseEvent* event) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position();
#else
    return event->localPos();
#endif
}
QPen cosmeticPen(const QColor& color, qreal width = 1, Qt::PenStyle style = Qt::SolidLine) {
    QPen pen(color, width, style); pen.setCosmetic(true); return pen;
}
}
LensProfileView::LensProfileView(QWidget* parent) : QWidget(parent) {
    setMinimumSize(400, 300);
    setObjectName("lensProfileView");
    setToolTip(tr("Scroll to zoom; drag with the middle mouse button to pan"));
}
void LensProfileView::loadLens(const Lens* lens) {
    mLens = lens; mPaths.clear(); mFocusZ = std::numeric_limits<double>::quiet_NaN(); resetView();
}
void LensProfileView::setRays(RayBundle paths, double focus_z_mm) {
    mPaths = std::move(paths); mFocusZ = focus_z_mm; update();
}
void LensProfileView::resetView() { mZoom = 1; mPan = {}; update(); }
void LensProfileView::paintEvent(QPaintEvent*) { QPainter painter(this); paint(painter); }
void LensProfileView::paint(QPainter& painter) {
    painter.save();
    painter.fillRect(rect(), QColor("#f6f8fc"));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor("#334155"));
    if (!mLens || mLens->empty()) {
        painter.drawText(rect(), Qt::AlignCenter, tr("Open a lens prescription to begin"));
        painter.restore(); return;
    }
    double extent = std::max(1.0, double(mLens->thickness()));
    if (std::isfinite(mFocusZ)) extent = std::max(extent, mFocusZ);
    double aperture = 1;
    for (auto it = mLens->cbegin(); it != mLens->cend(); ++it) aperture = std::max(aperture, it->aperture());
    const double scale = std::min((width() - 100.0) / (extent + 10), (height() - 110.0) / (aperture * 1.5)) * mZoom;
    painter.translate(width() / 2.0 + mPan.x(), height() / 2.0 + mPan.y());
    painter.scale(scale, -scale);
    painter.translate(-extent / 2, 0);
    painter.setPen(cosmeticPen(QColor("#c0cad9"), 1, Qt::DashLine));
    painter.drawLine(QPointF(-5, 0), QPointF(extent + 5, 0));
    double z = 0;
    painter.setPen(cosmeticPen(QColor("#243b53"), 2));
    for (auto it = mLens->cbegin(); it != mLens->cend(); ++it) {
        const double half = it->aperture() / 2;
        if (it->isAperture()) {
            painter.drawLine(QPointF(z, half), QPointF(z, half * 1.4));
            painter.drawLine(QPointF(z, -half), QPointF(z, -half * 1.4));
        } else {
            QPainterPath surface;
            for (int j = 0; j <= 128; ++j) {
                double y = half * (2.0 * j / 128 - 1), sag = 0;
                if (std::isfinite(it->radius())) {
                    double r = std::abs(it->radius());
                    sag = std::copysign(y * y / (r + std::sqrt(std::max(0.0, r * r - y * y))), it->radius());
                }
                if (j == 0) surface.moveTo(z + sag, y); else surface.lineTo(z + sag, y);
            }
            painter.drawPath(surface);
        }
        z += it->thickness();
    }
    painter.setPen(cosmeticPen(QColor("#7c3aed"), 2, Qt::DashLine));
    painter.drawLine(QPointF(z, -aperture / 2), QPointF(z, aperture / 2));
    if (std::isfinite(mFocusZ)) {
        painter.setPen(cosmeticPen(QColor("#e87916"), 2));
        painter.drawLine(QPointF(mFocusZ, -aperture / 2), QPointF(mFocusZ, aperture / 2));
    }
    painter.setPen(cosmeticPen(QColor(8, 145, 178, 130), 1));
    for (const auto& path : mPaths) {
        if (path.empty()) continue;
        QPainterPath drawing;
        drawing.moveTo(path.front().origin().z, path.front().origin().y);
        for (const auto& ray : path) drawing.lineTo(ray.origin().z, ray.origin().y);
        const auto& last = path.back();
        if (last.direction().z > 1e-6) {
            double end_z = std::max(z, std::isfinite(mFocusZ) ? mFocusZ : z);
            drawing.lineTo(end_z, last.origin().y + (end_z - last.origin().z) * last.direction().y / last.direction().z);
        }
        painter.drawPath(drawing);
    }
    painter.restore();
    painter.save(); painter.setPen(QColor("#334155"));
    painter.drawText(20, 28, tr("Meridional ray trace | lengths in mm"));
    painter.drawText(20, height() - 38, tr("Cyan: transmitted rays   Purple: prescribed sensor   Orange: optimized plane"));
    painter.drawText(20, height() - 18, tr("Scroll to zoom | Middle-drag to pan | Reset view to fit"));
    painter.restore();
}
void LensProfileView::wheelEvent(QWheelEvent* event) {
    mZoom = std::clamp(mZoom * (event->angleDelta().y() > 0 ? 1.15 : 1 / 1.15), 0.2, 20.0);
    update(); event->accept();
}
void LensProfileView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) { mLast = mousePosition(event); mPanning = true; event->accept(); }
    else QWidget::mousePressEvent(event);
}
void LensProfileView::mouseMoveEvent(QMouseEvent* event) {
    if (mPanning) { auto current = mousePosition(event); mPan += current - mLast; mLast = current; update(); event->accept(); }
    else QWidget::mouseMoveEvent(event);
}
void LensProfileView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) { mPanning = false; event->accept(); }
    else QWidget::mouseReleaseEvent(event);
}
