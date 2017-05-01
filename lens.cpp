#include "lens.h"
#include <QFile>
#include <QString>
#include <QTextStream>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <limits>
#include <cmath>
#include <vector>
#include <glm.hpp>
#include <ray.h>
#include <utility.h>
#include <sphere.h>
#include <QDebug>

Lens::Lens()
{
}

Lens::Lens(const QString &filename)
{
    load(filename);
}

void Lens::init()
{
    mElems.clear();
    mLensThickness = 0;
    mIdxApertureStop = -1;
}

void Lens::load(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Unable to open file");
        return;
    }

    init(); // clear all previous data

    QTextStream in(&file);
    QTextStream out(stdout);

    double elemOffset = 0.0;
    int idx = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        //std::cout << line << std::endl;
        //qDebug() << line;
        //out << line << "\n";
        if(line.isEmpty()) continue;
        if(line.at(0) == '#') // skip comments
            continue;
        QStringList param_list = line.split(",");
        double params[5] = {0};

        std::transform(param_list.cbegin(), param_list.cend(), params,
                       [](QString data)
                        { if(data.isEmpty())
                            return std::numeric_limits<double>::quiet_NaN();
                          return data.toDouble();
                        });
        std::for_each(params, params + 5, [&out](double v) { out << v << " ";});
        out << "\n";

        mElems.push_back(LensElement(params, elemOffset));
        if(mElems.back().isAperture())
            mIdxApertureStop = idx;
        idx++;

        elemOffset += mElems.back().thickness();
    }
    // first edge starts from 0. lens thickness is the front to back edge distance
    // the back edge is the sensor
    mLensThickness = elemOffset;
}

void Lens::findExitPupil(double &zPos, double& aperture)
{
    RayBundle rayBundles;

    findExitPupil(zPos, aperture, rayBundles, rayBundles);
}

void Lens::findExitPupil(double &zPos, double& aperture, RayBundle& topRayBundles, RayBundle& bottomRayBundles)
{
    // shoot rays from the two endpoints of the aperture stop and find the position of each
    // ray tracing is performed from iris to back
    glm::vec3 aperture_stop_pos = mElems[mIdxApertureStop].edgePos();
    glm::vec3 pt = aperture_stop_pos;
    pt.y = mElems[mIdxApertureStop].aperture()/2.0;

    qDebug() << "top : " << pt.x << " , " << pt.y << " , " << pt.z;

    glm::vec3 ptTop, ptBottom;
    topRayBundles = raytraceRayBundleIrisToBack(glm::vec4(pt, 1.0f), 10);
    intersectionRayBundle(topRayBundles, ptTop, int(mElems.size()) - mIdxApertureStop);

    qDebug() << "img top : " << ptTop.x << " , " << ptTop.y << " , " << ptTop.z;

    pt.y = -pt.y;
    bottomRayBundles = raytraceRayBundleIrisToBack(glm::vec4(pt, 1.0f), 10);
    intersectionRayBundle(bottomRayBundles, ptBottom, int(mElems.size()) - mIdxApertureStop);

    zPos = ptTop.z;
    aperture = std::abs(ptTop.y - ptBottom.y);

    qDebug() << "bottom : " << pt.x << " , " << pt.y << " , " << pt.z;
    qDebug() << "img bottom : " << ptBottom.x << " , " << ptBottom.y << " , " << ptBottom.z;

    qDebug() << "Exit Pupil z : " << zPos;
    qDebug() << "Exit Pupil size : " << aperture;
}

void Lens::findEntrancePupil(double &zPos, double& aperture)
{
    RayBundle rayBundles;
    findEntrancePupil(zPos, aperture, rayBundles, rayBundles);
}

