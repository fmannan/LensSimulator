#ifndef LENS_H
#define LENS_H
#include <vector>
#include <QString>
#include <ray.h>
#include <plane.h>
#include <glm.hpp>
#include <object.h>
#include <utility.h>
#include <cmath>
#include <stdexcept>

class LensElement
{
    // A LensElement is part of a Lens. The coordinate of the lens element is w.r.t. the lens that contains it.
    // This approach is taken to reduce the number of transformations on the ray.
    // The ray is transformed once it enters the lens system.
    // The profile of the lens is on the z-y plane with x going into the display. Therefore, the thickness is along
    // the z-axis (positive to the right). If radius is positive then the center is to the right.
    // aperture stop is represented either by NaN or Inf (Inf is more natural)
    // Setting the coordinate of the lens center: see update center
private:
    double mRadius; // can be pos or neg. NaN for aperture
    double mThickness; // thickness is the distance from the current lens surface to the next one
    double mNd;
    double mVno;
    double mAperture;
    float mOffsetLens; // offset w.r.t to the lens system
    glm::vec3 mEdgePos; // position of the surface point that intersects the optical axis. Defined with respect to the lens system coordinate
    glm::vec3 mCenter;  // center of the lens-element w.r.t to the lens system

    void updateEdgeAndCenter()
    {
        mEdgePos = glm::vec3(0.0f, 0.0f, mOffsetLens);
        if(std::isnan(mRadius)) {
            mCenter = mEdgePos;
        } else {
//            // if radius is negative then the lens is concave and the thickness is in the negative direction
//            float offset = mRadius < 0.0 ? mThickness : 0.0;
//            mEdgePos.z = mEdgePos.z + offset;
            mCenter = mEdgePos + glm::vec3(0.0f, 0.0f, mRadius);
        }
    }

public:

    LensElement(const double params[5], float offset_lens) :
        mThickness(params[1]), mVno(params[3]), mAperture(params[4]), mOffsetLens(offset_lens)
    {
        setRadius(params[0]);
        setNd(params[2]);
        updateEdgeAndCenter();
    }

    LensElement(const std::vector<double> params, float offset_lens) :
        mOffsetLens(offset_lens)
    {
        setParams(params); // set params already calls updateEdgeAndCenter
    }

    void setParams(const std::vector<double> params)
    {
        if(params.size() != 5)
            throw std::runtime_error("LensElement requires 5 arguments");

        mThickness = params[1];
        setRadius(params[0]);
        setNd(params[2]);
        mVno = params[3];
        mAperture = params[4];
        updateEdgeAndCenter();
    }
    double radius()     const { return mRadius; }
    double thickness()  const { return mThickness; }
    double nd()         const { return mNd; }
    double Vno()        const { return mVno; }
    double aperture()   const { return mAperture; }
    glm::vec3 edgePos() const { return mEdgePos; }
    glm::vec3 center()  const { return mCenter; }

    void setRadius(double radius)
    {
        mRadius = radius;
        updateEdgeAndCenter();
    }

    void setNd(double index_of_refraction)
    {
        mNd = is_nan(index_of_refraction) ? 1.0 : index_of_refraction;
        if(mNd < 0.0)
            throw std::runtime_error("Index of refraction cannot be negative");
    }

