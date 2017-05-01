#pragma once
#include <string>
#include "scene.h"

class SceneParser
{
public:
	static Scene parse(const std::string& filename); // parse the scene file and return the Scene
};

