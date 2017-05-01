#ifndef LENSPROFILEVIEW_H
#define LENSPROFILEVIEW_H

#include <QGLWidget>
#include <memory>
#include <utility>
#include "lens.h"

/**
 * @brief The LensProfileView class
 *
 */

class LensProfileView : public QGLWidget
{
    Q_OBJECT
public:
    explicit LensProfileView(QWidget *parent = 0);
    explicit LensProfileView(const Lens* lens, QWidget *parent = 0);

    void    loadLens(const Lens* lens);

    void    setMaxZoomScale(float maxZoom) { mMaxZoomScale = maxZoom; }
    void    zoomIn();
    void    zoomOut();
    void    pan(int x, int y);
    void    resetView();

	void paint(QPainter &painter);

    void    addRayBundle(const RayBundle& bundle, const Colorf& clr = Colorf(1.0f))
    {
        mRayBundles.push_back(std::make_pair(bundle, clr));
        if(mDrawRayBundles)
            updateGL();
    }

    void    addRayBundle(RayBundle&& bundle, const Colorf& clr = Colorf(1.0f))
    {
        mRayBundles.push_back(std::make_pair(bundle, clr));
        if(mDrawRayBundles)
            updateGL();
    }
    void    clearRayBundles()
    {
        mRayBundles.clear();
        if(mDrawRayBundles)
            updateGL();
    }
    void    setRayBundleDraw(bool bDraw)
    {
        mDrawRayBundles = bDraw;
        if(mDrawRayBundles)
            updateGL();
    }

protected:
    void paintGL();
    void resizeGL(int width, int height);

signals:

public slots:

private:
    int mWidth;
    int mHeight;
    const Lens* pLens;
    float   mZoomScale;
    float   mOffsetX, mOffsetY;
    float   mMaxZoomScale;
    std::vector<std::pair<RayBundle, Colorf>> mRayBundles;
    //std::vector<Colorf> mRayBundlesColor;
    bool  mDrawRayBundles;
    Colorf mLensElemColor;
    Colorf mClearColor;

    void drawFilledCircle(GLfloat x, GLfloat y, GLfloat radius, int slices);
    void drawArc(GLfloat x, GLfloat y, GLfloat radius, GLfloat angle_deg, int slices);
    void drawLine(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1);
    void drawGrid();
    void drawLensElement(double x, double y, double radius, double thickness, double aperture, int slices = 128);
    void drawLens();
    void drawRayBundles();
    void drawLensAndRayBundle();    // ensures that the lens elements and the ray bundles are in the same coordinate system
    void drawRayPath(const RayPath& path) const;
    void drawRayPathYZtoXY(const RayPath& path) const;
    void drawRayBundleYZtoXY(const RayBundle& bundle, const Colorf& clr = Colorf(1.0f)) const;

    void drawRefractionTest();
};

#endif // LENSPROFILEVIEW_H
