#pragma once

#include <vector>
#include "glm/glm.hpp"
#include <GL/glew.h>
#include "GL/freeglut.h"
#include "config.h"


class Mesh
{
public:
	//std::vector<glm::vec3> vertices;
	//std::vector<glm::vec3> normals;
	//std::vector<glm::vec2> uvs;

	unsigned long vertex_size;
	GLuint vertex_buffer, normal_buffer, uv_buffer;
	
	

	Mesh();
	Mesh(
		const std::vector<glm::vec3>& vertices,
		const std::vector<glm::vec3>& normals,
		const std::vector<glm::vec2>& uvs);
	~Mesh();

	void setupBuffers();
};

