#include "camera.h"
#include <cmath>
#include "utility.h"

using namespace glm;

Camera::Camera(void)
{
}

Camera::Camera(Camera&& cam) 
{
	setParams(cam.mPos, cam.mLookAt, cam.mUp, cam.mFovy);
}

Camera::Camera(glm::vec3&& pos, glm::vec3&& lookAt, glm::vec3&& up, double fovy)
{
	setParams(pos, lookAt, up, fovy);
}

Camera::~Camera(void)
{
}

void Camera::setParams(glm::vec3&& pos, glm::vec3&& lookAt, glm::vec3&& up, double fovy)
{
	this->mPos = pos;
	this->mLookAt = lookAt;
	this->mUp = up;
	this->mFovy = fovy;
}

void Camera::setParams(const glm::vec3& pos, const glm::vec3& lookAt, const glm::vec3& up, double fovy)
{
	this->mPos = pos;
	this->mLookAt = lookAt;
	this->mUp = up;
	this->mFovy = fovy;
}

double Camera::getSensorDist()
{
	double fovyrad = deg2rad(mFovy);
	return 1.0 / tan(fovyrad / 2.0);
}

glm::mat4 Camera::getViewMatrix()
{
	return mat4(1.0);
}
