#include "scene.h"

using namespace std;

Scene::Scene(void) : mCamera(0), mWidth(0), mHeight(0)
{
}

Scene::Scene(Scene&& scene) 
{
	mCamera = scene.mCamera;
	mObjs = scene.mObjs;
	mLights = scene.mLights;
	mWidth = scene.mWidth;
	mHeight = scene.mHeight;
	mMaxDepth = scene.mMaxDepth;
	mOutputFilename = scene.mOutputFilename;

	scene.mCamera = nullptr;
}

Scene::~Scene(void)
{
	if(mCamera)
		delete mCamera;
	mCamera = 0;
	mObjs.clear();
	mLights.clear();
}

void Scene::addCamera(Camera* camera)
{
	this->mCamera = camera;
}

void Scene::addObject(Object* obj)
{
	mObjs.push_back(obj);
}

void Scene::addLight(Light* light)
{
	mLights.push_back(light);
}

void Scene::setCanvasSize(int width, int height)
{
	mWidth = width;
	mHeight = height;
}

Camera* Scene::getCamera() const
{
	return mCamera;
}

ShadingInfo Scene::getFirstIntersection(const Ray& ray) const
{
	const double MAX_DIST_VAL = 1000000000;
	double minDist = MAX_DIST_VAL;
	ShadingInfo minShader;
	for(vector<Object*>::const_iterator it = mObjs.cbegin(); it != mObjs.cend(); it++) {
		ShadingInfo shader;
		double dist = -1.0;
		const Object* obj = *it;
		if(obj->intersection(ray, dist, shader)) {
			if(dist < minDist) {
				minDist = dist;
				minShader = shader;
				minShader.bValid = true;
			}
		}
	}

	//if(minDist < MAX_DIST_VAL) {
	//	// calculate shadow
	//	for(auto it = mLights.cbegin(); it != mLights.end(); it++) {

	//	}
	//}
	return minShader;
}

vector<const Light*> Scene::getVisibleLights(const glm::vec3& pos) const
{
	//Considers both directional and positional light sources.
	//For directional light source shoot a ray in the corrsponding direction
	//For positional light source shoot a ray in the direction defined by the position on the surface and light source position
	// Light::direction(pos) takes care of all the direction computation
	vector<const Light*> lights;
	for(auto it = mLights.cbegin(); it != mLights.end(); it++) {
		Ray ray(pos, (*it)->direction(pos));
		ShadingInfo shade = getFirstIntersection(ray);
		if(!shade.bValid) { // if no intersection with objects
			lights.push_back(*it);
		}
	}
	return lights;
}
