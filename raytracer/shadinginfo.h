#pragma once
#include "glm.hpp"
#include <limits>

template<typename T>
class Color {
	/*union ColorT {
		unsigned char v[4];
		struct {
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} vals;
	} rgba;*/
	T rgba[4];
public:
	Color() 
	{
		rgba[0] = 0;
		rgba[1] = 0;
		rgba[2] = 0;
		rgba[3] = 0;
	}

	Color(T v) 
	{
		set(v, v, v, v);
	}

	Color(T r, T g, T b, T a = 0) 
	{
		set(r, g, b, a);
	}

	//Color(unsigned char rgba[4]) {}
	double L2sqr() const {
		double dist_sqr = 0;
		for(int i = 0; i < 3; i++) {
			dist_sqr += rgba[i] * rgba[i];
		}
		return dist_sqr;
	}

	bool nonZero() const 
	{
		return L2sqr() > DBL_EPSILON;
	}
	
	bool operator<(const Color& rhs) const {
		return L2sqr() < rhs.L2sqr();
	}

	bool operator>(const Color& rhs) const  {
		return L2sqr() > rhs.L2sqr();
	}

	Color operator+(const Color& c) const {
		Color newClr(rgba[0] + c.rgba[0], rgba[1] + c.rgba[1], rgba[2] + c.rgba[2], rgba[3] + c.rgba[3]);
		return newClr;
	}

	Color operator*(const Color& c) const {
		Color newClr(rgba[0] * c.rgba[0], rgba[1] * c.rgba[1], rgba[2] * c.rgba[2], rgba[3] * c.rgba[3]);
		return newClr;
	}

	Color operator*(const T& v) const {
		Color newClr(rgba[0] * v, rgba[1] * v, rgba[2] * v, rgba[3] * v);
		return newClr;
	}

	Color operator/(const Color& c) const {
		Color newClr;
		for(int i = 0; i < 3; i++) {
			newClr.rgba[i] = rgba[i];
			if(abs(c.rgba[i]) > FLT_EPSILON) // avoid division by 0	
				newClr.rgba[i] /= c.rgba[i];
		}
		return newClr;
	}

	Color& operator+=(const Color& right) {
		for(int i = 0; i < 4; i++) {
			rgba[i] += right.rgba[i];
		}
		return *this;
	}

	Color& operator*=(const Color& right) {
		for(int i = 0; i < 4; i++) {
			rgba[i] *= right.rgba[i];
		}
		return *this;
	}

	void set(T r, T g, T b, T a = 0) 
	{
		rgba[0] = r;
		rgba[1] = g;
		rgba[2] = b;
		rgba[3] = a;
	}
	const T* get() const { return rgba; }
    T red()     const { return rgba[0]; }
    T green()   const { return rgba[1]; }
    T blue()    const { return rgba[2]; }
    T alpha()   const { return rgba[3]; }
};

typedef Color<float> Colorf;

class Material {
public:
	Material() : diffuse(0.0f), ambient(0.0f), specular(0.0f), emission(0.0f), shininess(0.0f) {}
	Material(const Colorf& diffuse) : diffuse(diffuse), ambient(0.2f), specular(0.0f), emission(0.0f), shininess(0.0f) {}
	Material(const Colorf& diffuse, const Colorf& ambient, const Colorf& specular, const Colorf& emission, double shininess) : diffuse(diffuse), ambient(ambient), specular(specular), emission(emission), shininess(shininess) {}

	Colorf diffuse;
	Colorf ambient;
	Colorf specular;
	Colorf emission;
	double shininess;

};

class ShadingInfo
{
public:
	ShadingInfo(void) : normal(0.0f), pos(0.0), bValid(false) {}
	//~ShadingInfo(void) {}

	glm::vec3 normal;
	glm::vec3 pos;
	Material mat;
	bool bValid;
};

