#pragma once
#pragma once

#include <vector>

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "Texture.h"
#include "PNGLoader.h"
#include "SOIL2/SOIL2/SOIL2.h"

using namespace std;

class TextureCube
{
public:
	GLuint gl_texture;
	unsigned long width, height;
	std::vector<unsigned char> pixels;

	TextureCube();
	
	GLuint LoadCube(vector <const GLchar * > faces);
	~TextureCube();
};

