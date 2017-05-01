#include "sceneparser.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include "sphere.h"
#include "triangle.h"
#include <vector>
#include <stdexcept>
#include <glm.hpp>

using namespace std;

Scene SceneParser::parse(const std::string& filename)
{
	Scene scene;
	ifstream fin(filename);
	
	if(!fin.is_open()) {
		cout << "Unable to open file : " << filename << endl;
        throw std::runtime_error("Unable to open file");
	}
	scene.setOutputFilename("test.png");
	vector<glm::vec3> verts;
	Material material; //current material
	material.ambient = Colorf(0.2f);
	string line;
	string cmd;

	while(getline(fin, line)) {
		//cout << line << endl;
		if(line.length() == 0) 
			continue;
		istringstream ss(line);
		
		ss >> cmd;
		if(cmd == "#") {
			continue;
		} else if(cmd == "size") {
			int width, height;
			ss >> width >> height;
			scene.setCanvasSize(width, height);
		} else if(cmd == "maxdepth") {
			int depth;
			ss >> depth;
			scene.setMaxDepth(depth);
		} else if(cmd == "output") {
			std::string outfilename;
			ss >> outfilename;
			scene.setOutputFilename(outfilename);
		} else if(cmd == "camera") {
			float lookfromx, lookfromy, lookfromz, lookatx, lookaty, lookatz, upx, upy, upz, fov ;
			ss >> lookfromx >> lookfromy >> lookfromz >> lookatx >> lookaty >> lookatz >> upx >> upy >> upz >> fov;
			Camera *cam = new Camera(glm::vec3(lookfromx, lookfromy, lookfromz), glm::vec3(lookatx, lookaty, lookatz), glm::vec3(upx, upy, upz), fov);
			scene.addCamera(cam);
		} else if(cmd == "point") {
			float x, y, z, r, g, b;
			ss >> x >> y >> z >> r >> g >> b;
			Light *light = new Light(glm::vec3(x, y, z), Colorf(r, g, b));
			scene.addLight(light);
		}  else if(cmd == "directional") {
			float x, y, z, r, g, b;
			ss >> x >> y >> z >> r >> g >> b;
			Light *light = new Light(glm::vec3(x, y, z), Colorf(r, g, b), Light::LightType::DIRECTIONAL);
			scene.addLight(light);
		} else if(cmd == "sphere") {
			float x, y, z, radius;
			ss >> x >> y >> z >> radius;
			Object *obj = new Object(new Sphere(glm::vec3(x, y, z), radius), material);
			scene.addObject(obj);
		} else if(cmd == "maxverts") {
			// skip command since we store vertices in std::vector
		} else if(cmd == "vertex") {
			float  x, y, z;
			ss >> x >> y >> z;
			verts.push_back(glm::vec3(x, y, z));
		} else if(cmd == "tri") {
			int v1, v2, v3;
			ss >> v1 >> v2 >> v3; // v1, v2, v3 in counter-clockwise order
			Object *obj = new Object(new Triangle(verts[v1], verts[v2], verts[v3]), material);
			scene.addObject(obj);
		} else if(cmd == "diffuse") {
			float r, g, b;
			ss >> r >> g >> b;
			material.diffuse = Colorf(r, g, b);
		} else if(cmd == "ambient") {
			float r, g, b;
			ss >> r >> g >> b;
			material.ambient = Colorf(r, g, b);
		} else if(cmd == "specular") {
			float r, g, b;
			ss >> r >> g >> b;
			material.specular = Colorf(r, g, b);
		} else if(cmd == "emission") {
			float r, g, b;
			ss >> r >> g >> b;
			material.emission = Colorf(r, g, b);
		} else if(cmd == "shininess") {
			float s;
			ss >> s;
			material.shininess = s;
		}
	}
	return scene;
}
