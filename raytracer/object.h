#pragma once
#include "shape.h"
#include "shadinginfo.h"

class Object
{
public:
	Object(void);
	Object(Shape* shape);
	Object(Shape* shape, const Material& mat);
	~Object(void);

	void setParams(Shape* shape, const Material& material);
	bool intersection(const Ray& ray, double &dist, ShadingInfo& shade) const;
private:
	Shape *mShape;
	Material mMaterial;
};

