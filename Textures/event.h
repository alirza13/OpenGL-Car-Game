#pragma once

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <iostream>

#include "ModelLoader.h"
#include "PNGLoader.h"
#include "Mesh.h"
#include "config.h"

namespace event {
	void setup();
	void display();
}