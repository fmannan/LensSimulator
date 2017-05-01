#pragma once
#include "image.h"
#include "scene.h"
class Raytracer
{
	//int mWidth;
	//int mHeight;
	//int mMaxDepth;
	
	Colorf raytrace(const Scene& scene, Ray ray, int maxDepth); // trace a single ray through the scene
public:
	Raytracer(void);
	~Raytracer(void);

	Image* Render(const Scene& scene, int width, int height, int maxDepth);
	
};