void Lens::findEntrancePupil(double &zPos, double& aperture, RayBundle& topRayBundles, RayBundle& bottomRayBundles)
{
    // shoot rays from the two endpoints of the aperture stop and find the position of each
    // ray tracing is performed from iris to back
    glm::vec3 aperture_stop_pos = mElems[mIdxApertureStop].edgePos();
    glm::vec3 pt = aperture_stop_pos;
    pt.y = mElems[mIdxApertureStop].aperture()/2.0;

    qDebug() << "top : " << pt.x << " , " << pt.y << " , " << pt.z;

    glm::vec3 ptTop, ptBottom;
    topRayBundles = raytraceRayBundleIrisToFront(glm::vec4(pt, 1.0f), 10);
    intersectionRayBundle(topRayBundles, ptTop, mIdxApertureStop + 1);

    qDebug() << "img top : " << ptTop.x << " , " << ptTop.y << " , " << ptTop.z;

    pt.y = -pt.y;
    bottomRayBundles = raytraceRayBundleIrisToFront(glm::vec4(pt, 1.0f), 10);
    intersectionRayBundle(bottomRayBundles, ptBottom, mIdxApertureStop + 1);

    zPos = ptTop.z;
    aperture = std::abs(ptTop.y - ptBottom.y);

    qDebug() << "bottom : " << pt.x << " , " << pt.y << " , " << pt.z;
    qDebug() << "img bottom : " << ptBottom.x << " , " << ptBottom.y << " , " << ptBottom.z;

    qDebug() << "Entrance Pupil z : " << zPos;
    qDebug() << "Entrance Pupil size : " << aperture;
}

glm::vec3 Lens::findFrontFocus(RayBundle &bundle, double* zPrincipalPlane)
{
    glm::vec3 focusPoint;
    bundle = imagePoint(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), focusPoint);
    if(zPrincipalPlane) {
        // find the ray sequence that traverses the entire lens
        for(auto it = bundle.begin(); it != bundle.end(); it++) {
            if(it->size() > mElems.size()) { // found the ray sequence
                glm::vec3 pt;
                if(it->begin()->intersection(it->back(), pt) == Ray::RAY_INTERSECTION) {
                    *zPrincipalPlane = pt.z;
                    qDebug() << "object side principal plane : " << pt.x << ", " << pt.y << ", " << pt.z;
                }
                break;
            }
        }
    }
    return focusPoint;
}

glm::vec3 Lens::findBackFocus(RayBundle &bundle, double *zPrincipalPlane)
{
    glm::vec3 focusPoint;
    bundle = imagePoint(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f), focusPoint);
    if(zPrincipalPlane) {
        // find the ray sequence that traverses the entire lens
        for(auto it = bundle.begin(); it != bundle.end(); it++) {
            if(it->size() > mElems.size()) { // found the ray sequence
                glm::vec3 pt;
                if(it->begin()->intersection(it->back(), pt) == Ray::RAY_INTERSECTION) {
                    *zPrincipalPlane = pt.z;
                    qDebug() << "image side principal plane : " << pt.x << ", " << pt.y << ", " << pt.z;
                }
                break;
            }
        }
    }
    return focusPoint;
}

glm::vec3 Lens::findBackFocus(double *zPrincipalPlane)
{
    RayBundle bundle;
    return findBackFocus(bundle, zPrincipalPlane);
}

glm::vec3 Lens::findFrontFocus(double *zPrincipalPlane)
{
   RayBundle bundle;
   return findFrontFocus(bundle, zPrincipalPlane);
}

RayBundle Lens::imagePoint(const glm::vec4 &pt, glm::vec3 &ptImg) const
{
    RayBundle res;
    if(pt.z < 0) {
        res = raytraceRayBundleFrontToBack(pt, 10);
    } else {
        res = raytraceRayBundleBackToFront(pt, 10);
    }
    intersectionRayBundle(res, ptImg, mElems.size() + 1);

    return res;
}

// generate a set of points and find their image. Return the point pairs
std::vector<std::pair<glm::vec3, glm::vec3> > Lens::imagePlanarGridPoints1D(float z, float dist_pts, int halfN) const
{
    std::vector<std::pair<glm::vec3, glm::vec3>> grid_point_mapping;

    for(int i = -halfN; i <= halfN; i++) {
        glm::vec3 pt(0.0f, i * dist_pts, z);
        glm::vec3 ptImg;
        imagePoint(glm::vec4(pt, 1.0), ptImg);
        grid_point_mapping.push_back(std::make_pair(pt, ptImg));
    }
    return grid_point_mapping;
}

