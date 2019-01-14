

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>





#include <iostream>
#include <vector>
#include <math.h>
#include <stdio.h>

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

int screenWidth = 600;
int screenHeight = 600;

float pi = 3.14159265358979323846;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow *window);

std::vector<float> calculateMobius(int rootOfVertices);
std::vector<int> calculateIndices(int rootOfIndices);
std::vector<float>g_color_buffer_data(int rootOfColors);

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

	//camera postition
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

	//focus of the camera
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

	// right axis of the camera
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

	// up axis
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);


	//camera matrix
	glm::mat4 view;
	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	//setting initial camera
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);

	//perpective
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &proj[0][0]);

	std::vector<float> mobiusVertices = calculateMobius(64 * 3); //Mobius with (100+1)^2 vertices 


	std::vector<int> mobiusIndices = calculateIndices(64 * 3);
	std::vector<float> mobiusColors = g_color_buffer_data(192 * 4);


	// ids for object
	GLuint  VAO;
	GLuint  EBO;
	GLuint  VBOcoords;
	GLint   m_iColorAttribPosition0;

	// create vertex array object
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);


	// create coords object
	glGenBuffers(1, &VBOcoords);
	glBindBuffer(GL_ARRAY_BUFFER, VBOcoords);
	glBufferData(GL_ARRAY_BUFFER, 4 * 192, &mobiusVertices.front(), GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// create color buffer
	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * 192, &mobiusColors.front(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// create buffer object for indices
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * 192, &mobiusIndices.front(), GL_STATIC_DRAW);

	std::cout << "test" << std::endl;

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClearColor(0.0f, 0.5f, 0.0f, 1.0f); //green background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);
		//
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 192, GL_UNSIGNED_INT, 0);
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

std::vector<int> calculateIndices(int rootOfIndices) {
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


	return mobiusIndices;
}

std::vector<float>g_color_buffer_data(int rootOfColors) {
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