#include <vector>
#include <algorithm>
#include "shadinginfo.h"
#include "raytracer.h"
#include "ray.h"
#include "glm.hpp"
#include "utility.h"

using namespace glm;

Raytracer::Raytracer(void)
{
}


Raytracer::~Raytracer(void)
{
}

Image* Raytracer::Render(const Scene& scene, int width, int height, int maxDepth)
{
	Image *img = new Image(width, height);
	float aspect_ratio = width / (float)height;
	// transform the scene
	//...

	Camera* cam = scene.getCamera();
	double f = cam->getSensorDist();
	vec3 orig(0.0);
	for(int row = 0; row < height; row++) {
		for(int col = 0; col < width; col++) {
	/*{{
	int row = 143;
	int col = 113;*/
			vec3 pt((col *  2.0/width - 1.0) * aspect_ratio, row * 2.0/height - 1.0, -f);
			Ray ray(orig, pt - orig);
			Colorf finalClr = raytrace(scene, ray, maxDepth);
			//Colorf finalClr(0.0f);
			/*for(int dh = -1; dh <= 1; dh++) {
				for(int dw = -1; dw <= 1; dw++) {
					vec3 pt(((col + dw * 0.25f) *  2.0/width - 1.0) * aspect_ratio, (row + dh * 0.25f) * 2.0/height - 1.0, -f);

					Ray ray(orig, pt - orig);
					Colorf clr = raytrace(scene, ray, maxDepth);
					float weight = 0.05f;
					if(dh == 0 || dw == 0)
						weight = 0.1f;
					if(dh == 0 && dw == 0)
						weight = 0.4f;
					finalClr += clr * weight;
				}
			}*/
			img->set(row, col, finalClr.red(), finalClr.green(), finalClr.blue(), finalClr.alpha());
		}
	}
	return img;
}

Colorf Raytracer::raytrace(const Scene& scene, Ray orig_ray, int maxDepth)
{
	Colorf clr(0.0f);
	ShadingInfo prev_shader;
	Ray ray = orig_ray;
	Colorf specular_coeff(1.0f);
	for(int iter = 0; iter <= maxDepth; iter++) {
		ShadingInfo shader = scene.getFirstIntersection(ray);

		if(!(shader.bValid && specular_coeff.nonZero())) 
			return clr;
		
		// shoot shadow ray
		// should the shadow ray check be done in the Scene class?
		// doing it here allows for more flexible mixing of light perhaps?
		auto lights = scene.getVisibleLights(shader.pos);
			
		// compute color 
		clr += specular_coeff * (shader.mat.ambient + shader.mat.emission);
		if(lights.size() > 0) { // if the scene point is lit by at least a single light
			for(auto it = lights.begin(); it != lights.end(); it++) {
				const Light* light = *it;
				// compute specular
				vec3 u = light->direction(shader.pos); // direction towards the light source
				vec3 v = -ray.direction(); // negative of incoming ray direction
				vec3 H = glm::normalize(u + v); // half angle
				//vec3 R = computeReflection(u, shader.normal);
				float specular_attenuation = glm::pow(std::max(0.0f, glm::dot(H, shader.normal)), static_cast<float>(shader.mat.shininess));

				float angle_attenuation = static_cast<float>(light->angular_attenuation(shader.normal, shader.pos));
				clr += specular_coeff * (light->color() * static_cast<float>(light->attenuation(shader.pos)) * (shader.mat.diffuse *  angle_attenuation + shader.mat.specular * specular_attenuation));
			}
		}
		// compute new ray direction
		ray = Ray(prev_shader.pos, computeReflection(-ray.direction(), prev_shader.normal)); 
		prev_shader = shader;
		specular_coeff *= shader.mat.specular;
	}
	return clr;
}
//if(shader.bValid) {			
//	// shoot shadow ray
//	// should the shadow ray check be done in the Scene class?
//	// doing it here allows for more flexible mixing of light perhaps?
//	auto lights = scene.getVisibleLights(shader.pos);
//
//	// compute color 
//	clr = shader.mat.ambient + shader.mat.emission;
//	if(lights.size() > 0) { // if the scene point is lit by at least a single light
//		for(auto it = lights.begin(); it != lights.end(); it++) {
//			const Light* light = *it;
//			// compute specular
//			vec3 u = light->direction(shader.pos); // direction towards the light source
//			vec3 v = -ray.direction(); // negative of incoming ray direction
//			vec3 H = glm::normalize(u + v); // half angle
//			//vec3 R = computeReflection(u, shader.normal);
//			float specular_attenuation = glm::pow(std::max(0.0f, glm::dot(H, shader.normal)), static_cast<float>(shader.mat.shininess));

//			float angle_attenuation = static_cast<float>(light->angular_attenuation(shader.normal, shader.pos));
//			clr += light->color() * static_cast<float>(light->attenuation(shader.pos)) * (shader.mat.diffuse *  angle_attenuation + shader.mat.specular * specular_attenuation);
//		}
//	}
//	// trace reflected rays
//	ShadingInfo prev_shader = shader;
//	Colorf specular_coeff = shader.mat.specular;
//	for(int d = 1; d < maxDepth && specular_coeff.nonZero(); d++) { // loop until maxdepth is reached or specularity coefficient term becomes 0
//		vec3 u = -ray.direction();
//		vec3 v = computeReflection(u, prev_shader.normal); // 2.0f * dot(u, prev_shader.normal) * prev_shader.normal - u;
//		Ray reflected_ray(prev_shader.pos, v);
//		const ShadingInfo reflection_shader = scene.getFirstIntersection(reflected_ray);
//		
//		if(!reflection_shader.bValid) // ray goes to infinity
//			break;

//		auto ref_lights = scene.getVisibleLights(shader.pos);
//
//		// compute color 
//		clr += specular_coeff * (reflection_shader.mat.ambient + reflection_shader.mat.emission);
//		if(lights.size() > 0) { // if the scene point is lit by at least a single light
//			for(auto it = ref_lights.begin(); it != ref_lights.end(); it++) {
//				const Light* light = *it;
//				float angle_attenuation = static_cast<float>(light->angular_attenuation(reflection_shader.normal, reflection_shader.pos)); // std::max(0.0f, dot(normalize(shader.normal), normalize(light->position() - shader.pos)));
//				//float attenuation = std::max(0.0f, dot(normalize(reflection_shader.normal), normalize(light->position() - reflection_shader.pos)));
//				clr += specular_coeff * reflection_shader.mat.diffuse * light->color() * angle_attenuation * static_cast<float>(light->attenuation(reflection_shader.pos));
//			}
//		}
//		prev_shader = reflection_shader;
//		specular_coeff *= reflection_shader.mat.specular;
//	}
//}
