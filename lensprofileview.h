#ifndef LENSPROFILEVIEW_H
#define LENSPROFILEVIEW_H
#include <QWidget>
#include <QColor>
#include <QPointF>
#include <limits>
#include "lens.h"

class QPainter;
class LensProfileView : public QWidget {
    Q_OBJECT
public:
    explicit LensProfileView(QWidget* parent = nullptr);
    void loadLens(const Lens* lens);
    void setRays(RayBundle paths, double focus_z_mm);
    void resetView();
    void paint(QPainter& painter);
protected:
    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
private:
    const Lens* mLens = nullptr;
    RayBundle mPaths;
    double mFocusZ = std::numeric_limits<double>::quiet_NaN();
    double mZoom = 1;
    QPointF mPan, mLast;
    bool mPanning = false;
};
#endif
