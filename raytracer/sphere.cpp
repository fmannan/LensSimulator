#include <cmath>
#include "glm.hpp"
#include "sphere.h"
#include "utility.h"

using namespace glm;

Sphere::Sphere(void) :
    mCenter(0.0), mRadius(0.0)
{
}

Sphere::Sphere(const vec3 &center, double radius) :
    mCenter(center), mRadius(radius)
{

}

Sphere::Sphere(vec3&& center, double radius) : mCenter(center), mRadius(radius) {}

Sphere::~Sphere(void)
{
}

bool Sphere::intersection(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const
{
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
	float t = t1;
	if(t < EPS_DISTANCE) {
		t = t2;
	}
	if(t < EPS_DISTANCE) return false;
	
	pos = ray.origin() + ray.direction() * t;
	normal = glm::normalize(pos - mCenter);

	return true;
}