bool Lens::intersectionRayBundle(RayBundle &bundle, glm::vec3 &intersection_point, int nRayIntersections) const
{
    // find the image of the input point
    Ray extremeRays[2];
    int idx = 0;
    //for(auto it = bundle.begin(); it != bundle.end(); it++) {
    for(auto& e : bundle) {
        //qDebug() << "size : " << it->size();
        if(e.size() >= nRayIntersections) {
            if(idx == 0) {
                extremeRays[idx++] = e.back();
            } else { // only store the last extreme ray
                extremeRays[idx] = e.back();
            }
        }
    }
    if(idx != 0) {
//        qDebug() << "[0] " << extremeRays[0].origin().x << " , " << extremeRays[0].origin().y << " , " << extremeRays[0].origin().z;
//        qDebug() << "[0] " << extremeRays[0].direction().x << " , " << extremeRays[0].direction().y << " , " << extremeRays[0].direction().z;
//        qDebug() << "[1] " << extremeRays[1].origin().x << " , " << extremeRays[1].origin().y << " , " << extremeRays[1].origin().z;
//        qDebug() << "[1] " << extremeRays[1].direction().x << " , " << extremeRays[1].direction().y << " , " << extremeRays[1].direction().z;
        if(extremeRays[0].intersection(extremeRays[1], intersection_point) == Ray::RAY_INTERSECTION) {
           //for(auto it = bundle.begin(); it != bundle.end(); it++) {
            for(auto& e: bundle) {
               if(e.size() >= nRayIntersections) {
                   Ray cLast = e.back();
                   // check if the final image point is in the same direction as the ray
                   glm::vec3 dir = intersection_point - cLast.origin();
                   glm::vec3 n = glm::cross(dir, cLast.direction());
                   qDebug() << glm::dot(n, n);
                   //assert(std::abs(glm::dot(n, n)) < std::numeric_limits<float>::epsilon());
                   e.push_back(Ray(intersection_point, glm::vec3(0.0f)));
                   e.push_back(cLast);
               }
           }
           return true;
        }
    }
    return false;
}

template<typename InputIt>
RayPath Lens::raytraceEndToEnd(const Ray &ray_orig, const InputIt& begin, const InputIt& end, RayTraceDirection dir) const
{
    RayPath raypath;
    Ray ray = ray_orig;
    raypath.push_back(ray);
    for(auto it = begin; it != end; it++) {
        glm::vec3 surface_pos;
        glm::vec3 surface_normal;
        if(it->intersection(ray, surface_pos, surface_normal)) {
            if(it->isAperture()) {// if intersects the aperture
                // this is considered as going through the opening of the aperture-stop
                // so only update the origin of the ray
                ray.setOrigin(surface_pos);
            } else { // otherwise compute the direction of the new ray
                double nd_curr = 1.0;
                double nd_next = 1.0; // air

                // interpret the index of refraction based on which direction the ray is traced
                // the nd in the lens prescription corresponds to the next medium
                // for front to back this needs to be r
                if(dir == RayTraceDirection::FRONT_TO_BACK) {
                    nd_next = it->nd();
                    if(it != begin) {
                        nd_curr = (it - 1)->nd();
                    }
                } else {
                    nd_curr = it->nd();
                    if((it + 1) != end) { // if there are more elements
                        nd_next = (it + 1)->nd(); // get the refractive index of the next medium
                    }
                }

                bool bTIR = false;
                glm::vec3 transmitted_ray = computeRefraction(ray.direction(), surface_normal, nd_curr, nd_next, &bTIR);
                //qDebug() << bTIR ;
                if(bTIR) {
                    qDebug() << ray.direction().z << " " << ray.direction().y;
                    qDebug() << surface_normal.z << " " << surface_normal.y;
                    qDebug() << transmitted_ray.z << " " << transmitted_ray.y;
                    qDebug() << "nd_curr " << nd_curr;
                    qDebug() << "nd_next " << nd_next;
                }
                ray.setParams(surface_pos, transmitted_ray); // update the ray
            }
            raypath.push_back(ray);
        } else { // ray is blocked i.e. falls outside the lens element's aperture
            if(it->isAperture()) { // blocked by the aperture-stop
                ray.setParams(surface_pos, glm::vec3(0.0f, 0.0f, 0.0f)); // this will draw the last ray appropriately
                raypath.push_back(ray);
            }
            break;
        }
    }
    return raypath;
}

RayPath Lens::raytraceFrontToBack(const Ray &ray) const
{
    return raytraceEndToEnd(ray, mElems.cbegin(), mElems.cend(), RayTraceDirection::FRONT_TO_BACK);
}

RayPath Lens::raytraceIrisToBack(const Ray &ray) const
{
    return raytraceEndToEnd(ray, mElems.cbegin() + mIdxApertureStop + 1, mElems.cend(), RayTraceDirection::FRONT_TO_BACK);
}

