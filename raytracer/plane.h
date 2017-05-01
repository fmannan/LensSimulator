#ifndef PLANE_H
#define PLANE_H

#include "shape.h"
#include <glm.hpp>

class Plane : public Shape
{
    glm::vec3 mV;
    glm::vec3 mNormal;
public:
    Plane(const glm::vec3& vertex, const glm::vec3& normal);

    glm::vec3 normal() const;
    glm::vec3 vertex() const;
    bool intersection(const Ray& ray, glm::vec3& pos, glm::vec3& normal) const override;
};

#endif // PLANE_H