    bool isAperture()   const { return (mRadius != mRadius) /*|| std::isinf(mRadius)*/; } // radius is set to nan for aperture
    bool isMediumAir()  const { return std::fabs(mNd) < std::numeric_limits<double>::epsilon(); }
    bool intersection(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const;
    bool intersectionAperture(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const;
    bool intersectionSphericalLens(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const;
    std::vector<glm::vec3> samplesYZPlaneIntersection(int nSamples = 10) const; // uniform samples on the YZ plane
    std::vector<glm::vec3> samplesPlaneIntersection(const Plane& plane, int nSamples = 10) const;     // uniform samples on a plane
    std::vector<glm::vec3> samplesSurface(const Plane& plane, int nSamples = 100) const;  // uniform samples on a surface
    bool isValidIntersection(const glm::vec3& intersect_pos) const; // check if intersects within the specified aperture
    glm::vec3 surfaceNormalAtIntersection(const Ray& ray, const glm::vec3& intersection) const; // normal is determined based on which direction the ray is coming from
};

typedef std::vector<LensElement> LensPrescription;

class Lens
{
    // The center of the surface of the first element is at (0, 0, 0) in the lens's coordinate system
    // The cross-section is the x-y plane and the thickness is along the x-axis, up is the y-axis.
public:
    Lens();
    Lens(const QString& filename);

    void load(const QString& filename);
    //void add(LensElement elem);

    void findExitPupil(double& z, double& aperture);    // returns position and size of the exit pupil
    void findExitPupil(double &zPos, double& aperture, RayBundle& topRayBundles, RayBundle& bottomRayBundles);

    void findEntrancePupil(double& z, double& aperture); // returns position and size of the entrance pupil
    void findEntrancePupil(double &zPos, double& aperture, RayBundle& topRayBundles, RayBundle& bottomRayBundles);

    glm::vec3 findFrontFocus(double* zPrinciplePlane = 0);    // focus on the object side
    glm::vec3 findFrontFocus(RayBundle& bundle, double* zPrinciplePlane = 0);

    glm::vec3 findBackFocus(double* zPrinciplePlane = 0);     // focus on the image side
    glm::vec3 findBackFocus(RayBundle& bundle, double* zPrinciplePlane = 0);

    // experimental no-parallax point finding procedure where the intersection between the scene point and image is found
    std::vector<glm::vec3> findNoParallaxPoint() const;

    // image of a point through the lens
    // This function automatically determines the order of ray tracing assuming z is the optical axis
    // and the sensor is in the positive z direction, object -z direction
    RayBundle imagePoint(const glm::vec4& pt, glm::vec3& ptImg) const;
    std::vector<std::pair<glm::vec3, glm::vec3>> imagePlanarGridPoints1D(float z, float dist_pts, int halfN) const;
    bool intersectionRayBundle(RayBundle& bundle, glm::vec3& intersection_point, int nRayIntersections) const;

    RayPath raytrace(const Ray& ray) const;
    RayPath raytraceApertureStopToBack(const Ray& ray); // ray originates on the aperture plane
    RayPath raytraceBackToFront(const Ray& ray) const;  // the ray hits the rear surface first (i.e. right-most near the sensor)
    RayPath raytraceFrontToBack(const Ray& ray) const;  // the ray hits the front surface first
    RayPath raytraceIrisToBack(const Ray& ray) const;  // the ray hits the first surface after the aperture stop
    RayPath raytraceIrisToFront(const Ray& ray) const;  // the ray hits the last surface before the aperture stop

    RayBundle raytraceRayBundleBackToFront(const glm::vec4& pt, int nRays = 7) const; // trace a bundle from rays from the given point to the front of the lens
    RayBundle raytraceRayBundleFrontToBack(const glm::vec4& pt, int nRays = 7) const; // trace a bundle from rays from the given point to the back of the lens
    RayBundle raytraceRayBundleIrisToFront(const glm::vec4& pt, int nRays = 7) const; // trace a bundle from rays from the given point on the iris to the front of the lens
    RayBundle raytraceRayBundleIrisToBack(const glm::vec4& pt, int nRays = 7) const; // trace a bundle from rays from the given point on the iris to the back of the lens

    std::vector<glm::vec3> getSamplesOnRearSurface() const;
    std::vector<glm::vec3> get2DSamplesOnRearSurface(int nSamples = 10) const;
    double thickness() const { return mLensThickness; }   // total thickness of the lens (end to end distance)
    const LensPrescription::const_iterator cbegin() const { return mElems.cbegin(); }
    const LensPrescription::const_iterator cend() const { return mElems.cend(); }
private:
    LensPrescription mOrigElems;    // for keeping a copy of the original
    LensPrescription mElems;
    int mIdxApertureStop;       // store the index of the aperture stop (NOTE: This assumes a single aperture stop)

    double mLensThickness; // total thickness of the lens
    enum RayTraceDirection { BACK_TO_FRONT = -1, FRONT_TO_BACK = 1 };

    void init();           // clean up all data
    //void computeThickness();

    //RayPath raytraceEndToEnd(const Ray& ray, const LensPrescription::const_iterator& begin, const LensPrescription::const_iterator& end) const;
    template<typename InputIt>
    RayPath raytraceEndToEnd(const Ray& ray, const InputIt& begin, const InputIt& end, RayTraceDirection dir) const;
    RayBundle raytraceRayBundleEndToEnd(const glm::vec4& pt, int idxSurface, RayPath (Lens::*fnPtr) (const Ray&) const, int nRays) const;
};

#endif // LENS_H
