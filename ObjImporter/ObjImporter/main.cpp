#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "Vertex.h"
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GL/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ObjImporter
{
public:
	ObjImporter(GLFWwindow * renderwindow)
	{
		window = renderwindow;

		Initialize();
		// Program loop
		while (!glfwWindowShouldClose(window))
		{
			// Check and call events
			glfwPollEvents();

			Render();
		}

		glfwTerminate();
	}

	void compile_shaders()
	{
		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint program;

		static const GLchar * vertex_shader_source[] =
		{
			"#version 330 core									\n"
			"                                                   \n"
			"layout (location = 0) in vec3 position;			\n"
			"uniform vec4 color;								\n"
			"uniform mat4 model;                                \n"
			"uniform mat4 view;                                 \n"
			"uniform mat4 projection;                           \n"
			"                                                   \n"
			"                                                   \n"
			"out vec4 vs_color;									\n"
			"void main()									    \n"
			"{													\n"
			"	gl_Position = projection * view * model * vec4(position,1.0f);\n"
			"	vs_color = color;								\n"
			"}													\n"
		};

		static const GLchar * fragment_shader_source[] =
		{
			"#version 330 core									\n"
			"in  vec4 vs_color;									\n"
			"out vec4 color;                                    \n"
			"													\n"
			"void main()                                        \n"
			"{                                                  \n"
			"	color = vs_color;					    		\n"
			"}													\n"
		};

		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		shader_program = glCreateProgram();
		glAttachShader(shader_program, vertex_shader);
		glAttachShader(shader_program, fragment_shader);
		glLinkProgram(shader_program);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		GLint vertexsuccess;
		GLchar infoLog[512];
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertexsuccess);

		if (!vertexsuccess)
		{
			glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		GLint fragmentsuccess;
		GLchar infoLog2[512];
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragmentsuccess);

		if (!fragmentsuccess)
		{
			glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog2);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog2 << std::endl;
		}
	}

	void Initialize()
	{
		compile_shaders();
		ReadFile("model1.obj");
		CreateVAO();
	}

	void Render()
	{
		const GLfloat color[] = { 0.0, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, 0, color);

		glUseProgram(shader_program);
		glBindVertexArray(VAO);

		//Transformation matrices
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 model;
		view = glm::translate(view, glm::vec3(3.0f, -3.0f, -10.0f));
		projection = glm::perspective(45.0f, (float)1024 / (float)768, 0.1f, 1000.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

		GLint modelLoc = glGetUniformLocation(shader_program, "model");
		GLint viewLoc = glGetUniformLocation(shader_program, "view");
		GLint projLoc = glGetUniformLocation(shader_program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Color
		GLuint color_location = glGetUniformLocation(shader_program, "color");
		float colorData[] = { 1.0f, 0.3f, 0.3f, 1.0f };
		glUniform4fv(color_location, 1, colorData);

		glDrawElements(GL_TRIANGLES, indices.size()/3, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	void ReadFile(char* filename)
	{
		
		std::ifstream objFile;
		objFile.open(filename);
		while (!objFile.eof())
		{
			std::string s;
			getline(objFile, s);

			std::string data[4];
			int i = 0;
			std::stringstream ssin(s);
			while (ssin.good() && i < 4)
			{
				ssin >> data[i];
				++i;
			}

			if (data[0] == "v")
			{
				vertices.push_back(std::stof(data[1]));
				vertices.push_back(std::stof(data[2]));
				vertices.push_back(std::stof(data[3]));
				//vertices.push_back(Vertex(std::stof(data[1]), std::stof(data[2]), std::stof(data[3])));
			}
			if (data[0] == "f")
			{
				indices.push_back(std::stoi(data[1])-1);
				indices.push_back(std::stoi(data[2])-1);
				indices.push_back(std::stoi(data[3])-1);
			}
		}
	}

	void CreateVAO()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
	}


private:

	GLFWwindow * window;
	GLuint shader_program;
	GLuint VAO, VBO, EBO;

	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;
};

// The MAIN function, from here we start our application and run our Program/Game loop
int main()
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(1024, 768, "LearnOpenGL", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, 1024, 768);

	ObjImporter objImporter(window);

	return 0;
}