

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include <iostream>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>

//Sphere Parameters
#define radius 0.5
#define stacks 8 
#define slices 16
#define kreisradius 2


//LightSphere Parameters
#define radiusLight 0.2
#define stacksLight 8 
#define slicesLight 16


const GLchar* vertexShaderSource =
"#version 440 core\n"
"layout(location = 0) in vec3 aPos;"
"layout(location = 1) in vec4 in_Color;"
""
"out vec4 fragmentColor;"
""
"uniform mat4 view;"
"uniform mat4 projection;"
""
"void main()"
"{"
"	gl_Position = projection * view * vec4(aPos, 1.0);"
"   fragmentColor = in_Color;"
"}";

const GLchar* vertexTextureShaderSource =
"#version 440 core\n"
"layout(location = 0) in vec3 aPos;"
"layout(location = 1) in vec4 in_Color;"
"layout(location = 2) in vec2 in_TexCoord;"
""
"out vec4 fragmentColor;"
"out vec2 TexCoord;"
""
"uniform mat4 view;"
"uniform mat4 projection;"
""
"void main()"
"{"
"	gl_Position = projection * view * vec4(aPos, 1.0);"
"   fragmentColor = in_Color;"
"	TexCoord = in_TexCoord;"
"}";

const GLchar* fragmentShaderSource =
"#version 440 core\n"
"out vec4 out_color;\n"
""
"in vec4 fragmentColor;"
""
"void main()"
"{"
"  out_color = fragmentColor;\n"
"}";

const GLchar* fragmentTextureShaderSource =
"#version 440 core\n"
"out vec4 out_color;\n"
""
"in vec4 fragmentColor;"
"in vec2 TexCoord;"
""
"uniform sampler2D texture1;"
""
"void main()"
"{"
"  out_color = texture(texture1, TexCoord);\n"
"}";

int screenWidth = 600;
int screenHeight = 600;


// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

float pi = 3.14159265358979323846;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void AdjustVertexData(int lightposition, std::vector<float> &LightSphereCenters, std::vector<float> &LightSphere);

void processInput(GLFWwindow *window);

std::vector<float> calculateMobiusVertices(int rootOfVertices);
std::vector<int> calculateMobiusIndices(int rootOfIndices);
std::vector<float> calculateMobiusColors(int rootOfMobiusColors);

std::vector<float> calculateSphereVertices(int rootOfSphereVertices);
std::vector<int> calculateSphereIndices(int rootOfSphereIndices);
std::vector<float> calculateSphereColors(int rootOfSphereColors);

std::vector<float> calculateLightSphereVertices(int rootOfSphereVertices);
std::vector<int> calculateLightSphereIndices(int rootOfSphereIndices);
std::vector<float> calculateLightSphereColors(int rootOfSphereColors);
std::vector<float> calculateLightSphereCenters(int rootOfMobiusColors);

