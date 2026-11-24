#include "plane.h"
#include <utility.h>
#include <glm.hpp>
Plane::Plane(const glm::vec3& vertex, const glm::vec3& normal) :
    mV(vertex), mNormal(normal)
{
}

glm::vec3 Plane::normal() const
{
    return mNormal;
}

glm::vec3 Plane::vertex() const
{
    return mV;
}

bool Plane::intersection(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const
{
    using namespace glm;
    // TODO: TEST CODE
    float denominator = dot(mNormal, ray.direction());
    if (std::abs(denominator) < 1e-12f) return false;
    float t = dot(mNormal, mV - ray.origin()) / denominator;
    pos = ray.origin() + ray.direction() * t;
    normal = mNormal;
    if(t < EPS_DISTANCE) return false;

    return true;
}
