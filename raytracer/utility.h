#pragma once
#include <cmath>
#include <limits>
const double PI = 2.0 * acos(0.0);
const double EPS_DISTANCE = 1e-5;

inline double deg2rad(double deg) 
{
	return deg * PI / 180.0;
}

template<typename T>
inline T SQR(T x) { return x * x; }

inline int SGN(double x) 
{
	static const double SGN_EPS = 1e-6;
	if(x < -SGN_EPS) return -1;
	else if( x > SGN_EPS) return 1;
	return 0;
}

inline glm::vec3 computeReflection(const glm::vec3& u, const glm::vec3& normal)
{
    // u is incident ray
    return 2.0f * glm::dot(-u, normal) * normal + u;
}

inline glm::vec3 computeRefraction(const glm::vec3& u, const glm::vec3& normal, double nu, double nv, bool *bTIR = 0)
{
    glm::vec3 upar = u - glm::dot(u, normal) * normal; // interface-parallel component of incident ray
    glm::vec3 vpar = static_cast<float>(nu / nv) * upar;
    double sinth2 = glm::dot(vpar, vpar);

    if(sinth2 > 1) { // total internal reflection
        if(bTIR)
            *bTIR = true; // Total internal reflection
        return computeReflection(u, normal);
    }
    if(bTIR)
        *bTIR = false;
    float cosv = static_cast<float> (glm::sqrt(1 - sinth2));
    return -cosv * normal + vpar;
}

inline bool is_nan(double x)
{
    return x != x;
}
