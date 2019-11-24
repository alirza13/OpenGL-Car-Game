#pragma once

#include <vector>

#include "GL/glew.h"
#include "GL/freeglut.h"

class Texture
{
public:
	GLuint gl_texture;
	unsigned long width, height;
	std::vector<unsigned char> pixels;

	Texture();
	Texture(unsigned long width, unsigned long height, const std::vector<unsigned char>& pixels);
	~Texture();
};

