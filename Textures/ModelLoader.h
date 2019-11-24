#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>

#include "glm/glm.hpp"
#include "Mesh.h"

struct ModelData {};

class ModelLoader {
public:
	static Mesh load(const char* file_name);
	~ModelLoader();

private:
	ModelLoader();
	static void loadOBJ(
		const char * path,
		std::vector<glm::vec3> & out_vertices,
		std::vector<glm::vec3> & out_normals,
		std::vector<glm::vec2> & out_uvs);
};