RayPath Lens::raytraceIrisToFront(const Ray &ray) const
{
    // find the index in the reverse iterator
    int reverseIdxAperture = static_cast<int>(mElems.size()) - mIdxApertureStop - 1;
    return raytraceEndToEnd(ray, mElems.crbegin() + reverseIdxAperture + 1, mElems.crend(), RayTraceDirection::BACK_TO_FRONT);
}

RayPath Lens::raytraceBackToFront(const Ray &ray) const
{
    return raytraceEndToEnd(ray, mElems.crbegin(), mElems.crend(), RayTraceDirection::BACK_TO_FRONT);
}

RayBundle Lens::raytraceRayBundleBackToFront(const glm::vec4 &pt, int nRays) const
{
    return raytraceRayBundleEndToEnd(pt, static_cast<int>(mElems.size()) - 1, &Lens::raytraceBackToFront, nRays);
}

RayBundle Lens::raytraceRayBundleFrontToBack(const glm::vec4 &pt, int nRays) const
{
    return raytraceRayBundleEndToEnd(pt, 0, &Lens::raytraceFrontToBack, nRays);
}

RayBundle Lens::raytraceRayBundleIrisToFront(const glm::vec4 &pt, int nRays) const
{
    // By default assume the point is at infinity
    glm::vec4 pt_ap = pt;

    // if the point is not at infinity
    if(std::fabs(pt.w) > std::numeric_limits<float>::epsilon()) {
        // make sure that the point is on the aperture stop i.e. z = aperturestop.z
        pt_ap.z = mElems[mIdxApertureStop].edgePos().z;
    }
    // NOTE: The second argument is the index of the lens element that should be sampled
    // Since the ray is going from the iris to the front, it's the element before the aperture stop.
    // raytraceIrisToFront will automatically start raytracing from the aperture surface index in the reverse iterator.
    return raytraceRayBundleEndToEnd(pt_ap, mIdxApertureStop - 1, &Lens::raytraceIrisToFront, nRays);
}

RayBundle Lens::raytraceRayBundleIrisToBack(const glm::vec4 &pt, int nRays) const
{
    // By default assume the point is at infinity
    glm::vec4 pt_ap = pt;

    // if the point is not at infinity
    if(std::fabs(pt.w) > std::numeric_limits<float>::epsilon()) {
        // make sure that the point is on the aperture stop i.e. z = aperturestop.z
        pt_ap.z = mElems[mIdxApertureStop].edgePos().z;
    }
    // NOTE: The second argument is the index of the lens element that should be sampled
    // raytraceIrisToBack will take care of the iterator positioning.
    return raytraceRayBundleEndToEnd(pt_ap, mIdxApertureStop + 1, &Lens::raytraceIrisToBack, nRays);
}

RayBundle Lens::raytraceRayBundleEndToEnd(const glm::vec4 &pt, int idxSurface, RayPath (Lens::*fnPtr) (const Ray &) const, int nRays) const
{
    RayBundle rayBundle;
    if(mElems.size() == 0)
        return rayBundle;
    std::vector<glm::vec3> samplesOnSurface = mElems[idxSurface].samplesYZPlaneIntersection(nRays);
    if(std::fabs(pt.w) < std::numeric_limits<float>::epsilon() ) { // if the point is at infinity
        for(auto it = samplesOnSurface.begin(); it != samplesOnSurface.end(); it++) {
            glm::vec3 orig((*it).x + pt.x, (*it).y + pt.y, (*it).z + pt.z);
            glm::vec3 dir(-pt.x, -pt.y, -pt.z);
            rayBundle.push_back( (this->*fnPtr)( Ray(orig, dir) ) );
        }
    } else {
        glm::vec3 orig(pt.x/pt.w, pt.y/pt.w, pt.z/pt.w);
        for(auto it = samplesOnSurface.begin(); it != samplesOnSurface.end(); it++) {
            rayBundle.push_back( (this->*fnPtr)( Ray(orig, (*it) - orig) ) );
        }
    }

    return rayBundle;
}

std::vector<glm::vec3> Lens::get2DSamplesOnRearSurface(int nSamples) const
{
    if(mElems.size() == 0)
        return std::vector<glm::vec3>();
    return mElems.back().samplesYZPlaneIntersection(nSamples);
}

//void Lens::computeThickness()
//{
//    mLensThickness = 0.0;
//    std::for_each(mElems.cbegin(), mElems.cend(), [this] (const LensElement &e) { this->mLensThickness += e.thickness(); });
//}

