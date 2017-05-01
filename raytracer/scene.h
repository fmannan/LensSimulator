#pragma once
#include "camera.h"
#include "shadinginfo.h"
#include "shape.h"
#include "object.h"
#include "light.h"
#include <string>
#include <vector>
#include <memory>
class Scene
{
public:
	Scene(void);
	Scene(Scene&& scene);
	~Scene(void);

	/*void addCamera(std::shared_ptr<Camera> camera);
	void addObject(std::shared_ptr<Object> obj);
	void addLight(std::shared_ptr<Light> light);*/
	void addCamera(Camera* camera);
	void addObject(Object* obj);
	void addLight(Light* light);
	void setCanvasSize(int width, int height);
	void setMaxDepth(int depth) { mMaxDepth = depth; }
	void setOutputFilename(const std::string& filename) { mOutputFilename = filename; }

	Camera* getCamera() const;
	int width() const { return mWidth; }
	int height() const { return mHeight; }
	int depth() const { return mMaxDepth; }
	const std::string& getOutputFilename() const { return mOutputFilename; }

	ShadingInfo getFirstIntersection(const Ray& ray) const;	// get the shading info from the object ray intersection position.
	std::vector<const Light*> getVisibleLights(const glm::vec3& pos) const; // get the color of visible light sources from the given position
private:
	//std::shared_ptr<Camera> mCamera; // TODO: maybe use unique_ptr ?
	//std::vector<std::shared_ptr<Object>> mObjs;
	//std::vector<std::shared_ptr<Light>> mLights;
	Camera* mCamera; // TODO: maybe use unique_ptr ?
	std::vector<Object*> mObjs;
	std::vector<Light*> mLights;
	int mWidth, mHeight;
	int mMaxDepth;
	std::string mOutputFilename;
};

