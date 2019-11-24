#include <GL/glew.h>		// GLEW, binds OpenGL functions
#include <GL/freeglut.h>	// FreeGLUT, includes glu.h and gl.
#include <iostream>

#include "config.h"
#include "event.h"

using namespace std;

int main(int argc, char *argv[]) {

	//	Initialize GLUT
	glutInit(&argc, argv);					
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_DEBUG);

	//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);

	//	Set the window's initial width & height and position
	glutInitWindowSize(config::SCREEN_WIDTH, config::SCREEN_HEIGHT);
	glutInitWindowPosition(200, 50);

	//	Create a window with the given title
	glutCreateWindow("CS405 Recitation Demo");

	//	Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err || !GL_VERSION_4_3)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	//	Call setup
	event::setup();

	//	Set glut handlers
	glutDisplayFunc(event::display);		//	Handler for window re-paint
	glutIdleFunc(event::display);			//	Handler for constant drawing
	
	//	Enter the infinitely event-processing loop
	glutMainLoop();

	return 0;
}