#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <math.h>

const GLchar* vertexShaderSource =
"#version 440 core\n"
"layout(location = 0) in vec3 aPos;"
"layout(location = 1) in vec3 aColor;"
"layout(location = 2) in vec3 aNormal;"
""
"flat out vec4 ourColor;"
"flat out vec3 ourNormal;"
""
"void main()"
"{"
"	gl_Position = vec4(aPos, 1.0);"
"	ourColor = vec4(aColor, 1.0);"
"	ourNormal = vec3(aNormal);"
"}" ;

const GLchar* fragmentShaderSource =
"#version 440 core\n"
"out vec4 FragColor;"
""
"flat in vec4 ourColor;"
"flat in vec3 ourNormal;"
""
"void main()"
"{"
"	FragColor = ourColor;"
"}";

float pi = 3.14159265358979323846;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow *window);

std::vector<float> calculateMobius(int rootOfVertices);

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


	glViewport(0, 0, 600, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	

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



	std::vector<float> mobiusVertices = calculateMobius(100); //Mobius with (100+1)^2 vertices 


	std::vector<int> mobiusIndices;

	std::vector<float> mobiusNormals;

	unsigned int EBO;
	glGenBuffers(1, &EBO);

	unsigned int VBO[2];
	glGenBuffers(2, VBO);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mobiusVertices), &mobiusVertices.front(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mobiusNormals), &mobiusNormals.front(), GL_STATIC_DRAW);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mobiusIndices), &mobiusIndices.front(), GL_STATIC_DRAW);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClearColor(0.0f, 0.5f, 0.0f, 1.0f); //green background
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

std::vector<float> calculateMobius(int rootOfVertices) {
	std::vector<float> mobius;

	for (int i = 0; i < rootOfVertices; i++) {
		float u = ((2 * pi * i )/  rootOfVertices);
		float v = (( 2* i) / rootOfVertices) - 1;

		float x = (1 + (v / 2) * cos(u / 2)) * cos(u);
		float y = (1 + (v / 2) * cos(u / 2)) * sin(u);
		float z = (v / 2) * sin(u / 2);

		mobius.push_back(x);
		mobius.push_back(y);
		mobius.push_back(z);
	}

	return mobius;
}