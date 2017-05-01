#pragma once
#include "shape.h"
#include "glm.hpp"

class Sphere : 	public Shape
{
private:
	glm::vec3 mCenter;
	double mRadius;
public:
	Sphere(void);
    Sphere(const glm::vec3 &center, double radius);
	Sphere(glm::vec3&& center, double radius);
	~Sphere(void);

    bool intersection(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const override;
};

