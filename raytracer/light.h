#pragma once
#include "shadinginfo.h"
#include <glm.hpp>

class Light
{
public:
	enum LightType { POINT, DIRECTIONAL };
	Light(void);
	Light(const glm::vec3& pos, const Colorf& color = Colorf(1.0, 1.0, 1.0), LightType type = LightType::POINT);
	Light(const glm::vec3& pos, const Colorf& color, const float attenuation[3], LightType type = LightType::POINT);
	//Light(glm::vec3&& pos, Color&& color = Color(255, 255, 255));
	~Light(void);

	LightType type() const { return mType; }
	//const glm::vec3 position() const { return mPos; }
	const glm::vec3 direction(const glm::vec3& surface_pos) const 
	{
		if(mType == DIRECTIONAL)
			return mPos;
		return glm::normalize(mPos - surface_pos);
	}
	const Colorf color() const { return mColor; }
	const float* attenuation_components() const { return mAttenuation; }
	double attenuation(double distance) const;
	double attenuation(const glm::vec3& pos) const;
	double angular_attenuation(const glm::vec3& surface_normal, const glm::vec3& surface_pos) const;
private:
	glm::vec3 mPos;
	Colorf mColor;
	float mAttenuation[3];
	LightType mType;
};
