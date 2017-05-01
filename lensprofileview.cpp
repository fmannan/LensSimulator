#include "lensprofileview.h"
#include <GL/glu.h>
#include <iostream>
#include <cmath>
#include <cfloat>
#include <limits>
#include <algorithm>
#include <ray.h>
#include <utility.h>
#include <shadinginfo.h>
#include <QDebug>

LensProfileView::LensProfileView(QWidget *parent) :
    QGLWidget(parent), pLens(nullptr), mZoomScale(3.0f), mMaxZoomScale(20.0f),
    mOffsetX(0.0f), mOffsetY(0.0f), mDrawRayBundles(false), mLensElemColor(Colorf(0.0f, 0.0f, 0.0f)), mClearColor(Colorf(1.0f))
{
}

LensProfileView::LensProfileView(const Lens *lens, QWidget *parent) :
    QGLWidget(parent), pLens(lens), mZoomScale(3.0f), mMaxZoomScale(20.0f),
    mOffsetX(0.0f), mOffsetY(0.0f), mDrawRayBundles(false), mLensElemColor(Colorf(0.0f, 0.0f, 0.0f)), mClearColor(Colorf(1.0f))
{
}

void LensProfileView::loadLens(const Lens *lens)
{
    pLens = lens;
}

void LensProfileView::zoomIn()
{
    mZoomScale = std::min(mMaxZoomScale, mZoomScale + 0.5f);
    updateGL();
}

void LensProfileView::zoomOut()
{
    mZoomScale = std::max(1.0f, mZoomScale - 0.5f);
    updateGL();
}

void LensProfileView::pan(int x, int y)
{
    this->mOffsetX += x;
    this->mOffsetY += y;
    updateGL();
}

void LensProfileView::resetView()
{
    mZoomScale = 3.0f;
    mOffsetX = 0;
    mOffsetY = 0;
    updateGL();
}

void LensProfileView::paint(QPainter & painter)
{
}

void LensProfileView::drawRefractionTest()
{
    glPushMatrix();
    glScalef(3.0f, 3.0f, 1.0f);
    glm::vec3 orig(2.0f, 2.0f, 0.0f);
    glm::vec3 u = glm::normalize(glm::vec3(-5.0f, -1.0f, 0.0f));
    Ray ray(orig, u);


    glm::vec3 normal = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
    glm::vec3 nrot(-normal.y, normal.x, 0.0f);
    drawLine(-20 * nrot.x, -20 * nrot.y, 20 * nrot.x, 20 * nrot.y);
    drawLine(-20 * normal.x, -20 * normal.y, 20 * normal.x, 20 * normal.y);

    glm::vec3 t = computeRefraction(u, normal, 1.603, 1.658);

    qDebug() << u.x << " " << u.y << " " << u.z;
    qDebug() << t.x << " " << t.y << " " << t.z;
    drawLine(0, 0, -20 * u.x, -20 * u.y);
    drawLine(0, 0, 20 * t.x, 20 * t.y);
    glPopMatrix();
}

void LensProfileView::paintGL()
{
    //std::cout << "paintGL called " << mWidth << " , " << mHeight << std::endl;

    //qDebug() << mClearColor.red() << mClearColor.green() << mClearColor.blue() << mClearColor.alpha() << endl;
    glClearColor (mClearColor.red(),mClearColor.green(),mClearColor.blue(),mClearColor.alpha());

    glClear (GL_COLOR_BUFFER_BIT);

    glLoadIdentity();
    glPushMatrix();
    glTranslatef(mWidth/2.0f + mOffsetX, mHeight/2.0f + mOffsetY, 0.0f);
    glScalef(mZoomScale, mZoomScale, 1.0f);
    drawGrid();
    //drawRefractionTest();
    drawLensAndRayBundle();
    glPopMatrix();

    QFont font;
    font.setPointSize(10);
    renderText(10, 10, 0, "Test", font);
}

