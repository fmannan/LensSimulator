#include "light.h"
#include <limits>
Light::Light(void) : mPos(0.0), mColor(0.0f), mType(LightType::POINT)
{
	mAttenuation[0] = 1.0;
	mAttenuation[1] = mAttenuation[2] = 0;
}

Light::Light(const glm::vec3& pos, const Colorf& color, LightType type) : mPos(pos), mColor(color), mType(type)
{
	mAttenuation[0] = 1.0;
	mAttenuation[1] = mAttenuation[2] = 0;
}

Light::Light(const glm::vec3& pos, const Colorf& color, const float attenuation[3], LightType type) : mPos(pos), mColor(color), mType(type)
{
	for(int i = 0; i < 3; i++)
		mAttenuation[i] = attenuation[i];
}

//Light::Light(glm::vec3&& pos, Color&& color) : mPos(pos) : mColor(color) {}

Light::~Light(void)
{
}

double Light::attenuation(double distance) const
{
	if(mType == LightType::DIRECTIONAL)
		return 1.0;
	double denom = mAttenuation[0] + mAttenuation[1] * distance + mAttenuation[2] * distance * distance;
	if(abs(denom) < std::numeric_limits<double>::epsilon())
		return 1.0;
	return 1.0 / denom;
}

double Light::attenuation(const glm::vec3& pos) const
{
	return attenuation(glm::distance(pos,mPos));
}

double Light::angular_attenuation(const glm::vec3& surface_normal, const glm::vec3& surface_pos) const
{
	using namespace glm;
	if(mType == DIRECTIONAL)
		return 1.0; // no attenuation
	return  glm::max(0.0f, dot(normalize(surface_normal), normalize(mPos - surface_pos)));
}
