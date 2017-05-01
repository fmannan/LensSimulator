#pragma once
#include <glm.hpp>

class Camera
{
public:
	Camera(void);
	Camera(Camera&& camera);
	Camera(glm::vec3&& pos, glm::vec3&& mLookAt, glm::vec3&& mUp, double fovy);
	virtual ~Camera(void);

	void setParams(glm::vec3&& pos, glm::vec3&& mLookAt, glm::vec3&& mUp, double fovy);
	void setParams(const glm::vec3& pos, const glm::vec3& mLookAt, const glm::vec3& mUp, double fovy);
	double getSensorDist();
	glm::mat4 getViewMatrix();
private:
	glm::vec3 mPos;
	glm::vec3 mLookAt;
	glm::vec3 mUp;
	double mFovy;
};

