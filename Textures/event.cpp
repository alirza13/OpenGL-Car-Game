#pragma once

#include "event.h"
#include <iostream>
#include <cstdlib> 
#include <ctime>
#include <GLFW/glfw3.h>
#include "TextureCube.h"
#include "Frustrum.h"

using namespace std;
using namespace glm;

namespace event {
	//	DEBUG PART
	void APIENTRY openglCallbackFunction(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		void* userParam) {

		std::cout << "---------------------opengl-callback-start------------" << std::endl;
		std::cout << "message: " << message << std::endl;
		std::cout << "type: ";
		switch (type) {
		case GL_DEBUG_TYPE_ERROR:
			std::cout << "ERROR";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			std::cout << "DEPRECATED_BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			std::cout << "UNDEFINED_BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			std::cout << "PORTABILITY";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			std::cout << "PERFORMANCE";
			break;
		case GL_DEBUG_TYPE_OTHER:
			std::cout << "OTHER";
			break;
		default:
			std::cout << "UNKNOWN";
		}
		std::cout << std::endl;

		std::cout << "id: " << id << std::endl;
		std::cout << "severity: ";
		switch (severity) {
		case GL_DEBUG_SEVERITY_LOW:
			std::cout << "LOW";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			std::cout << "MEDIUM";
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			std::cout << "HIGH";
			break;
		}
		std::cout << std::endl;
		std::cout << "---------------------opengl-callback-end--------------" << std::endl;
	}

	void setupDebuging() {
		std::cout << "Register OpenGL debug callback " << std::endl;
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglCallbackFunction, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(
			GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			&unusedIds,
			true);
	}
	// DEBUG PART END

