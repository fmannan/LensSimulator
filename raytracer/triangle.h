#pragma once
#include "shape.h"
#include <glm.hpp>
#include <vector>
class Triangle : public Shape
{
	glm::vec3 v[3];
	glm::vec3 mNormal;

	void computeNormal();	// normal needs to be computed only when the triangle is created
public:
	Triangle(void);
	Triangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);
	Triangle(const glm::vec3* v);
	Triangle(const std::vector<glm::vec3>& v);
	~Triangle(void);

	glm::vec3 normal() const;
    bool intersection(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const override;
};

