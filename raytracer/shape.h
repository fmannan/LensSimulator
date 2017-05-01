#pragma once
#include "glm.hpp"
#include "ray.h"

class Shape
{
public:
	Shape(void);
	virtual ~Shape(void);

	virtual bool intersection(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const = 0;
};
