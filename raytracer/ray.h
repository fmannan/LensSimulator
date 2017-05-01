#pragma once
#include <glm.hpp>
#include <vector>
#include <string>
class Ray
{
	glm::vec3 mOrigin;
	glm::vec3 mDirection;

public:
    enum RayIntersectionType { RAY_NO_INTERSECTION = 0, RAY_NO_INTERSECTION_ON_SAME_PLANE, RAY_INTERSECTION, RAY_OVERLAP };

	Ray(void);
	Ray(const glm::vec3& origin, const glm::vec3& dir);
	Ray(glm::vec3&& origin, glm::vec3&& dir);

	const glm::vec3& origin() const { return mOrigin; }
	const glm::vec3& direction() const { return mDirection; }
    void setParams(const glm::vec3& origin, const glm::vec3& direction)
    {
        mOrigin = origin;
        mDirection = glm::normalize(direction);
    }

    void setOrigin(const glm::vec3& origin) { mOrigin = origin; }
    void setDirection(const glm::vec3& direction) { mDirection = direction; }
    RayIntersectionType intersection(const Ray& rhs, glm::vec3& intersection_point) const;

    std::string toString()
    {
        return "";
    }
};

typedef std::vector<Ray> RayPath;
typedef std::vector<RayPath> RayBundle;