bool LensElement::intersectionSphericalLens(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const
{
    using namespace glm;
    pos = vec3(0.0);
    normal = vec3(0.0);

    const vec3 center = mCenter - ray.origin();
    const vec3 dir = ray.direction();

    float a = glm::dot(dir, dir);
    float b = -2.0f * glm::dot(dir, center);
    float c = static_cast<float>(dot(center, center) - SQR(mRadius));

    float det = SQR(b) - 4 * a * c;
    if(glm::abs(det) < 1e-9) det = 0.0f;
    if(det < 0) return false;

    float sqdet = glm::sqrt(det);
    float t1 = (-b - sqdet) / (2 * a);
    float t2 = (-b + sqdet) / (2 * a);

    if(t1 < EPS_DISTANCE && t2 < EPS_DISTANCE)
        return false;

    // at this point at least one of them is valid
    // also note t1 < t2
    // we now need to first find the intersection point corresponding to the shape of the lens surface
    // and then check if the intersection point is within the lens aperture.
    if(glm::abs(det) < 1e-9 || t1 < EPS_DISTANCE) {
        // if both points are the same or the second one is valid
        // only t2 is a valid intersection with sphere (because t1 < t2)
        pos = ray.origin() + ray.direction() * t2;
        normal = surfaceNormalAtIntersection(ray, pos);
        return isValidIntersection(pos);
    } else { // both points are valid sphere intersection
        pos = ray.origin() + ray.direction() * t1;
        normal = surfaceNormalAtIntersection(ray, pos);
        if(isValidIntersection(pos))
            return true;
        pos = ray.origin() + ray.direction() * t2;
        normal = surfaceNormalAtIntersection(ray, pos);
        if(isValidIntersection(pos))
            return true;
    }

    return false;
}

std::vector<glm::vec3> LensElement::samplesYZPlaneIntersection(int nSamples) const
{
    std::vector<glm::vec3> samples;

    const glm::vec3 pos = edgePos();
    double half_angle = asin(aperture() / (2.0 * radius()));
    double part = half_angle / nSamples; //twicePi / slices;

    for (int i = -nSamples; i <= nSamples; i++) {
        samples.push_back(glm::vec3(pos.x, pos.y + (-radius() * sin(i * part)), pos.z + radius() + (-radius() * cos(i *  part))));
    }
    return samples;
}


bool LensElement::intersectionAperture(const Ray &ray, glm::vec3 &pos, glm::vec3 &normal) const
{
    Plane plane(mCenter, glm::vec3(0, 0, ray.origin().z - mCenter.z));
    plane.intersection(ray, pos, normal);
    glm::vec3 u = pos - mCenter;
    return !(glm::dot(u, u) > SQR(mAperture/2.0));
}

bool LensElement::intersection(const Ray &ray, glm::vec3 &pos, glm::vec3 &normal) const
{
    if(isAperture())
        return intersectionAperture(ray, pos, normal);
    return intersectionSphericalLens(ray, pos, normal);
}

bool LensElement::isValidIntersection(const glm::vec3 &intersect_pos) const
{
//    if(SGN(radius) == SGN(post2.x))
//        return false;
    glm::vec3 u = intersect_pos - mCenter;

    //double max_costheta = std::sqrt(SQR(mRadius) - SQR(mAperture/2.0)) / std::fabs(mRadius);
    double max_apdistSqr = SQR(mRadius) - SQR(mAperture/2.0);
    //glm::vec3 dirV(0.0f, 0.0f, SGN(mRadius)); // center to edge direction vector

    // normal and the direction vector should point in the same direction i.e. same sign
    // if radius < 0 then direction towards the edge is positive otherwise it's negative
    if(SGN(u.z) != SGN(-mRadius))
        return false;

    if(SQR(u.z) < max_apdistSqr) // distance on the optical axis from the center to the intersection point
        return false;

    return true;
}

glm::vec3 LensElement::surfaceNormalAtIntersection(const Ray &ray, const glm::vec3 &intersection) const
{
    glm::vec3 u = -ray.direction();
    //glm::vec3 v = glm::vec3(0.0f, 0.0f, SGN(mRadius));

    glm::vec3 normal = glm::normalize(intersection - mCenter);
    if(SGN(u.z) != SGN(mRadius)) // ray originating from the opposite direction as the center (same as u.v < 0)
        return normal;
    return -normal; // otherwise need to invert the normal

}