void LensProfileView::resizeGL(int width, int height)
{
    mWidth = width;
    mHeight = height;

    glViewport(0, 0, mWidth, mHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, mWidth, 0, mHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void LensProfileView::drawFilledCircle(GLfloat x, GLfloat y, GLfloat radius, int slices) {
    int i;

    GLfloat twicePi = 2.0f * PI;
    GLfloat part = twicePi / slices;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // center of circle
    for (i = 0; i <= slices; i++) {
        glVertex2f(x + (radius * cos(i *  part)),  y + (radius * sin(i * part)));
    }
    glEnd();
}

void LensProfileView::drawArc(GLfloat x, GLfloat y, GLfloat radius, GLfloat angle_deg, int slices)
{
    GLfloat part = (angle_deg * PI/180.0) / slices; //twicePi / slices;

    glBegin(GL_LINE_STRIP);
    for (int i = -slices; i <= slices; i++) {
        glVertex2f(x + (radius * cos(i *  part)), y + (radius * sin(i * part)));
    }
    glEnd();
}

void LensProfileView::drawLine(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1)
{
    glBegin(GL_LINES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();
}

void LensProfileView::drawGrid()
{
    glColor3f(1.0f, 0.5f, 0.5f);
    drawLine(-mWidth/2.0f, 0.0, mWidth/2.0f, 0.0f);
    //drawLine(0.0, -mHeight/2.0f, 0.0, mHeight/2.0f);
}

void LensProfileView::drawLensElement(double x, double y, double radius, double thickness, double aperture, int slices)
{
    if(radius != radius) { // aperture stop
        drawLine(x, y - aperture/2.0, x, y - aperture * 1.5);
        drawLine(x, y + aperture/2.0, x, y + aperture * 1.5);
    } else {
        double half_angle = asin(aperture / (2.0 * radius));
        drawArc(x + radius, y, -radius, half_angle * 180.0/PI, slices);
        //drawLine(x, y + aperture/2.0, x + thickness, y + aperture/2.0);
        //drawLine(x, y - aperture/2.0, x + thickness, y - aperture/2.0);
    }
}

void LensProfileView::drawLensAndRayBundle()
{
    if(pLens == nullptr) {
        qDebug() << "pLens is null\n";
        return;
    }
    glPushMatrix();
    glTranslatef(-pLens->thickness()/2.0f, 0.0f, 0.0f);

    drawLens();
    drawRayBundles();

    glPopMatrix();
}

void LensProfileView::drawLens()
{
    //glColor3f(1.0f, 1.0f, 1.0f);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glLineWidth(2.0f);
    glColor3f(mLensElemColor.red(), mLensElemColor.green(), mLensElemColor.blue());
    double xpos = 0.0;
    double ypos = 0.0;

    for(auto it = pLens->cbegin(); it != pLens->cend(); it++)
    {
        //qDebug() << it->radius << " " << it->thickness << " " << it->aperture;
        double thickness = is_nan(it->nd()) ? 0 : it->thickness();
        drawLensElement(xpos, ypos, it->radius(), thickness, it->aperture(), 128);
        xpos += it->thickness();
    }
    // draw sensor plane
    double sensor_aperture =  35.0 / sqrt(2.0);
    drawLine(xpos, -sensor_aperture/2.0, xpos, sensor_aperture/2.0);
    glPopAttrib();
}

void LensProfileView::drawRayBundles()
{
    if(!mDrawRayBundles)
        return;
    for(auto it = mRayBundles.begin(); it != mRayBundles.end(); it++) {
        drawRayBundleYZtoXY(it->first, it->second);
    }
}

void LensProfileView::drawRayPath(const RayPath& path) const
{
    glBegin(GL_LINE_STRIP);
    glm::vec3 pt1;
    for(auto it = path.begin(); it != path.end(); it++) {
        const glm::vec3& pt0 = it->origin();
        pt1 = pt0 + 20.0f * it->direction();
        //qDebug() << pt0.x << ", " << pt0.y << ", " << pt0.z;
        //qDebug() << pt1.x << ", " << pt1.y << ", " << pt1.z;
        glVertex3f(pt0.x, pt0.y, pt0.z);
    }
    glVertex3f(pt1.x, pt1.y, pt1.z);
    glEnd();
}

void LensProfileView::drawRayPathYZtoXY(const RayPath &path) const
{
    glBegin(GL_LINE_STRIP);
    glm::vec3 pt1;
    //qDebug() << "-----";
    for(auto it = path.begin(); it != path.end(); it++) {
        const glm::vec3& pt0 = it->origin();
        pt1 = pt0 + 20.0f * it->direction();
//        qDebug() << pt0.x << ", " << pt0.y << ", " << pt0.z;
//        qDebug() << pt1.x << ", " << pt1.y << ", " << pt1.z;
        glVertex2f(pt0.z, pt0.y);
    }
    glVertex2f(pt1.z, pt1.y);
    glEnd();
}

void LensProfileView::drawRayBundleYZtoXY(const RayBundle &bundle, const Colorf& clr) const
{
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    glColor3f(clr.red(), clr.green(), clr.blue());
    for(auto it = bundle.cbegin(); it != bundle.cend(); it++) {
        drawRayPathYZtoXY(*it);
    }
    glPopAttrib();
}
