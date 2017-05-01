#include "ray.h"

Ray::Ray(void) : mOrigin(0.0), mDirection(0.0)
{
}

Ray::Ray(const glm::vec3& origin, const glm::vec3& dir) : mOrigin(origin), mDirection(glm::normalize(dir)) {}

Ray::Ray(glm::vec3&& origin, glm::vec3&& dir) : mOrigin(origin), mDirection(glm::normalize(dir)) {}

Ray::RayIntersectionType Ray::intersection(const Ray& rhs, glm::vec3& intersection_point) const
{
    // graphics gems v1. p. 304
    // p + s u = q + t v
    // s u = (q - p) + t v
    // (u x v) s = (q - p) x v
    // s = ((q - p) x v) . (u x v) / ||u x v||^2
    // if ||u x v||^2 = 0 then rays are parallel
    // otherwise closest point on the two rays
    // <strike>The following implementation is a bit different
    // but is it correct? need to check</strike>
    //
//    glm::vec3 orig_dir = rhs.origin() - origin();
//    glm::vec3 n1 = glm::cross(direction(), orig_dir);
//    double vol = glm::dot(n1, rhs.direction());
//    if(std::abs(vol) > 0) { // the two rays are not on the same plane
//        return RAY_NO_INTERSECTION;
//    }
//    glm::vec3 n2 = glm::cross(direction(), rhs.direction());
//    if(std::abs(glm::dot(n2, n2)) < std::numeric_limits<float>::epsilon()) {// parallel lines
//        // check if they overlap
//        if(std::abs(glm::dot(n1, n1)) < std::numeric_limits<float>::epsilon()) {
//            intersection_point = origin();
//            return RAY_OVERLAP;
//        }
//        return RAY_NO_INTERSECTION;
//    }

//    // find intersection of two intersecting rays
//    float t1 = -glm::dot(orig_dir, n1) / glm::dot(n1, n2);
//    intersection_point = rhs.origin() + t1 * rhs.direction();
//    return RAY_INTERSECTION;
    glm::vec3 orig_dir = rhs.origin() - mOrigin;
    glm::vec3 n1 = glm::cross(mDirection, rhs.direction());
    glm::vec3 n2 = glm::cross(orig_dir, rhs.direction());
    float mag2_n1 = glm::dot(n1, n1);
    if(std::abs(mag2_n1) < std::numeric_limits<float>::epsilon()) { // parallel lines
        if(std::abs(glm::dot(n2, n2)) < std::numeric_limits<float>::epsilon()) {
            intersection_point = origin();
            return RAY_OVERLAP;
        }
        return RAY_NO_INTERSECTION;
    }
    float t0 = glm::dot(n2, n1) / mag2_n1;
    intersection_point = mOrigin + t0 * mDirection;
    if(std::abs(glm::dot(n2, mDirection)) > 0) {
        return RAY_NO_INTERSECTION_ON_SAME_PLANE;
    }
    return RAY_INTERSECTION;
}
