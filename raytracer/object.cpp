#include "object.h"
#include <glm.hpp>

using namespace glm;

Object::Object(void) : mShape(0)
{
}

Object::Object(Shape *shape) : mShape(shape) {}

Object::Object(Shape* shape, const Material& mat)
{
	setParams(shape, mat);
}

Object::~Object(void)
{
	if(mShape)
		delete mShape;
	mShape = 0;
}

void Object::setParams(Shape* shape, const Material& material)
{
	this->mShape = shape;
	this->mMaterial = material;
}

bool Object::intersection(const Ray& ray, double& dist, ShadingInfo& shade) const
{
	vec3 pos(0.0);
	vec3 normal(0.0);
	if(mShape->intersection(ray, pos, normal)) {
		shade.pos = pos;
		shade.normal = normal;
		shade.mat = this->mMaterial;
		dist = glm::distance(ray.origin(), pos);
		return true;
	}
	return false;
}