	void configureGl() {
		//	Set background color to black and opaque
		glClearColor(0.4F, 0.1F, 0.1F, 1.0F);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_MULTISAMPLE);

		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
	}

	GLuint program, programSimple, programQuad, programSkybox;

	GLuint setup_program() {
		// Vertex Shader
		const GLchar *vertexSource = R"glsl(
			#version 430

			in vec3 aPosition;
			in vec3 aNormal;
			in vec2 aTexCoord;

			uniform mat4 uTransform;
			uniform mat4 uCamera;
			uniform mat4 uPerspective;
			uniform mat4 lightSpaceMatrix;

			out vec3 WorldPosition;
			out vec3 Normal;
			out vec2 TexCoord;
			out vec4 FragPosLightSpace;
			out mat4 Camera;

			void main()
			{
				WorldPosition = (uTransform * vec4(aPosition, 1.0)).xyz;
				Normal = normalize(mat3(transpose(inverse(uTransform))) * aNormal);
				TexCoord = aTexCoord;
				vec4 opengl_pos = uPerspective * uCamera * uTransform * vec4(aPosition, 1.0);
				FragPosLightSpace = lightSpaceMatrix  * vec4(WorldPosition, 1.0);
				gl_Position = opengl_pos;
				Camera = uCamera;
			}
		)glsl";

		// Fragment Shader
		const GLchar *fragmentSource = R"glsl(
			#version 430

			in vec3 WorldPosition;
			in vec3 Normal;
			in vec2 TexCoord;
            in vec4 FragPosLightSpace;
			in mat4 Camera;

			//uniform sampler2D Texture2;
			uniform sampler2D Texture;
			uniform sampler2D shadowMap;
			
			layout (location = 0) out vec4 outColor;

			float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
			{			
					// perform perspective divide
					vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
					// transform to [0,1] range
					projCoords = projCoords * 0.5 + 0.5;
					// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
					float closestDepth = texture(shadowMap, projCoords.xy).r; 
					// get depth of current fragment from light's perspective
					float currentDepth = projCoords.z;
					// check whether current frag pos is in shadow
					float bias = 0.005;  
					float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

					return shadow;
			}  

			void main()
			{
				
				//vec3 light_pos = vec3(300.0, 1000.0, 0.0);

				//vec3 light_direction = normalize(light_pos - WorldPosition.xyz);
				//float light_density = dot(light_direction, Normal);


				//vec2 flippedTexCoord = vec2(TexCoord.x, 1.0 - TexCoord.y);
				//vec4 color = texture(Texture, flippedTexCoord);

				////vec4 tint = vec4(0.2, 0.8, 0.5, 1.0);
				////color *= tint;

				//outColor = color * (light_density + 0.2);
				////outColor = vec4(Normal, 1.0);
	
				vec3 light_pos = vec3(-300.0, 1000.0, 1250.0);

				vec3 viewPos = Camera[3].xyz;
				
				vec3 color = texture(Texture, TexCoord).rgb;
				vec3 normal = normalize(Normal);
				vec3 lightColor = vec3(1.0);
				// ambient
				vec3 ambient = 0.15 * color;
				// diffuse
				vec3 lightDir = normalize(light_pos - WorldPosition);
				float diff = max(dot(lightDir, normal), 0.0);
				vec3 diffuse = diff * lightColor;
				// specular
				vec3 viewDir = normalize(viewPos - WorldPosition);
				float spec = 0.0;
				vec3 halfwayDir = normalize(lightDir + viewDir);  
				spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
				vec3 specular = spec * lightColor;    
				// calculate shadow
				float shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);       
				vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
				outColor = vec4(lighting, 1.0);
			}
		)glsl";

		// Create and compile the vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);

		// Create and compile the fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);

		// Link the vertex and fragment shader into a shader program
		GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);


		glBindAttribLocation(program, ATTR_LOCATION::POSITION, "aPosition");
		glBindAttribLocation(program, ATTR_LOCATION::NORMAL, "aNormal");
		glBindAttribLocation(program, ATTR_LOCATION::UV, "aTexCoord");



		glLinkProgram(program);
		GLint position = glGetAttribLocation(program, "aNormal");

		return program;
	}


	GLuint setup_simpleProgramBuffer() {
		// Vertex Shader
		const GLchar *vertexSource = R"glsl(
			#version 430
			
			in layout(location = 0) vec3 aPosition;
			uniform mat4 uTransform;
			uniform mat4 lightSpaceMatrix;
			void main()
			{
				gl_Position = lightSpaceMatrix * uTransform * vec4(aPosition, 1.0);
			}
		)glsl";

		// Fragment Shader
		const GLchar *fragmentSource = R"glsl(
			#version 430

			void main()
			{
				gl_FragDepth = gl_FragCoord.z;
	
			}
		)glsl";

		// Create and compile the vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);

		// Create and compile the fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);

		// Link the vertex and fragment shader into a shader program
		GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		glBindAttribLocation(program, ATTR_LOCATION::POSITION, "aPosition");

		glLinkProgram(program);
		GLint position = glGetAttribLocation(program, "aPosition");

		return program;
	}

	GLuint setup_simpleProgramQuad() {
		// Vertex Shader
		const GLchar *vertexSource = R"glsl(
			#version 430

			in vec3 aPosition;
			in vec2 aTexCoord;
			out vec2 TexCoords;
		
			void main()
			{
				TexCoords = aTexCoord;
				gl_Position = vec4(aPosition, 1.0);
			}
		)glsl";

		// Fragment Shader
		const GLchar *fragmentSource = R"glsl(
			#version 430

		out vec4 FragColor;

		in vec2 TexCoords;

		uniform sampler2D depthMap;
		uniform float near_plane;
		uniform float far_plane;

		// required when using a perspective projection matrix
		float LinearizeDepth(float depth)
		{
			float z = depth * 2.0 - 1.0; // Back to NDC 
			return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
		}
		void main()
		{
			float depthValue = texture(depthMap, TexCoords).r;
			FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
			//FragColor = vec4(vec3(depthValue), 1.0); // orthographic
	
		}
		)glsl";

		// Create and compile the vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);

		// Create and compile the fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);

		// Link the vertex and fragment shader into a shader program
		GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		glBindAttribLocation(program, ATTR_LOCATION::POSITION, "aPosition");
		glBindAttribLocation(program, ATTR_LOCATION::UV, "aTexCoord");

		glLinkProgram(program);

		return program;
	}

	// SKYBOX SHADER SKYBOX SHADER SKYBOX SHADER SKYBOX SHADER SKYBOX SHADER

	GLuint setup_skyboxProgramBuffer() {
		// Vertex Shader
		const GLchar *vertexSource = R"glsl(
			#version 430
			
			in layout(location = 0) vec3 aPosition;
			uniform mat4 view;
			uniform mat4 projection;
			out vec3 TexCoords;

			void main()
			{
				vec4 pos = projection * view * vec4(aPosition, 1.0);
				TexCoords = aPosition;
				gl_Position = pos.xyww;		
			}
		)glsl";

		// Fragment Shader
		const GLchar *fragmentSource = R"glsl(
			#version 430

			in vec3 TexCoords;
			out vec4 color;
			uniform samplerCube skybox;

			void main()
			{
				color = texture(skybox,TexCoords);
			}
		)glsl";

		// Create and compile the vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);

		// Create and compile the fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);

		// Link the vertex and fragment shader into a shader program
		GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		glBindAttribLocation(program, ATTR_LOCATION::POSITION, "aPosition");

		glLinkProgram(program);

		return program;
	}

	// SKYBOX SHADER SKYBOX SHADER SKYBOX SHADER SKYBOX SHADER SKYBOX SHADER


	unsigned int depthMapFBO;
	int oldTimeSinceStart, deltaTime;
	glm::mat4 transformCar, transformRoad, transformRamp, transformCollison, camera;
	mat4 view;
	mat4 comboMatrix;
	glm::mat4 perspective;
	FrustumG frustum;
	const int numberOfPoints = 50;
	const int  numberOfBarriers = 100;
	float numberOfPointsDrawn = 0;
	GLuint skyboxVAO;
	GLuint cubemapTexture;
	glm::mat4 points[numberOfPoints];
	glm::mat4 barriers[numberOfBarriers];
	Mesh meshCar, meshRoad, meshRamp;
	Mesh pointCubes[numberOfPoints];
	Mesh barrierCubes[numberOfBarriers];
	Mesh collisionCube;
	bool isCollidedPoint[numberOfPoints];
	bool isCollidedBarrier[numberOfBarriers];
	float pointCounter = 100;

	const float g = 9.81f;  // Gravity of Earth in m/s²
	GLuint index, index1, index2, collisionIndex;
	GLuint pointIndexes[numberOfPoints];
	GLuint barrierIndexes[numberOfBarriers];
	bool insideCar = false;

	Texture texture, texture1, texture2, texture3;

	void keyboard(unsigned char key, int x, int y)
	{

		if (key == 'a')
		{
			if (transformCar[3].x > -60) {
				transformCar = glm::rotate(transformCar, radians(20.0f / 60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				/*transformCar = glm::translate(transformCar, glm::vec3(-5.0f / 60.0f, 0.0f, 0.0f));*/
				transformCar[3].x -= 5.0f / 60.0f * deltaTime;
			}
		}

		if (key == 'd')
		{
			if (transformCar[3].x < 45) {
				transformCar = glm::rotate(transformCar, radians(-20.0f / 60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				//transformCar = glm::translate(transformCar, glm::vec3(5.0f / 60.0f, 0.0f, 0.0f));
				transformCar[3].x += 5.0f / 60.0f * deltaTime;
			}
		}

		if (key == 32)
		{
			if (transformCar[3].y <= 1)
				transformCar = glm::translate(transformCar, glm::vec3(0.0f, 150.0f / 60.0f, 0.0f));
		}

		if (key == 'w')
		{
			camera[3].z += 30.0f / 60.0f;
			camera[3].y -= 10.0f / 60.0f;
			glUniformMatrix4fv(glGetUniformLocation(program, "uCamera"), 1, GL_FALSE, &camera[0][0]);

		}

		if (key == 'v')
		{
			if (!insideCar)
				insideCar = true;
			else
				insideCar = false;

		}

		if (key == 's')
		{
			camera[3].z -= 30.0f / 60.0f;
			camera[3].y += 10.0f / 60.0f;
			glUniformMatrix4fv(glGetUniformLocation(program, "uCamera"), 1, GL_FALSE, &camera[0][0]);

		}


	}

	// FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM
	// FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM
	struct Plane
	{
		float a, b, c, d;
	};

	enum Halfspace
	{
		NEGATIVE = -1,
		ON_PLANE = 0,
		POSITIVE = 1,
	};

	Halfspace ClassifyPoint(const Plane & plane, vec3 pt)
	{
		float d;
		d = plane.a*pt.x + plane.b*pt.y + plane.c*pt.z + plane.d;
		if (d < 0) return NEGATIVE;
		if (d > 0) return POSITIVE;
		return ON_PLANE;
	}

	struct Matrix4x4
	{
		// The elements of the 4x4 matrix are stored in
		// column-major order (see "OpenGL Programming Guide",
		// 3rd edition, pp 106, glLoadMatrix).
		float _11, _21, _31, _41;
		float _12, _22, _32, _42;
		float _13, _23, _33, _43;
		float _14, _24, _34, _44;
	};

	void NormalizePlane(Plane & plane)
	{
		float mag;
		mag = sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
		plane.a = plane.a / mag;
		plane.b = plane.b / mag;
		plane.c = plane.c / mag;
		plane.d = plane.d / mag;
	}

	void ExtractPlanesGL(
		Plane * p_planes,
		const mat4 & comboMatrix,
		bool normalize)
	{
		// Left clipping plane
		p_planes[0].a = comboMatrix[3][0] + comboMatrix[0][0];
		p_planes[0].b = comboMatrix[3][1] + comboMatrix[0][1];
		p_planes[0].c = comboMatrix[3][2] + comboMatrix[0][2];
		p_planes[0].d = comboMatrix[3][3] + comboMatrix[0][3];
		// Right clipping plane
		p_planes[1].a = comboMatrix[3][0] - comboMatrix[0][0];
		p_planes[1].b = comboMatrix[3][1] - comboMatrix[0][1];
		p_planes[1].c = comboMatrix[3][2] - comboMatrix[0][2];
		p_planes[1].d = comboMatrix[3][3] - comboMatrix[0][3];
		// Top clipping plane
		p_planes[2].a = comboMatrix[3][0] - comboMatrix[1][0];
		p_planes[2].b = comboMatrix[3][1] - comboMatrix[1][1];
		p_planes[2].c = comboMatrix[3][2] - comboMatrix[1][2];
		p_planes[2].d = comboMatrix[3][3] - comboMatrix[1][3];
		// Bottom clipping plane
		p_planes[3].a = comboMatrix[3][0] + comboMatrix[1][0];
		p_planes[3].b = comboMatrix[3][1] + comboMatrix[1][1];
		p_planes[3].c = comboMatrix[3][2] + comboMatrix[1][2];
		p_planes[3].d = comboMatrix[3][3] + comboMatrix[1][3];
		// Near clipping plane
		p_planes[4].a = comboMatrix[2][0] + comboMatrix[3][0];
		p_planes[4].b = comboMatrix[2][1] + comboMatrix[3][1];
		p_planes[4].c = comboMatrix[2][2] + comboMatrix[3][2];
		p_planes[4].d = comboMatrix[2][3] + comboMatrix[3][3];
		// Far clipping plane
		p_planes[5].a = comboMatrix[3][0] - comboMatrix[2][0];
		p_planes[5].b = comboMatrix[3][1] - comboMatrix[2][1];
		p_planes[5].c = comboMatrix[3][2] - comboMatrix[2][2];
		p_planes[5].d = comboMatrix[3][3] - comboMatrix[2][3];

		// Normalize the plane equations, if requested
		if (normalize == true)
		{
			NormalizePlane(p_planes[0]);
			NormalizePlane(p_planes[1]);
			NormalizePlane(p_planes[2]);
			NormalizePlane(p_planes[3]);
			NormalizePlane(p_planes[4]);
			NormalizePlane(p_planes[5]);
		}
	}

	bool boxInFrustumPoints(mat4 &b) {
		Plane * planes = new Plane[6];
		ExtractPlanesGL(planes, comboMatrix, true);		
		//for (int i = 0; i < 6; i++)
		//	cout <<   " Plane " << i << " x: " << planes[i].a << " Plane y: " << planes[i].b << " Plane z: " << planes[i].c <<  " Plane w: " << planes[i].d << endl;
		
		for (int i = 0; i < 6; i++)
		{
			float distance = planes[i].a * b[3].x + planes[i].b * b[3].y + planes[i].c* b[3].z + planes[i].d - 11.5f;
			if (distance > 0)
			{
				return false;
			}
		}
		return true;
	}

	// FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM
	// FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM FRUSTRUM

	void CarCameraFollow() {
		if (!insideCar) {
			transformCar = glm::translate(transformCar, glm::vec3(0.0f, 0.0f, -5.31f / 60.0f));
			vec3 cameraPosition = vec3(0, 0, transformCar[3].z) - vec3(0.0f, -15.0f, -110.0f);
			vec3 cameraTarget = vec3(0, 0, transformCar[3].z);
			vec3 cameraUp = vec3(0, 1, 0);
			transformCollison[3].z = transformCar[3].z;
			view = lookAt(cameraPosition, cameraTarget, cameraUp);
			Vec3 camPos(cameraPosition.x, cameraPosition.y, cameraPosition.z);
			Vec3 camTarget(cameraTarget.x, cameraTarget.y, cameraTarget.z);
			Vec3 camUp(cameraUp.x, cameraUp.y, cameraUp.z);
			frustum.setCamDef(camPos, camTarget, camUp);
			//cameraPosition.z -= 0.02f / 60.0f;
			//cout << cameraTarget.x << "  " << cameraTarget.y << "   " << cameraTarget.z << endl;
			//cout << cameraPosition.x << "  " << cameraPosition.y << "   " << cameraPosition.z << endl;
			glUniformMatrix4fv(glGetUniformLocation(program, "uCamera"), 1, GL_FALSE, &view[0][0]);
		}
		else {
			transformCar = glm::translate(transformCar, glm::vec3(0.0f, 0.0f, -3.31f / 60.0f));
			vec3 cameraPosition = vec3(transformCar[3].x, transformCar[3].y, transformCar[3].z) - vec3(0.0f, -2.5f, 16.585f);
			vec3 cameraTarget = vec3(0, 0, -1000000);
			//vec3 cameraDirection = glm::normalize(-cameraPosition - cameraTarget);
			vec3 cameraUp = vec3(0, 1, 0);
			transformCollison[3].z = transformCar[3].z;
			view = lookAt(cameraPosition, cameraTarget, cameraUp);
			Vec3 camPos(cameraPosition.x, cameraPosition.y, cameraPosition.z);
			Vec3 camTarget(cameraTarget.x, cameraTarget.y, cameraTarget.z);
			Vec3 camUp(cameraUp.x, cameraUp.y, cameraUp.z);
			frustum.setCamDef(camPos, camTarget, camUp);
			//cameraPosition.z -= 0.02f / 60.0f;
			//cout << cameraTarget.x << "  " << cameraTarget.y << "   " << cameraTarget.z << endl;
			//cout << cameraPosition.x << "  " << cameraPosition.y << "   " << cameraPosition.z << endl;
			glUniformMatrix4fv(glGetUniformLocation(program, "uCamera"), 1, GL_FALSE, &view[0][0]);
		}
	}

	void Gravity(glm::mat4 & object)
	{
		if (object[3].y >= 0.55)
		{
			object = glm::translate(object, glm::vec3(0.0f, -0.55f / 60.0f, 0.0f));
		}
	}

	void RestrictBounds()
	{
		if (transformCar[3].x < -60) {
			transformCar[3].x = -60;
		}
		if (transformCar[3].x > 45) {
			transformCar[3].x = 45;
		}
	}

	void AABBCollisionPoint(const glm::mat4 & object, int index)
	{

		if (transformCar[3].x < object[3].x + 13.5f &&
			transformCar[3].x + 13.5f > object[3].x &&
			transformCar[3].y < object[3].y + 10.5f &&
			transformCar[3].y + 10.5f > object[3].y  &&
			transformCar[3].z < object[3].z + 11.5f &&
			transformCar[3].z + 11.5f > object[3].z
			)
		{
			if (!isCollidedPoint[index])
			{
				isCollidedPoint[index] = true;
				pointCounter += 100;
				cout << "Points: " << pointCounter << endl;
			}
		}

	}

	void AABBCollisionBarrier(const glm::mat4 & object, int index)
	{

		if ((transformCar[3].x < object[3].x + 35.5f) &&
			transformCar[3].x + 13.5f > object[3].x - 22.5f &&
			transformCar[3].y < object[3].y + 10.5f &&
			transformCar[3].y + 10.5f > object[3].y  &&
			transformCar[3].z < object[3].z + 11.5f &&
			transformCar[3].z + 11.5f > object[3].z
			)
		{
			if (!isCollidedBarrier[index])
			{
				pointCounter -= 100;
				isCollidedBarrier[index] = true;
			}

			for (int i = 0; i < numberOfPoints; i++)
			{
				isCollidedPoint[i] = false;
			}
			transformCar[3].x = 0;
			transformCar[3].y = 0;
			transformCar[3].z = -5;
		}
	}



	void setup() {
		setupDebuging();

		configureGl();

		//	Create and bind a VAO	
		glGenVertexArrays(1, &index);
		glBindVertexArray(index);

		//	Create program
		program = setup_program();
		programSimple = setup_simpleProgramBuffer();
		programQuad = setup_simpleProgramQuad();
		programSkybox = setup_skyboxProgramBuffer();

		glUseProgram(program);

		//	Create a mesh
		meshCar = ModelLoader::load("Porsche_911_GT2.obj");

		//	Create a texture
		texture = PNGLoader::load("road.png");
		glUniform1i(glGetUniformLocation(program, "Texture"), 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.gl_texture);

		/*	glUniform1i(glGetUniformLocation(program, "Texture2"), 1);
			texture1 = PNGLoader::load("default.png");
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, texture1.gl_texture);*/

		texture1 = PNGLoader::load("default.png");
		texture2 = PNGLoader::load("brick.png");


		glGenFramebuffers(1, &depthMapFBO);
		const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
		unsigned int depthMap;
		glGenTextures(1, &depthMap);


		glActiveTexture(GL_TEXTURE1);
		glUniform1i(glGetUniformLocation(program, "shadowMap"), 1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glUseProgram(programQuad);
		glUniform1i(glGetUniformLocation(programQuad, "depthMap"), 0);
		glUseProgram(program);


		//	Create model matrix
		transformCar = glm::scale(glm::mat4(1.0), glm::vec3(10.0F));

		glUniformMatrix4fv(glGetUniformLocation(program, "uTransform"), 1, GL_FALSE, &transformCar[0][0]);
		//rotate = glm::scale(rotate, glm::vec3(1.2F));

		//	Create Camera matrix
		glm::vec3 camera_pos = glm::vec3(0.0F, 15.0F, 310.0F);
		glm::vec3 lookat_pos = glm::vec3(0.0F, 0.0F, 0.0F);
		glm::vec3 up = glm::vec3(0.0F, 1.0F, 0.0F);
		camera = glm::lookAt(camera_pos, lookat_pos, up);
		glUniformMatrix4fv(glGetUniformLocation(program, "uCamera"), 1, GL_FALSE, &camera[0][0]);

		//	Create Perspective matrix
		float FoV = 3.14159 * 70. / 180;
		perspective = glm::perspective(FoV, config::SCREEN_RATIO, 0.1F, 600.0F);
		glUniformMatrix4fv(glGetUniformLocation(program, "uPerspective"), 1, GL_FALSE, &perspective[0][0]);
		frustum.setCamInternals(70.0, config::SCREEN_RATIO, 0.1F, 600.0F);

		float near_plane = 1.0f, far_plane = 7.5f;
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		glUseProgram(programQuad);
		glUniform1f(glGetUniformLocation(programQuad, "near_plane"), near_plane);
		glUniform1f(glGetUniformLocation(programQuad, "far_plane"), far_plane);

		glm::mat4 lightSpaceMatrix = lightProjection * lightView;
		glUseProgram(programSimple);
		glUniformMatrix4fv(glGetUniformLocation(programSimple, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		//display();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);




		glGenVertexArrays(1, &index1);
		glBindVertexArray(index1);

		meshRoad = ModelLoader::load("cube.obj");   // Road
		transformRoad = glm::scale(glm::mat4(1.0), glm::vec3(500.0F, 1.0F, 200000.0F));
		transformRoad = glm::translate(transformRoad, glm::vec3(0.0f, -5.5f, 0.0f));
		glUseProgram(programSimple);
		glUniformMatrix4fv(glGetUniformLocation(programSimple, "uTransform"), 1, GL_FALSE, &transformRoad[0][0]);
		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "uTransform"), 1, GL_FALSE, &transformRoad[0][0]);

		glGenVertexArrays(1, &index2);
		glBindVertexArray(index2);

		meshRamp = ModelLoader::load("16308_SkatePark_launchramp_v2_NEW.obj");

		transformRamp = glm::scale(glm::mat4(1.0), glm::vec3(0.1F, 0.1F, 0.1F));
		transformRamp = glm::translate(transformRamp, glm::vec3(3.0f, 0.5f, -350.0f));
		glUniformMatrix4fv(glGetUniformLocation(program, "uTransform"), 1, GL_FALSE, &transformRamp[0][0]);


		float distanceZ = 0;

		for (int i = 0; i < numberOfPoints; i++) // Points
		{
			float r3 = -6 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (6 - (-6))));
			float distanceX = r3;
			glGenVertexArrays(1, &pointIndexes[i]);
			glBindVertexArray(pointIndexes[i]);
			pointCubes[i] = ModelLoader::load("cube.obj");
			points[i] = glm::scale(glm::mat4(1.0), glm::vec3(10.0F, 10.0F, 10.0F));
			points[i] = glm::translate(points[i], glm::vec3(distanceX, 0.0f, distanceZ));
			glUniformMatrix4fv(glGetUniformLocation(program, "uTransform"), 1, GL_FALSE, &points[i][0][0]);
			distanceZ -= 60.0f;
			//distanceX -= 1.0f;
		}

		float distanceBZ = 0;

		for (int i = 0; i < numberOfBarriers; i++) // Barriers
		{
			float randomNumber = -0.80 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.80 - (-0.80))));
			float distanceX = randomNumber;
			glGenVertexArrays(1, &barrierIndexes[i]);
			glBindVertexArray(barrierIndexes[i]);
			barrierCubes[i] = ModelLoader::load("cube.obj");
			barriers[i] = glm::scale(glm::mat4(1.0), glm::vec3(50.0F, 10.0F, 5.0F));
			barriers[i] = glm::translate(barriers[i], glm::vec3(distanceX, 0.0f, distanceBZ));
			glUniformMatrix4fv(glGetUniformLocation(program, "uTransform"), 1, GL_FALSE, &barriers[i][0][0]);
			distanceBZ -= 70.0f;
			//distanceX -= 1.0f;
		}

		glGenVertexArrays(1, &collisionIndex);
		glBindVertexArray(collisionIndex);
		collisionCube = ModelLoader::load("cube.obj");
		transformCollison = glm::scale(glm::mat4(1.0), glm::vec3(10.0F, 10.0F, 15.0F));
		transformCollison = glm::translate(transformCollison, glm::vec3(transformCar[3].x - 1, 0.0F, transformCar[3].z - 2));
		glUniformMatrix4fv(glGetUniformLocation(programSkybox, "uTransform"), 1, GL_FALSE, &transformCollison[0][0]);


		GLfloat skyboxVertices[] = {
			// Positions
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f,  1.0f
		};

		// Setup skybox VAO
		GLuint skyboxVBO;
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
		glBindVertexArray(0);


		// Cubemap (Skybox)
		vector<const GLchar*> faces;
		faces.push_back("arctic_rt.tga");
		faces.push_back("arctic_lf.tga");
		faces.push_back("arctic_up.tga");
		faces.push_back("arctic_dn.tga");
		faces.push_back("arctic_bk.tga");
		faces.push_back("arctic_ft.tga");

		TextureCube* txCube = new TextureCube();

		cubemapTexture = txCube->LoadCube(faces);

		oldTimeSinceStart = 0;

	}

	void display() {
		//	Clear the color buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
		deltaTime = timeSinceStart - oldTimeSinceStart;
		oldTimeSinceStart = timeSinceStart;

		glBindVertexArray(index);
		glUseProgram(programSimple);
		glUniformMatrix4fv(glGetUniformLocation(programSimple, "uTransform"), 1, GL_FALSE, &transformCar[0][0]);

		glUseProgram(programQuad);
		glUniformMatrix4fv(glGetUniformLocation(programQuad, "uTransform"), 1, GL_FALSE, &transformCar[0][0]);
		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "uTransform"), 1, GL_FALSE, &transformCar[0][0]);
		CarCameraFollow();
		glUniform1i(glGetUniformLocation(program, "Texture"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.gl_texture);
		RestrictBounds();
		Gravity(transformCar);
		glDrawArrays(GL_TRIANGLES, 0, meshCar.vertex_size);
		glBindVertexArray(index1);
		glUseProgram(programSimple);
		glUniformMatrix4fv(glGetUniformLocation(programSimple, "uTransform"), 1, GL_FALSE, &transformRoad[0][0]);
		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "uTransform"), 1, GL_FALSE, &transformRoad[0][0]);
		glUniform1i(glGetUniformLocation(program, "Texture"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.gl_texture);
		glDrawArrays(GL_TRIANGLES, 0, meshRoad.vertex_size);

		for (int i = 0; i < numberOfPoints; i++)   //  Points
		{
			glBindVertexArray(pointIndexes[i]);
			points[i] = glm::rotate(points[i], radians(10.0f / 60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glUniform1i(glGetUniformLocation(program, "Texture"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1.gl_texture);
			AABBCollisionPoint(points[i], i);
			//comboMatrix = perspective * view;
			//bool inFrustrum = boxInFrustumPoints(points[i]);
			glUniformMatrix4fv(glGetUniformLocation(program, "uTransform"), 1, GL_FALSE, &points[i][0][0]);
			Vec3 pointPosition;
			pointPosition.x = points[i][3].x;
			pointPosition.y = points[i][3].y;
			pointPosition.z = points[i][3].z;
			if ((frustum.sphereInFrustum(pointPosition, 13.5f) != FrustumG::OUTSIDE) && !isCollidedPoint[i])
			{
				glDrawArrays(GL_TRIANGLES, 0, pointCubes[i].vertex_size);
			}
				
		}
		// TEST TEST TEST FOR FRUSTUM
		//Vec3 testPosition;   
		//testPosition.x = -59.0;
		//testPosition.y = 0;
		//testPosition.z = 0;
		//if (frustum.sphereInFrustum(testPosition, 13.5f) != FrustumG::OUTSIDE) 
		//{
		//	cout << "First one is inside" << endl;
		//}
		

		for (int i = 0; i < numberOfBarriers; i++)  //  Barriers
		{
			glBindVertexArray(barrierIndexes[i]);

			glUniform1i(glGetUniformLocation(program, "Texture"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture2.gl_texture);
			AABBCollisionBarrier(barriers[i], i);
			Vec3 barrierPosition;
			barrierPosition.x = barriers[i][3].x;
			barrierPosition.y = barriers[i][3].y;
			barrierPosition.z = barriers[i][3].z;
			glUniformMatrix4fv(glGetUniformLocation(program, "uTransform"), 1, GL_FALSE, &barriers[i][0][0]);
			if (frustum.sphereInFrustum(barrierPosition, 35.5f) != FrustumG::OUTSIDE)
				glDrawArrays(GL_TRIANGLES, 0, barrierCubes[i].vertex_size);
		}

		// Draw skybox as last
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		glUseProgram(programSkybox);
		mat4 viewSkybox = mat4(mat3(camera));	// Remove any translation component of the view matrix

		glUniformMatrix4fv(glGetUniformLocation(programSkybox, "view"), 1, GL_FALSE, &viewSkybox[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(programSkybox, "projection"), 1, GL_FALSE, &perspective[0][0]);

		// skybox cube
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default

		glutSwapBuffers();
		glutKeyboardFunc(keyboard);
	}
}