#include "triangle.h"
#include "utility.h"

using namespace glm;
Triangle::Triangle(void)
{
	v[0] = v[1] = v[2] = glm::vec3(0.0f);
}

Triangle::Triangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
	computeNormal();
}

Triangle::Triangle(const glm::vec3* v)
{
	this->v[0] = v[0];
	this->v[1] = v[1];
	this->v[2] = v[2];
	computeNormal();
}

Triangle::Triangle(const std::vector<glm::vec3>& v)
{
	this->v[0] = v[0];
	this->v[1] = v[1];
	this->v[2] = v[2];
	computeNormal();
}

Triangle::~Triangle(void)
{
}

void Triangle::computeNormal()
{
	vec3 v01 = v[1] - v[0];
	vec3 v02 = v[2] - v[0];

	mNormal = normalize(cross(v01, v02));
}

glm::vec3 Triangle::normal() const
{
	return mNormal;
}

bool Triangle::intersection(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const
{
	float t = dot(mNormal, v[0] - ray.origin()) / dot(mNormal, ray.direction());
	pos = ray.origin() + ray.direction() * t;
	normal = mNormal;
	if(t < EPS_DISTANCE) return false;

	// check if point is inside or not
	vec3 v01 = v[1] - v[0];
	vec3 v12 = v[2] - v[1];
	vec3 v20 = v[0] - v[2];
	
	int s1 = SGN(cross(v01, pos - v[0]).z);
	int s2 = SGN(cross(v12, pos - v[1]).z);
	int s3 = SGN(cross(v20, pos - v[2]).z);
	if(s1 == s2  && s2 == s3)  return true;
	if(s1 == 0) return s2 == s3;
	if(s2 == 0) return s1 == s3;
	if(s3 == 0) return s1 == s2;
	return false;
}