std::vector<float>GenerateSphereTexCoordinates();

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //initiate Opengl 4.4
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(600, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}


	glViewport(0, 0, screenWidth, screenHeight);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	//NON-TEXTURED SHADER
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(0);
	//TEXTURED SHADER
	unsigned int vertexTextureShader;
	vertexTextureShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexTextureShader, 1, &vertexTextureShaderSource, NULL);
	glCompileShader(vertexTextureShader);

	glGetShaderiv(vertexTextureShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexTextureShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int fragmentTextureShader;
	fragmentTextureShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentTextureShader, 1, &fragmentTextureShaderSource, NULL);
	glCompileShader(fragmentTextureShader);

	glGetShaderiv(fragmentTextureShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragmentTextureShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int shaderTextureProgram;
	shaderTextureProgram = glCreateProgram();

	glAttachShader(shaderTextureProgram, vertexTextureShader);
	glAttachShader(shaderTextureProgram, fragmentTextureShader);
	glLinkProgram(shaderTextureProgram);


	glDeleteShader(vertexTextureShader);
	glDeleteShader(fragmentTextureShader);

	glUseProgram(shaderTextureProgram);

	glUseProgram(shaderProgram);
	//camera matrix
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	//setting initial camera
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);

	//perpective
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &proj[0][0]);

	std::vector<float> mobiusVertices = calculateMobiusVertices(64 * 3);
	std::vector<int> mobiusIndices = calculateMobiusIndices(64 * 3);
	std::vector<float> mobiusColors = calculateMobiusColors(192 * 4);

	std::vector<float> sphereVertices = calculateSphereVertices(160 * 3);
	std::vector<int> sphereIndices = calculateSphereIndices(272);
	std::vector<float> sphereColors = calculateSphereColors(160 * 4);

	std::vector<float> LightSphereVertices = calculateLightSphereVertices(160 * 3);
	std::vector<int> LightSphereIndices = calculateLightSphereIndices(272);
	std::vector<float> LightSphereColors = calculateLightSphereColors(160 * 4);
	std::vector<float> LightSphereCenters = calculateLightSphereCenters(360 * 3);


	// ids for mobius
	GLuint  VAO;
	GLuint  EBO;
	GLuint  VBOcoords;



	// create mobius vertex array object
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// create coords object
	glGenBuffers(1, &VBOcoords);
	glBindBuffer(GL_ARRAY_BUFFER, VBOcoords);
	glBufferData(GL_ARRAY_BUFFER, 4 * mobiusVertices.size(), &mobiusVertices.front(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// create color buffer
	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * mobiusColors.size(), &mobiusColors.front(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// create buffer object for indices
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * mobiusIndices.size(), &mobiusIndices.front(), GL_STATIC_DRAW);


	glUseProgram(shaderTextureProgram);
	glUniformMatrix4fv(glGetUniformLocation(shaderTextureProgram, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderTextureProgram, "projection"), 1, GL_FALSE, &proj[0][0]);
	// ids for sphere
	GLuint  sphere_VAO;
	GLuint  sphere_EBO;
	GLuint  sphere_VBOcoords;
	GLuint  sphere_VBOtex;

	//sphere
	glGenVertexArrays(1, &sphere_VAO);
	glBindVertexArray(sphere_VAO);

	//sphere
	glGenBuffers(1, &sphere_VBOcoords);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_VBOcoords);
	glBufferData(GL_ARRAY_BUFFER, 4 * sphereVertices.size(), &sphereVertices.front(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0); //Sphere is position2
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//Sphere Color buffer
	GLuint colorbufferSphere;
	glGenBuffers(1, &colorbufferSphere);
	glBindBuffer(GL_ARRAY_BUFFER, colorbufferSphere);
	glBufferData(GL_ARRAY_BUFFER, 6 * sphereColors.size(), &sphereColors.front(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1); //Sphere colors is position3
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//Sphere Indices
	glGenBuffers(1, &sphere_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sphereIndices.size(), &sphereIndices.front(), GL_STATIC_DRAW);

	//Sphere Texture Coordinates
	std::vector<float> texCoords = GenerateSphereTexCoordinates();
	glGenBuffers(1, &sphere_VBOtex);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_VBOtex);
	glBufferData(GL_ARRAY_BUFFER, 4 * texCoords.size(), &texCoords.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);

	//EarthTexture
	
	unsigned int textureEarth;
	glGenTextures(1, &textureEarth);
	glBindTexture(GL_TEXTURE_2D, textureEarth); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char *data = stbi_load(std::string("2k_earth_daymap.jpg").c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	std::cout << "test" << std::endl;

	// ids for LightSphere
	GLuint  LightSphere_VAO;
	GLuint  LightSphere_EBO;
	GLuint  LightSphere_VBOcoords;
	GLuint  LightSphere_VBOtex;

	//Lightsphere
	glGenVertexArrays(1, &LightSphere_VAO);
	glBindVertexArray(LightSphere_VAO);

	//Lightsphere
	glGenBuffers(1, &LightSphere_VBOcoords);
	glBindBuffer(GL_ARRAY_BUFFER, LightSphere_VBOcoords);
	glBufferData(GL_ARRAY_BUFFER, 4 * LightSphereVertices.size(), &LightSphereVertices.front(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0); //Sphere is position2
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//LightSphere Color buffer
	GLuint colorbufferLightSphere;
	glGenBuffers(1, &colorbufferLightSphere);
	glBindBuffer(GL_ARRAY_BUFFER, colorbufferLightSphere);
	glBufferData(GL_ARRAY_BUFFER, 6 * LightSphereColors.size(), &LightSphereColors.front(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1); //Sphere colors is position3
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//LightSphere Indices
	glGenBuffers(1, &LightSphere_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,LightSphere_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * LightSphereIndices.size(), &LightSphereIndices.front(), GL_DYNAMIC_DRAW);

	std::cout << "test" << std::endl;

	//LightSphereTexcoordinates
	glGenBuffers(1, &LightSphere_VBOtex);
	glBindBuffer(GL_ARRAY_BUFFER, LightSphere_VBOtex);
	glBufferData(GL_ARRAY_BUFFER, 4 * texCoords.size(), &texCoords.front(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);

	std::cout << "test" << std::endl;

	//Sun Texture
	
	unsigned int textureSun;
	glGenTextures(1, &textureSun);
	glBindTexture(GL_TEXTURE_2D, textureSun); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width2, height2, nrChannels2;
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char *data2 = stbi_load(std::string("2k_sun.jpg").c_str(), &width2, &height2, &nrChannels2, 0);
	std::cout << "test" << std::endl;
	if (data2)
	{
		std::cout << "test" << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
		std::cout << "test" << std::endl;
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data2);

	std::cout << "test" << std::endl;
	int i = 0;
	int lightsphereposition = 0;
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.0f, 0.5f, 0.0f, 1.0f); //green background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);
		
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
		

		glBindVertexArray(VAO);
		i++;
		
		if (i >= 60)
		{
			
			//mobius Farben
			float s = mobiusColors.at(0);
			float t = mobiusColors.at(1);
			float u = mobiusColors.at(2);
			float v = mobiusColors.at(3);
			float w = mobiusColors.at(4);
			float x = mobiusColors.at(5);
			float y = mobiusColors.at(6);
			float z = mobiusColors.at(7);
			mobiusColors.erase(mobiusColors.begin());
			mobiusColors.erase(mobiusColors.begin());
			mobiusColors.erase(mobiusColors.begin());
			mobiusColors.erase(mobiusColors.begin());
			mobiusColors.erase(mobiusColors.begin());
			mobiusColors.erase(mobiusColors.begin());
			mobiusColors.erase(mobiusColors.begin());
			mobiusColors.erase(mobiusColors.begin());
			mobiusColors.push_back(s);
			mobiusColors.push_back(t);
			mobiusColors.push_back(u);
			mobiusColors.push_back(v);
			mobiusColors.push_back(w);
			mobiusColors.push_back(x);
			mobiusColors.push_back(y);
			mobiusColors.push_back(z);
			glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
			glBufferData(GL_ARRAY_BUFFER, 6 * mobiusColors.size(), &mobiusColors.front(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
		}

		glUseProgram(shaderProgram);
		glDrawElements(GL_TRIANGLES, mobiusIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(sphere_VAO);

		glUseProgram(shaderTextureProgram);
		glUniformMatrix4fv(glGetUniformLocation(shaderTextureProgram, "view"), 1, GL_FALSE, &view[0][0]);

		glBindTexture(GL_TEXTURE_2D, textureEarth);
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

		
		glBindVertexArray(LightSphere_VAO);
		if (i >= 60)
		{
			//Umkreis der Sonne
			lightsphereposition = lightsphereposition + 3;
			AdjustVertexData(lightsphereposition, LightSphereCenters, LightSphereVertices);
			glBindBuffer(GL_ARRAY_BUFFER, LightSphere_VBOcoords);
			glBufferData(GL_ARRAY_BUFFER, 4 * LightSphereVertices.size(), &LightSphereVertices.front(), GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0); //Sphere is position2
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			if (lightsphereposition > LightSphereCenters.size() / 3) { lightsphereposition = 3; };
			i = 0;
		}
		
		glBindTexture(GL_TEXTURE_2D, textureSun);
		glDrawElements(GL_TRIANGLES, LightSphereIndices.size(), GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, LightSphere_VBOcoords);
		glBufferData(GL_ARRAY_BUFFER, 4 * LightSphereVertices.size(), &LightSphereVertices.front(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0); //Sphere is position2
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glUseProgram(0);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void AdjustVertexData(int lightposition, std::vector<float> &LightSphereCenters, std::vector<float> &LightSphere)
{
	std::vector<float> temp;
	for (unsigned int stackNumber = 0; stackNumber <= stacksLight; ++stackNumber)
	{
		for (unsigned int sliceNumber = 0; sliceNumber <= slicesLight; ++sliceNumber)
		{
			float theta = stackNumber * pi / stacksLight;
			float phi = sliceNumber * 2 * pi / slicesLight;
			float sinTheta = std::sin(theta);
			float sinPhi = std::sin(phi);
			float cosTheta = std::cos(theta);
			float cosPhi = std::cos(phi);
			temp.push_back(radiusLight * cosPhi * sinTheta + LightSphereCenters.at(lightposition));
			temp.push_back(radiusLight * sinPhi * sinTheta + LightSphereCenters.at(lightposition + 1));
			temp.push_back(radiusLight * cosTheta + LightSphereCenters.at(lightposition + 2));
		}
	}
	LightSphere = temp;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

std::vector<float> calculateMobiusVertices(int rootOfVertices) {
	std::vector<float> mobius;
	double a = 0;
	while (a < 2 * pi) {
		float u = cos(a)* (1 + (-0.5 * cos(a / 2)));
		mobius.push_back(u);

		float v = sin(a)* (1 + (-0.5 * cos(a / 2)));
		mobius.push_back(v);

		float w = -0.5 * sin(a / 2);
		mobius.push_back(w);

		float x = cos(a)* (1 + (0.5 * cos(a / 2)));
		mobius.push_back(x);

		float y = sin(a)* (1 + (0.5 * cos(a / 2)));
		mobius.push_back(y);

		float z = 0.5 * sin(a / 2);
		mobius.push_back(z);
		a = a + 0.2;

	}
	return mobius;
}


std::vector<float> calculateSphereVertices(int rootOfVertices) {
	std::vector<float> sphere;
	for (unsigned int stackNumber = 0; stackNumber <= stacks; ++stackNumber)
	{
		for (unsigned int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
		{
			float theta = stackNumber * pi / stacks;
			float phi = sliceNumber * 2 * pi / slices;
			float sinTheta = std::sin(theta);
			float sinPhi = std::sin(phi);
			float cosTheta = std::cos(theta);
			float cosPhi = std::cos(phi);
			sphere.push_back(radius * cosPhi * sinTheta);
			sphere.push_back(radius * sinPhi * sinTheta);
			sphere.push_back(radius * cosTheta);
		}
	}
	return sphere;
}

std::vector<float> calculateLightSphereCenters(int rootOfVertices) {
	std::vector<float> LightSphereCenters;
	float phi = 0;
	while (phi <= 6 * pi) {
		float x = kreisradius * sin(phi);
		float y = kreisradius * cos(phi);
		float z = kreisradius;
		LightSphereCenters.push_back(x);
		LightSphereCenters.push_back(y);
		LightSphereCenters.push_back(z);
		phi = phi + 0.1;
	}
	return LightSphereCenters;
}

std::vector<float> calculateLightSphereVertices(int rootOfVertices) {
	std::vector<float> LightSphere;
	for (unsigned int stackNumber = 0; stackNumber <= stacksLight; ++stackNumber)
	{
		for (unsigned int sliceNumber = 0; sliceNumber <= slicesLight; ++sliceNumber)
		{
			float theta = stackNumber * pi / stacksLight;
			float phi = sliceNumber * 2 * pi / slicesLight;
			float sinTheta = std::sin(theta);
			float sinPhi = std::sin(phi);
			float cosTheta = std::cos(theta);
			float cosPhi = std::cos(phi);
			LightSphere.push_back(radiusLight * cosPhi * sinTheta + kreisradius);
			LightSphere.push_back(radiusLight * sinPhi * sinTheta + kreisradius);
			LightSphere.push_back(radiusLight * cosTheta + kreisradius);
		}
	}
	return LightSphere;
}

std::vector<int> calculateMobiusIndices(int rootOfIndices) {
	std::vector<int> mobiusIndices;
	int i = 0;
	while (i < rootOfIndices) {
		mobiusIndices.push_back(i);
		i++;
		mobiusIndices.push_back(i);
		i++;
		if (i == 64) {
			i = 2;
			mobiusIndices.push_back(i);
			i = 62;
			mobiusIndices.push_back(i);
			i = 1;
			mobiusIndices.push_back(i);
			i = 2;
			mobiusIndices.push_back(i);
			break;
		}
		mobiusIndices.push_back(i);
		i--;
	}
	//correct rotation of normalized vectors
	i = 0;
	int temp = 0;
	while (i < mobiusIndices.size()) {
		i = i + 3;
		temp = mobiusIndices.at(i);
		mobiusIndices.at(i) = mobiusIndices.at(i + 1);
		mobiusIndices.at(i + 1) = temp;
		i = i + 3;
	};
	return mobiusIndices;
}

std::vector<int> calculateSphereIndices(int rootOfIndices) {
	std::vector<int> sphereIndices;
	for (unsigned int stackNumber = 0; stackNumber < stacks; ++stackNumber)
	{
		for (unsigned int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
		{
			sphereIndices.push_back((stackNumber * (slices + 1)) + sliceNumber);
			sphereIndices.push_back(((stackNumber + 1) * (slices + 1)) + sliceNumber);
		}
	}
	//create triangle out of triangle-strip 
	std::vector<int> sphereIndices2;
	int i = 0;

	while (i < sphereIndices.size()) {
		sphereIndices2.push_back(sphereIndices.at(i));
		i++;
		sphereIndices2.push_back(sphereIndices.at(i));
		i++;
		if (i >= sphereIndices.size()) {
			break;
		}
		sphereIndices2.push_back(sphereIndices.at(i));
		i--;
	}
	//correct rotation of normalized vectors
	i = 0;
	int temp = 0;
	while (i < sphereIndices2.size() - 3) {
		i = i + 3;
		temp = sphereIndices2.at(i);
		sphereIndices2.at(i) = sphereIndices2.at(i + 1);
		sphereIndices2.at(i + 1) = temp;
		i = i + 3;
	};
	return sphereIndices2;
}

std::vector<int> calculateLightSphereIndices(int rootOfIndices) {
	std::vector<int> sphereIndices;
	for (unsigned int stackNumber = 0; stackNumber < stacksLight; ++stackNumber)
	{
		for (unsigned int sliceNumber = 0; sliceNumber <= slicesLight; ++sliceNumber)
		{
			sphereIndices.push_back((stackNumber * (slicesLight + 1)) + sliceNumber);
			sphereIndices.push_back(((stackNumber + 1) * (slicesLight + 1)) + sliceNumber);
		}
	}
	//create triangle out of triangle-strip 
	std::vector<int> sphereIndices2;
	int i = 0;

	while (i < sphereIndices.size()) {
		sphereIndices2.push_back(sphereIndices.at(i));
		i++;
		sphereIndices2.push_back(sphereIndices.at(i));
		i++;
		if (i >= sphereIndices.size()) {
			break;
		}
		sphereIndices2.push_back(sphereIndices.at(i));
		i--;
	}
	//correct rotation of normalized vectors
	i = 0;
	int temp = 0;
	while (i < sphereIndices2.size() - 3) {
		i = i + 3;
		temp = sphereIndices2.at(i);
		sphereIndices2.at(i) = sphereIndices2.at(i + 1);
		sphereIndices2.at(i + 1) = temp;
		i = i + 3;
	};
	return sphereIndices2;
}






std::vector<float>calculateMobiusColors(int rootOfColors) {
	std::vector<float> mobiuscolors = {
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //10
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //20
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //30
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //40
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //50
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //60
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //70
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //80
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //90
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //100
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //110
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //120
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //130
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //140
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //150
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //160
	0.543f, 0.021f, 0.978f, 1.0f,
	0.279f, 0.317f, 0.505f, 1.0f,
	0.167f, 0.620f, 0.077f, 1.0f,
	0.347f, 0.857f, 0.137f, 1.0f,
	0.055f, 0.953f, 0.042f, 1.0f,
	0.714f, 0.505f, 0.345f, 1.0f,
	0.783f, 0.290f, 0.734f, 1.0f,
	0.722f, 0.645f, 0.174f, 1.0f,
	0.302f, 0.455f, 0.848f, 1.0f,
	0.225f, 0.587f, 0.040f, 1.0f, //170
	0.517f, 0.713f, 0.338f, 1.0f,
	0.053f, 0.959f, 0.120f, 1.0f,
	0.393f, 0.621f, 0.362f, 1.0f,
	0.673f, 0.211f, 0.457f, 1.0f,
	0.820f, 0.883f, 0.371f, 1.0f,
	0.982f, 0.099f, 0.879f, 1.0f,
	0.714f, 0.505f, 0.345f, 1.0f,
	0.783f, 0.290f, 0.734f, 1.0f,
	0.722f, 0.645f, 0.174f, 1.0f,
	0.302f, 0.455f, 0.848f, 1.0f, //180
	0.583f, 0.771f, 0.014f, 1.0f,
	0.609f, 0.115f, 0.436f, 1.0f,
	0.327f, 0.483f, 0.844f, 1.0f,
	0.822f, 0.569f, 0.201f, 1.0f,
	0.435f, 0.602f, 0.223f, 1.0f,
	0.310f, 0.747f, 0.185f, 1.0f,
	0.597f, 0.770f, 0.761f, 1.0f,
	0.559f, 0.436f, 0.730f, 1.0f,
	0.359f, 0.583f, 0.152f, 1.0f,
	0.310f, 0.747f, 0.185f, 1.0f, //190
	0.597f, 0.770f, 0.761f, 1.0f,
	0.559f, 0.436f, 0.730f, 1.0f };
	return mobiuscolors;
};

std::vector<float>calculateSphereColors(int rootOfColors) {
	std::vector<float> spherecolors = {
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //10
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //20
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //30
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //40
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //50
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //60
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //70
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //80
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //90
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //100
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //110
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //120
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //130
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //140
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //150
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //160
	0.543f, 0.021f, 0.978f, 1.0f,
	0.279f, 0.317f, 0.505f, 1.0f,
	0.167f, 0.620f, 0.077f, 1.0f,
	0.347f, 0.857f, 0.137f, 1.0f,
	0.055f, 0.953f, 0.042f, 1.0f,
	0.714f, 0.505f, 0.345f, 1.0f,
	0.783f, 0.290f, 0.734f, 1.0f,
	0.722f, 0.645f, 0.174f, 1.0f,
	0.302f, 0.455f, 0.848f, 1.0f,
	0.225f, 0.587f, 0.040f, 1.0f, //170
	0.517f, 0.713f, 0.338f, 1.0f,
	0.053f, 0.959f, 0.120f, 1.0f,
	0.393f, 0.621f, 0.362f, 1.0f,
	0.673f, 0.211f, 0.457f, 1.0f,
	0.820f, 0.883f, 0.371f, 1.0f,
	0.982f, 0.099f, 0.879f, 1.0f,
	0.714f, 0.505f, 0.345f, 1.0f,
	0.783f, 0.290f, 0.734f, 1.0f,
	0.722f, 0.645f, 0.174f, 1.0f,
	0.302f, 0.455f, 0.848f, 1.0f, //180
	0.583f, 0.771f, 0.014f, 1.0f,
	0.609f, 0.115f, 0.436f, 1.0f,
	0.327f, 0.483f, 0.844f, 1.0f,
	0.822f, 0.569f, 0.201f, 1.0f,
	0.435f, 0.602f, 0.223f, 1.0f,
	0.310f, 0.747f, 0.185f, 1.0f,
	0.597f, 0.770f, 0.761f, 1.0f,
	0.559f, 0.436f, 0.730f, 1.0f,
	0.359f, 0.583f, 0.152f, 1.0f,
	0.310f, 0.747f, 0.185f, 1.0f, //190
		0.583f, 0.771f, 0.014f, 1.0f,
		0.609f, 0.115f, 0.436f, 1.0f,
		0.327f, 0.483f, 0.844f, 1.0f,
		0.822f, 0.569f, 0.201f, 1.0f,
		0.435f, 0.602f, 0.223f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f,
		0.597f, 0.770f, 0.761f, 1.0f,
		0.559f, 0.436f, 0.730f, 1.0f,
		0.359f, 0.583f, 0.152f, 1.0f,
		0.483f, 0.596f, 0.789f, 1.0f, //200
		0.559f, 0.861f, 0.639f, 1.0f,
		0.195f, 0.548f, 0.859f, 1.0f,
		0.014f, 0.184f, 0.576f, 1.0f,
		0.771f, 0.328f, 0.970f, 1.0f,
		0.406f, 0.615f, 0.116f, 1.0f,
		0.676f, 0.977f, 0.133f, 1.0f,
		0.971f, 0.572f, 0.833f, 1.0f,
		0.140f, 0.616f, 0.489f, 1.0f,
		0.997f, 0.513f, 0.064f, 1.0f,
		0.945f, 0.719f, 0.592f, 1.0f, //210
		0.543f, 0.021f, 0.978f, 1.0f,
		0.279f, 0.317f, 0.505f, 1.0f,
		0.167f, 0.620f, 0.077f, 1.0f,
		0.347f, 0.857f, 0.137f, 1.0f,
		0.055f, 0.953f, 0.042f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f,
		0.225f, 0.587f, 0.040f, 1.0f, //220
		0.517f, 0.713f, 0.338f, 1.0f,
		0.053f, 0.959f, 0.120f, 1.0f,
		0.393f, 0.621f, 0.362f, 1.0f,
		0.673f, 0.211f, 0.457f, 1.0f,
		0.820f, 0.883f, 0.371f, 1.0f,
		0.982f, 0.099f, 0.879f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f, //230
		0.583f, 0.771f, 0.014f, 1.0f,
		0.609f, 0.115f, 0.436f, 1.0f,
		0.327f, 0.483f, 0.844f, 1.0f,
		0.822f, 0.569f, 0.201f, 1.0f,
		0.435f, 0.602f, 0.223f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f,
		0.597f, 0.770f, 0.761f, 1.0f,
		0.559f, 0.436f, 0.730f, 1.0f,
		0.359f, 0.583f, 0.152f, 1.0f,
		0.483f, 0.596f, 0.789f, 1.0f, //240
		0.559f, 0.861f, 0.639f, 1.0f,
		0.195f, 0.548f, 0.859f, 1.0f,
		0.014f, 0.184f, 0.576f, 1.0f,
		0.771f, 0.328f, 0.970f, 1.0f,
		0.406f, 0.615f, 0.116f, 1.0f,
		0.676f, 0.977f, 0.133f, 1.0f,
		0.971f, 0.572f, 0.833f, 1.0f,
		0.140f, 0.616f, 0.489f, 1.0f,
		0.997f, 0.513f, 0.064f, 1.0f,
		0.945f, 0.719f, 0.592f, 1.0f, //250
		0.543f, 0.021f, 0.978f, 1.0f,
		0.279f, 0.317f, 0.505f, 1.0f,
		0.167f, 0.620f, 0.077f, 1.0f,
		0.347f, 0.857f, 0.137f, 1.0f,
		0.055f, 0.953f, 0.042f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f,
		0.225f, 0.587f, 0.040f, 1.0f, //260
		0.517f, 0.713f, 0.338f, 1.0f,
		0.053f, 0.959f, 0.120f, 1.0f,
		0.393f, 0.621f, 0.362f, 1.0f,
		0.673f, 0.211f, 0.457f, 1.0f,
		0.820f, 0.883f, 0.371f, 1.0f,
		0.982f, 0.099f, 0.879f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f, //270
		0.583f, 0.771f, 0.014f, 1.0f,
		0.609f, 0.115f, 0.436f, 1.0f,
		0.327f, 0.483f, 0.844f, 1.0f,
		0.822f, 0.569f, 0.201f, 1.0f,
		0.435f, 0.602f, 0.223f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f,
		0.597f, 0.770f, 0.761f, 1.0f,
		0.559f, 0.436f, 0.730f, 1.0f,
		0.359f, 0.583f, 0.152f, 1.0f,
		0.483f, 0.596f, 0.789f, 1.0f, //280
		0.559f, 0.861f, 0.639f, 1.0f,
		0.195f, 0.548f, 0.859f, 1.0f,
		0.014f, 0.184f, 0.576f, 1.0f,
		0.771f, 0.328f, 0.970f, 1.0f,
		0.406f, 0.615f, 0.116f, 1.0f,
		0.676f, 0.977f, 0.133f, 1.0f,
		0.971f, 0.572f, 0.833f, 1.0f,
		0.140f, 0.616f, 0.489f, 1.0f,
		0.997f, 0.513f, 0.064f, 1.0f,
		0.945f, 0.719f, 0.592f, 1.0f, //290
		0.543f, 0.021f, 0.978f, 1.0f,
		0.279f, 0.317f, 0.505f, 1.0f,
		0.167f, 0.620f, 0.077f, 1.0f,
		0.347f, 0.857f, 0.137f, 1.0f,
		0.055f, 0.953f, 0.042f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f,
		0.225f, 0.587f, 0.040f, 1.0f, //300
		0.517f, 0.713f, 0.338f, 1.0f,
		0.053f, 0.959f, 0.120f, 1.0f,
		0.393f, 0.621f, 0.362f, 1.0f,
		0.673f, 0.211f, 0.457f, 1.0f,
		0.820f, 0.883f, 0.371f, 1.0f,
		0.982f, 0.099f, 0.879f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f, //310
		0.583f, 0.771f, 0.014f, 1.0f,
		0.609f, 0.115f, 0.436f, 1.0f,
		0.327f, 0.483f, 0.844f, 1.0f,
		0.822f, 0.569f, 0.201f, 1.0f,
		0.435f, 0.602f, 0.223f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f,
		0.597f, 0.770f, 0.761f, 1.0f,
		0.559f, 0.436f, 0.730f, 1.0f,
		0.359f, 0.583f, 0.152f, 1.0f,
		0.483f, 0.596f, 0.789f, 1.0f, //320
		0.559f, 0.861f, 0.639f, 1.0f,
		0.195f, 0.548f, 0.859f, 1.0f,
		0.014f, 0.184f, 0.576f, 1.0f,
		0.771f, 0.328f, 0.970f, 1.0f,
		0.406f, 0.615f, 0.116f, 1.0f,
		0.676f, 0.977f, 0.133f, 1.0f,
		0.971f, 0.572f, 0.833f, 1.0f,
		0.140f, 0.616f, 0.489f, 1.0f,
		0.997f, 0.513f, 0.064f, 1.0f,
		0.945f, 0.719f, 0.592f, 1.0f, //330
		0.543f, 0.021f, 0.978f, 1.0f,
		0.279f, 0.317f, 0.505f, 1.0f,
		0.167f, 0.620f, 0.077f, 1.0f,
		0.347f, 0.857f, 0.137f, 1.0f,
		0.055f, 0.953f, 0.042f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f,
		0.225f, 0.587f, 0.040f, 1.0f, //340
		0.517f, 0.713f, 0.338f, 1.0f,
		0.053f, 0.959f, 0.120f, 1.0f,
		0.393f, 0.621f, 0.362f, 1.0f,
		0.673f, 0.211f, 0.457f, 1.0f,
		0.820f, 0.883f, 0.371f, 1.0f,
		0.982f, 0.099f, 0.879f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f, //350
		0.543f, 0.021f, 0.978f, 1.0f,
		0.279f, 0.317f, 0.505f, 1.0f,
		0.167f, 0.620f, 0.077f, 1.0f,
		0.347f, 0.857f, 0.137f, 1.0f,
		0.055f, 0.953f, 0.042f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f,
		0.225f, 0.587f, 0.040f, 1.0f, //360
		0.517f, 0.713f, 0.338f, 1.0f,
		0.053f, 0.959f, 0.120f, 1.0f,
		0.393f, 0.621f, 0.362f, 1.0f,
		0.673f, 0.211f, 0.457f, 1.0f,
		0.820f, 0.883f, 0.371f, 1.0f,
		0.982f, 0.099f, 0.879f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f, //370
		0.583f, 0.771f, 0.014f, 1.0f,
		0.609f, 0.115f, 0.436f, 1.0f,
		0.327f, 0.483f, 0.844f, .0f,
		0.822f, 0.569f, 0.201f, 1.0f,
		0.435f, 0.602f, 0.223f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f,
		0.597f, 0.770f, 0.761f, 1.0f,
		0.559f, 0.436f, 0.730f, 1.0f,
		0.359f, 0.583f, 0.152f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f, //380 };
			0.483f, 0.596f, 0.789f, 1.0f, //390
			0.559f, 0.861f, 0.639f, 1.0f,
			0.195f, 0.548f, 0.859f, 1.0f,
			0.014f, 0.184f, 0.576f, 1.0f,
			0.771f, 0.328f, 0.970f, 1.0f,
			0.406f, 0.615f, 0.116f, 1.0f,
			0.676f, 0.977f, 0.133f, 1.0f,
			0.971f, 0.572f, 0.833f, 1.0f,
			0.140f, 0.616f, 0.489f, 1.0f,
			0.997f, 0.513f, 0.064f, 1.0f,
			0.945f, 0.719f, 0.592f, 1.0f, //400
			0.543f, 0.021f, 0.978f, 1.0f,
			0.279f, 0.317f, 0.505f, 1.0f,
			0.167f, 0.620f, 0.077f, 1.0f,
			0.347f, 0.857f, 0.137f, 1.0f,
			0.055f, 0.953f, 0.042f, 1.0f,
			0.714f, 0.505f, 0.345f, 1.0f,
			0.783f, 0.290f, 0.734f, 1.0f,
			0.722f, 0.645f, 0.174f, 1.0f,
			0.302f, 0.455f, 0.848f, 1.0f,
			0.225f, 0.587f, 0.040f, 1.0f, //410
			0.517f, 0.713f, 0.338f, 1.0f,
			0.053f, 0.959f, 0.120f, 1.0f,
			0.393f, 0.621f, 0.362f, 1.0f,
			0.673f, 0.211f, 0.457f, 1.0f,
			0.820f, 0.883f, 0.371f, 1.0f,
			0.982f, 0.099f, 0.879f, 1.0f,
			0.714f, 0.505f, 0.345f, 1.0f,
			0.783f, 0.290f, 0.734f, 1.0f,
			0.722f, 0.645f, 0.174f, 1.0f,
			0.302f, 0.455f, 0.848f, 1.0f, //420
			0.583f, 0.771f, 0.014f, 1.0f,
			0.609f, 0.115f, 0.436f, 1.0f,
			0.327f, 0.483f, 0.844f, 1.0f,
			0.822f, 0.569f, 0.201f, 1.0f,
			0.435f, 0.602f, 0.223f, 1.0f,
			0.310f, 0.747f, 0.185f, 1.0f,
			0.597f, 0.770f, 0.761f, 1.0f,
			0.559f, 0.436f, 0.730f, 1.0f,
			0.359f, 0.583f, 0.152f, 1.0f,
			0.483f, 0.596f, 0.789f, 1.0f, //430
			0.559f, 0.861f, 0.639f, 1.0f,
			0.195f, 0.548f, 0.859f, 1.0f,
			0.014f, 0.184f, 0.576f, 1.0f,
			0.771f, 0.328f, 0.970f, 1.0f,
			0.406f, 0.615f, 0.116f, 1.0f,
			0.676f, 0.977f, 0.133f, 1.0f,
			0.971f, 0.572f, 0.833f, 1.0f,
			0.140f, 0.616f, 0.489f, 1.0f,
			0.997f, 0.513f, 0.064f, 1.0f,
			0.945f, 0.719f, 0.592f, 1.0f, //440
			0.543f, 0.021f, 0.978f, 1.0f,
			0.279f, 0.317f, 0.505f, 1.0f,
			0.167f, 0.620f, 0.077f, 1.0f,
			0.347f, 0.857f, 0.137f, 1.0f,
			0.055f, 0.953f, 0.042f, 1.0f,
			0.714f, 0.505f, 0.345f, 1.0f,
			0.783f, 0.290f, 0.734f, 1.0f,
			0.722f, 0.645f, 0.174f, 1.0f,
			0.302f, 0.455f, 0.848f, 1.0f,
			0.225f, 0.587f, 0.040f, 1.0f, //450
			0.517f, 0.713f, 0.338f, 1.0f,
			0.053f, 0.959f, 0.120f, 1.0f,
			0.393f, 0.621f, 0.362f, 1.0f,
			0.673f, 0.211f, 0.457f, 1.0f,
			0.820f, 0.883f, 0.371f, 1.0f,
			0.982f, 0.099f, 0.879f, 1.0f,
			0.714f, 0.505f, 0.345f, 1.0f,
			0.783f, 0.290f, 0.734f, 1.0f,
			0.722f, 0.645f, 0.174f, 1.0f,
			0.302f, 0.455f, 0.848f, 1.0f, //460
			0.543f, 0.021f, 0.978f, 1.0f,
			0.279f, 0.317f, 0.505f, 1.0f,
			0.167f, 0.620f, 0.077f, 1.0f,
			0.347f, 0.857f, 0.137f, 1.0f,
			0.055f, 0.953f, 0.042f, 1.0f,
			0.714f, 0.505f, 0.345f, 1.0f,
			0.783f, 0.290f, 0.734f, 1.0f,
			0.722f, 0.645f, 0.174f, 1.0f,
			0.302f, 0.455f, 0.848f, 1.0f,
			0.225f, 0.587f, 0.040f, 1.0f, //470
			0.517f, 0.713f, 0.338f, 1.0f,
			0.053f, 0.959f, 0.120f, 1.0f };
	return spherecolors;
};


std::vector<float>calculateLightSphereColors(int rootOfColors) {
	std::vector<float> LightSpherecolors = {
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //10
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //20
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //30
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //40
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //50
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //60
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //70
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //80
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //90
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //100
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //110
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //120
	0.583f,  0.771f,  0.014f, 1.0f,
	0.609f,  0.115f,  0.436f, 1.0f,
	0.327f,  0.483f,  0.844f, 1.0f,
	0.822f,  0.569f,  0.201f, 1.0f,
	0.435f,  0.602f,  0.223f, 1.0f,
	0.310f,  0.747f,  0.185f, 1.0f,
	0.597f,  0.770f,  0.761f, 1.0f,
	0.559f,  0.436f,  0.730f, 1.0f,
	0.359f,  0.583f,  0.152f, 1.0f,
	0.483f,  0.596f,  0.789f, 1.0f, //130
	0.559f,  0.861f,  0.639f, 1.0f,
	0.195f,  0.548f,  0.859f, 1.0f,
	0.014f,  0.184f,  0.576f, 1.0f,
	0.771f,  0.328f,  0.970f, 1.0f,
	0.406f,  0.615f,  0.116f, 1.0f,
	0.676f,  0.977f,  0.133f, 1.0f,
	0.971f,  0.572f,  0.833f, 1.0f,
	0.140f,  0.616f,  0.489f, 1.0f,
	0.997f,  0.513f,  0.064f, 1.0f,
	0.945f,  0.719f,  0.592f, 1.0f, //140
	0.543f,  0.021f,  0.978f, 1.0f,
	0.279f,  0.317f,  0.505f, 1.0f,
	0.167f,  0.620f,  0.077f, 1.0f,
	0.347f,  0.857f,  0.137f, 1.0f,
	0.055f,  0.953f,  0.042f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f,
	0.225f,  0.587f,  0.040f, 1.0f, //150
	0.517f,  0.713f,  0.338f, 1.0f,
	0.053f,  0.959f,  0.120f, 1.0f,
	0.393f,  0.621f,  0.362f, 1.0f,
	0.673f,  0.211f,  0.457f, 1.0f,
	0.820f,  0.883f,  0.371f, 1.0f,
	0.982f,  0.099f,  0.879f, 1.0f,
	0.714f,  0.505f,  0.345f, 1.0f,
	0.783f,  0.290f,  0.734f, 1.0f,
	0.722f,  0.645f,  0.174f, 1.0f,
	0.302f,  0.455f,  0.848f, 1.0f, //160
	0.543f, 0.021f, 0.978f, 1.0f,
	0.279f, 0.317f, 0.505f, 1.0f,
	0.167f, 0.620f, 0.077f, 1.0f,
	0.347f, 0.857f, 0.137f, 1.0f,
	0.055f, 0.953f, 0.042f, 1.0f,
	0.714f, 0.505f, 0.345f, 1.0f,
	0.783f, 0.290f, 0.734f, 1.0f,
	0.722f, 0.645f, 0.174f, 1.0f,
	0.302f, 0.455f, 0.848f, 1.0f,
	0.225f, 0.587f, 0.040f, 1.0f, //170
	0.517f, 0.713f, 0.338f, 1.0f,
	0.053f, 0.959f, 0.120f, 1.0f,
	0.393f, 0.621f, 0.362f, 1.0f,
	0.673f, 0.211f, 0.457f, 1.0f,
	0.820f, 0.883f, 0.371f, 1.0f,
	0.982f, 0.099f, 0.879f, 1.0f,
	0.714f, 0.505f, 0.345f, 1.0f,
	0.783f, 0.290f, 0.734f, 1.0f,
	0.722f, 0.645f, 0.174f, 1.0f,
	0.302f, 0.455f, 0.848f, 1.0f, //180
	0.583f, 0.771f, 0.014f, 1.0f,
	0.609f, 0.115f, 0.436f, 1.0f,
	0.327f, 0.483f, 0.844f, 1.0f,
	0.822f, 0.569f, 0.201f, 1.0f,
	0.435f, 0.602f, 0.223f, 1.0f,
	0.310f, 0.747f, 0.185f, 1.0f,
	0.597f, 0.770f, 0.761f, 1.0f,
	0.559f, 0.436f, 0.730f, 1.0f,
	0.359f, 0.583f, 0.152f, 1.0f,
	0.310f, 0.747f, 0.185f, 1.0f, //190
		0.583f, 0.771f, 0.014f, 1.0f,
		0.609f, 0.115f, 0.436f, 1.0f,
		0.327f, 0.483f, 0.844f, 1.0f,
		0.822f, 0.569f, 0.201f, 1.0f,
		0.435f, 0.602f, 0.223f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f,
		0.597f, 0.770f, 0.761f, 1.0f,
		0.559f, 0.436f, 0.730f, 1.0f,
		0.359f, 0.583f, 0.152f, 1.0f,
		0.483f, 0.596f, 0.789f, 1.0f, //200
		0.559f, 0.861f, 0.639f, 1.0f,
		0.195f, 0.548f, 0.859f, 1.0f,
		0.014f, 0.184f, 0.576f, 1.0f,
		0.771f, 0.328f, 0.970f, 1.0f,
		0.406f, 0.615f, 0.116f, 1.0f,
		0.676f, 0.977f, 0.133f, 1.0f,
		0.971f, 0.572f, 0.833f, 1.0f,
		0.140f, 0.616f, 0.489f, 1.0f,
		0.997f, 0.513f, 0.064f, 1.0f,
		0.945f, 0.719f, 0.592f, 1.0f, //210
		0.543f, 0.021f, 0.978f, 1.0f,
		0.279f, 0.317f, 0.505f, 1.0f,
		0.167f, 0.620f, 0.077f, 1.0f,
		0.347f, 0.857f, 0.137f, 1.0f,
		0.055f, 0.953f, 0.042f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f,
		0.225f, 0.587f, 0.040f, 1.0f, //220
		0.517f, 0.713f, 0.338f, 1.0f,
		0.053f, 0.959f, 0.120f, 1.0f,
		0.393f, 0.621f, 0.362f, 1.0f,
		0.673f, 0.211f, 0.457f, 1.0f,
		0.820f, 0.883f, 0.371f, 1.0f,
		0.982f, 0.099f, 0.879f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f, //230
		0.583f, 0.771f, 0.014f, 1.0f,
		0.609f, 0.115f, 0.436f, 1.0f,
		0.327f, 0.483f, 0.844f, 1.0f,
		0.822f, 0.569f, 0.201f, 1.0f,
		0.435f, 0.602f, 0.223f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f,
		0.597f, 0.770f, 0.761f, 1.0f,
		0.559f, 0.436f, 0.730f, 1.0f,
		0.359f, 0.583f, 0.152f, 1.0f,
		0.483f, 0.596f, 0.789f, 1.0f, //240
		0.559f, 0.861f, 0.639f, 1.0f,
		0.195f, 0.548f, 0.859f, 1.0f,
		0.014f, 0.184f, 0.576f, 1.0f,
		0.771f, 0.328f, 0.970f, 1.0f,
		0.406f, 0.615f, 0.116f, 1.0f,
		0.676f, 0.977f, 0.133f, 1.0f,
		0.971f, 0.572f, 0.833f, 1.0f,
		0.140f, 0.616f, 0.489f, 1.0f,
		0.997f, 0.513f, 0.064f, 1.0f,
		0.945f, 0.719f, 0.592f, 1.0f, //250
		0.543f, 0.021f, 0.978f, 1.0f,
		0.279f, 0.317f, 0.505f, 1.0f,
		0.167f, 0.620f, 0.077f, 1.0f,
		0.347f, 0.857f, 0.137f, 1.0f,
		0.055f, 0.953f, 0.042f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f,
		0.225f, 0.587f, 0.040f, 1.0f, //260
		0.517f, 0.713f, 0.338f, 1.0f,
		0.053f, 0.959f, 0.120f, 1.0f,
		0.393f, 0.621f, 0.362f, 1.0f,
		0.673f, 0.211f, 0.457f, 1.0f,
		0.820f, 0.883f, 0.371f, 1.0f,
		0.982f, 0.099f, 0.879f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f, //270
		0.583f, 0.771f, 0.014f, 1.0f,
		0.609f, 0.115f, 0.436f, 1.0f,
		0.327f, 0.483f, 0.844f, 1.0f,
		0.822f, 0.569f, 0.201f, 1.0f,
		0.435f, 0.602f, 0.223f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f,
		0.597f, 0.770f, 0.761f, 1.0f,
		0.559f, 0.436f, 0.730f, 1.0f,
		0.359f, 0.583f, 0.152f, 1.0f,
		0.483f, 0.596f, 0.789f, 1.0f, //280
		0.559f, 0.861f, 0.639f, 1.0f,
		0.195f, 0.548f, 0.859f, 1.0f,
		0.014f, 0.184f, 0.576f, 1.0f,
		0.771f, 0.328f, 0.970f, 1.0f,
		0.406f, 0.615f, 0.116f, 1.0f,
		0.676f, 0.977f, 0.133f, 1.0f,
		0.971f, 0.572f, 0.833f, 1.0f,
		0.140f, 0.616f, 0.489f, 1.0f,
		0.997f, 0.513f, 0.064f, 1.0f,
		0.945f, 0.719f, 0.592f, 1.0f, //290
		0.543f, 0.021f, 0.978f, 1.0f,
		0.279f, 0.317f, 0.505f, 1.0f,
		0.167f, 0.620f, 0.077f, 1.0f,
		0.347f, 0.857f, 0.137f, 1.0f,
		0.055f, 0.953f, 0.042f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f,
		0.225f, 0.587f, 0.040f, 1.0f, //300
		0.517f, 0.713f, 0.338f, 1.0f,
		0.053f, 0.959f, 0.120f, 1.0f,
		0.393f, 0.621f, 0.362f, 1.0f,
		0.673f, 0.211f, 0.457f, 1.0f,
		0.820f, 0.883f, 0.371f, 1.0f,
		0.982f, 0.099f, 0.879f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f, //310
		0.583f, 0.771f, 0.014f, 1.0f,
		0.609f, 0.115f, 0.436f, 1.0f,
		0.327f, 0.483f, 0.844f, 1.0f,
		0.822f, 0.569f, 0.201f, 1.0f,
		0.435f, 0.602f, 0.223f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f,
		0.597f, 0.770f, 0.761f, 1.0f,
		0.559f, 0.436f, 0.730f, 1.0f,
		0.359f, 0.583f, 0.152f, 1.0f,
		0.483f, 0.596f, 0.789f, 1.0f, //320
		0.559f, 0.861f, 0.639f, 1.0f,
		0.195f, 0.548f, 0.859f, 1.0f,
		0.014f, 0.184f, 0.576f, 1.0f,
		0.771f, 0.328f, 0.970f, 1.0f,
		0.406f, 0.615f, 0.116f, 1.0f,
		0.676f, 0.977f, 0.133f, 1.0f,
		0.971f, 0.572f, 0.833f, 1.0f,
		0.140f, 0.616f, 0.489f, 1.0f,
		0.997f, 0.513f, 0.064f, 1.0f,
		0.945f, 0.719f, 0.592f, 1.0f, //330
		0.543f, 0.021f, 0.978f, 1.0f,
		0.279f, 0.317f, 0.505f, 1.0f,
		0.167f, 0.620f, 0.077f, 1.0f,
		0.347f, 0.857f, 0.137f, 1.0f,
		0.055f, 0.953f, 0.042f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f,
		0.225f, 0.587f, 0.040f, 1.0f, //340
		0.517f, 0.713f, 0.338f, 1.0f,
		0.053f, 0.959f, 0.120f, 1.0f,
		0.393f, 0.621f, 0.362f, 1.0f,
		0.673f, 0.211f, 0.457f, 1.0f,
		0.820f, 0.883f, 0.371f, 1.0f,
		0.982f, 0.099f, 0.879f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f, //350
		0.543f, 0.021f, 0.978f, 1.0f,
		0.279f, 0.317f, 0.505f, 1.0f,
		0.167f, 0.620f, 0.077f, 1.0f,
		0.347f, 0.857f, 0.137f, 1.0f,
		0.055f, 0.953f, 0.042f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f,
		0.225f, 0.587f, 0.040f, 1.0f, //360
		0.517f, 0.713f, 0.338f, 1.0f,
		0.053f, 0.959f, 0.120f, 1.0f,
		0.393f, 0.621f, 0.362f, 1.0f,
		0.673f, 0.211f, 0.457f, 1.0f,
		0.820f, 0.883f, 0.371f, 1.0f,
		0.982f, 0.099f, 0.879f, 1.0f,
		0.714f, 0.505f, 0.345f, 1.0f,
		0.783f, 0.290f, 0.734f, 1.0f,
		0.722f, 0.645f, 0.174f, 1.0f,
		0.302f, 0.455f, 0.848f, 1.0f, //370
		0.583f, 0.771f, 0.014f, 1.0f,
		0.609f, 0.115f, 0.436f, 1.0f,
		0.327f, 0.483f, 0.844f, .0f,
		0.822f, 0.569f, 0.201f, 1.0f,
		0.435f, 0.602f, 0.223f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f,
		0.597f, 0.770f, 0.761f, 1.0f,
		0.559f, 0.436f, 0.730f, 1.0f,
		0.359f, 0.583f, 0.152f, 1.0f,
		0.310f, 0.747f, 0.185f, 1.0f, //380 };
			0.483f, 0.596f, 0.789f, 1.0f, //390
			0.559f, 0.861f, 0.639f, 1.0f,
			0.195f, 0.548f, 0.859f, 1.0f,
			0.014f, 0.184f, 0.576f, 1.0f,
			0.771f, 0.328f, 0.970f, 1.0f,
			0.406f, 0.615f, 0.116f, 1.0f,
			0.676f, 0.977f, 0.133f, 1.0f,
			0.971f, 0.572f, 0.833f, 1.0f,
			0.140f, 0.616f, 0.489f, 1.0f,
			0.997f, 0.513f, 0.064f, 1.0f,
			0.945f, 0.719f, 0.592f, 1.0f, //400
			0.543f, 0.021f, 0.978f, 1.0f,
			0.279f, 0.317f, 0.505f, 1.0f,
			0.167f, 0.620f, 0.077f, 1.0f,
			0.347f, 0.857f, 0.137f, 1.0f,
			0.055f, 0.953f, 0.042f, 1.0f,
			0.714f, 0.505f, 0.345f, 1.0f,
			0.783f, 0.290f, 0.734f, 1.0f,
			0.722f, 0.645f, 0.174f, 1.0f,
			0.302f, 0.455f, 0.848f, 1.0f,
			0.225f, 0.587f, 0.040f, 1.0f, //410
			0.517f, 0.713f, 0.338f, 1.0f,
			0.053f, 0.959f, 0.120f, 1.0f,
			0.393f, 0.621f, 0.362f, 1.0f,
			0.673f, 0.211f, 0.457f, 1.0f,
			0.820f, 0.883f, 0.371f, 1.0f,
			0.982f, 0.099f, 0.879f, 1.0f,
			0.714f, 0.505f, 0.345f, 1.0f,
			0.783f, 0.290f, 0.734f, 1.0f,
			0.722f, 0.645f, 0.174f, 1.0f,
			0.302f, 0.455f, 0.848f, 1.0f, //420
			0.583f, 0.771f, 0.014f, 1.0f,
			0.609f, 0.115f, 0.436f, 1.0f,
			0.327f, 0.483f, 0.844f, 1.0f,
			0.822f, 0.569f, 0.201f, 1.0f,
			0.435f, 0.602f, 0.223f, 1.0f,
			0.310f, 0.747f, 0.185f, 1.0f,
			0.597f, 0.770f, 0.761f, 1.0f,
			0.559f, 0.436f, 0.730f, 1.0f,
			0.359f, 0.583f, 0.152f, 1.0f,
			0.483f, 0.596f, 0.789f, 1.0f, //430
			0.559f, 0.861f, 0.639f, 1.0f,
			0.195f, 0.548f, 0.859f, 1.0f,
			0.014f, 0.184f, 0.576f, 1.0f,
			0.771f, 0.328f, 0.970f, 1.0f,
			0.406f, 0.615f, 0.116f, 1.0f,
			0.676f, 0.977f, 0.133f, 1.0f,
			0.971f, 0.572f, 0.833f, 1.0f,
			0.140f, 0.616f, 0.489f, 1.0f,
			0.997f, 0.513f, 0.064f, 1.0f,
			0.945f, 0.719f, 0.592f, 1.0f, //440
			0.543f, 0.021f, 0.978f, 1.0f,
			0.279f, 0.317f, 0.505f, 1.0f,
			0.167f, 0.620f, 0.077f, 1.0f,
			0.347f, 0.857f, 0.137f, 1.0f,
			0.055f, 0.953f, 0.042f, 1.0f,
			0.714f, 0.505f, 0.345f, 1.0f,
			0.783f, 0.290f, 0.734f, 1.0f,
			0.722f, 0.645f, 0.174f, 1.0f,
			0.302f, 0.455f, 0.848f, 1.0f,
			0.225f, 0.587f, 0.040f, 1.0f, //450
			0.517f, 0.713f, 0.338f, 1.0f,
			0.053f, 0.959f, 0.120f, 1.0f,
			0.393f, 0.621f, 0.362f, 1.0f,
			0.673f, 0.211f, 0.457f, 1.0f,
			0.820f, 0.883f, 0.371f, 1.0f,
			0.982f, 0.099f, 0.879f, 1.0f,
			0.714f, 0.505f, 0.345f, 1.0f,
			0.783f, 0.290f, 0.734f, 1.0f,
			0.722f, 0.645f, 0.174f, 1.0f,
			0.302f, 0.455f, 0.848f, 1.0f, //460
			0.543f, 0.021f, 0.978f, 1.0f,
			0.279f, 0.317f, 0.505f, 1.0f,
			0.167f, 0.620f, 0.077f, 1.0f,
			0.347f, 0.857f, 0.137f, 1.0f,
			0.055f, 0.953f, 0.042f, 1.0f,
			0.714f, 0.505f, 0.345f, 1.0f,
			0.783f, 0.290f, 0.734f, 1.0f,
			0.722f, 0.645f, 0.174f, 1.0f,
			0.302f, 0.455f, 0.848f, 1.0f,
			0.225f, 0.587f, 0.040f, 1.0f, //470
			0.517f, 0.713f, 0.338f, 1.0f,
			0.053f, 0.959f, 0.120f, 1.0f };
	return LightSpherecolors;
};

std::vector<float> GenerateSphereTexCoordinates()
{
	std::vector<float> TexCoord;
	for (int i = 0; i <= stacks;i++)
	{
		for(int j = 0; j <= slices;j++)
		{
			TexCoord.push_back((j*1.0f)/(slices*1.0f));
			TexCoord.push_back(1.0f - (i*1.0f) / (stacks*1.0f));
		}
	}
	return TexCoord;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		pitch += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		pitch -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		yaw -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		yaw += 0.1f;


	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	glm::vec3 up;
	up.x = 0;
	up.y = cos(glm::radians(pitch));
	up.z = sin(glm::radians(pitch));
	cameraUp = glm::normalize(up);
}