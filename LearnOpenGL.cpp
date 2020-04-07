#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stb_image.h>
#include <vector>
#include <string>


#include "Shader.h"
#include "Camera.h"

void frame_buffer_size_callback(GLFWwindow* window, int width, int height);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void process_input(GLFWwindow* window);

unsigned int loadTexture(const char* path);

unsigned int loadCubemap(const std::vector<std::string>& faces);

/* settings */
const auto scr_with = 1280;

const auto scr_height = 720;

/* camera */
Camera camera(glm::vec3(0.f, 0.f, 3.f));

auto lastX = scr_with / 2.f;

auto lastY = scr_height / 2.f;

auto bIsFirstMouse = true;

/* timing */
/* time between current frame and last frame */
auto deltaTime = 0.f;

auto lastFrame = 0.f;

int main()
{
	/* glfw: initialize and configure */
	// ------------------------------
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* glfw: window creation */
	// ------------------------------
	const auto window = glfwCreateWindow(scr_with, scr_height, "LearnOpenGL", nullptr, nullptr);

	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;

		glfwTerminate();

		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, frame_buffer_size_callback);

	glfwSetCursorPosCallback(window, mouse_callback);

	// glfwSetScrollCallback(window, scroll_callback);

	/* tell GLFW to capture our mouse */
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	/* glad: load all OpenGL function pointers */
	// ------------------------------
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	/* configure global opengl state */
	// ------------------------------
	glEnable(GL_DEPTH_TEST);

	/* build and compile our shader program */
	// ------------------------------
	const Shader shaderRed("Shaders/8.advanced_glsl.vs", "Shaders/8.red.fs");

	const Shader shaderGreen("Shaders/8.advanced_glsl.vs", "Shaders/8.green.fs");

	const Shader shaderBlue("Shaders/8.advanced_glsl.vs", "Shaders/8.blue.fs");

	const Shader shaderYellow("Shaders/8.advanced_glsl.vs", "Shaders/8.yellow.fs");

	/* set up vertex data (and buffer(s)) and configure vertex attributes */
	// ------------------------------
	float cubeVertices[] = {
		/* positions */
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,

		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,

		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,

		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
	};

	/* cube VAO */
	unsigned int cubeVAO, cubeVBO;

	glGenVertexArrays(1, &cubeVAO);

	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	/* configure a uniform buffer object */
	// ------------------------------
	/* first. We get the relevant block indices */
	const auto uniformBlockIndexRed = glGetUniformBlockIndex(shaderRed.ID, "Matrices");

	const auto uniformBlockIndexGreen = glGetUniformBlockIndex(shaderGreen.ID, "Matrices");

	const auto uniformBlockIndexBlue = glGetUniformBlockIndex(shaderBlue.ID, "Matrices");

	const auto uniformBlockIndexYellow = glGetUniformBlockIndex(shaderYellow.ID, "Matrices");

	/* then we link each shader's uniform block to this uniform binding point */
	glUniformBlockBinding(shaderRed.ID, uniformBlockIndexRed, 0);

	glUniformBlockBinding(shaderGreen.ID, uniformBlockIndexGreen, 0);

	glUniformBlockBinding(shaderBlue.ID, uniformBlockIndexBlue, 0);

	glUniformBlockBinding(shaderYellow.ID, uniformBlockIndexYellow, 0);

	/* Now actually create the buffer */
	unsigned int uboMatrices;

	glGenBuffers(1, &uboMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	/* define the range of the buffer that links to a uniform binding point */
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

	/* store the projection matrix (we only do this once now) (note: we're not using zoom anymore by changing the FoV) */
	auto projection = glm::perspective(45.f, static_cast<float>(scr_with) / scr_height, 0.1f, 100.f);

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), value_ptr(projection));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	/* render loop */
	// ------------------------------
	while (!glfwWindowShouldClose(window))
	{
		/* per-frame time logic */
		// ------------------------------
		const auto currentFrame = glfwGetTime();

		deltaTime = currentFrame - lastFrame;

		lastFrame = currentFrame;

		/* input */
		// ------------------------------
		process_input(window);

		/* render */
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* set the view and projection matrix in the uniform block - we only have to do this once per loop iteration. */
		auto view = camera.GetViewMatrix();

		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), value_ptr(view));

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		/* draw 4 cubes  */
		// ------------------------------
		/* RED */
		glBindVertexArray(cubeVAO);

		shaderRed.use();

		auto model = glm::mat4(1.f);

		/* move top-left */
		model = translate(model, glm::vec3(-0.75f, 0.75f, 0.f));

		shaderRed.setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		/* GREEN */
		shaderGreen.use();

		model = glm::mat4(1.f);

		/* move top-right */
		model = translate(model, glm::vec3(0.75f, 0.75f, 0.f));

		shaderGreen.setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		/* YELLOW */
		shaderYellow.use();

		model = glm::mat4(1.f);

		/* move bottom-left */
		model = translate(model, glm::vec3(-0.75f, -0.75f, 0.f));

		shaderYellow.setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		/* BLUE */
		shaderBlue.use();

		model = glm::mat4(1.f);

		/* move bottom-right */
		model = translate(model, glm::vec3(0.75f, -0.75f, 0.f));

		shaderBlue.setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		/* glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.) */
		// ------------------------------
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	/* optional: de-allocate all resources once they've outlived their purpose: */
	// ------------------------------
	glDeleteVertexArrays(1, &cubeVAO);

	glDeleteBuffers(1, &cubeVBO);

	/* glfw: terminate, clearing all previously allocated GLFW resources. */
	// ------------------------------
	glfwTerminate();

	return 0;
}

/* process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly */
// ------------------------------
void process_input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
	}
}

/* glfw: whenever the window size changed (by OS or user resize) this callback function executes */
// ------------------------------
void frame_buffer_size_callback(GLFWwindow* window, const int width, const int height)
{
	/*
	 * make sure the viewport matches the new window dimensions; note that width and
	 * height will be significantly larger than specified on retina displays.
	 */
	glViewport(0, 0, width, height);
}

/* glfw: whenever the mouse moves, this callback is called */
// ------------------------------
void mouse_callback(GLFWwindow* window, const double xpos, const double ypos)
{
	if (bIsFirstMouse)
	{
		lastX = xpos;

		lastY = ypos;

		bIsFirstMouse = false;
	}

	const auto xoffset = xpos - lastX;

	/* reversed since y-coordinates go from bottom to top */
	const auto yoffset = lastY - ypos;

	lastX = xpos;

	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

/* glfw: whenever the mouse scroll wheel scrolls, this callback is called */
// ------------------------------
void scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

/* utility function for loading a 2D texture from file */
// ------------------------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;

	glGenTextures(1, &textureID);

	int width, height, nrComponents;

	const auto data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format;

		if (nrComponents == 1)
		{
			format = GL_RED;
		}
		else if (nrComponents == 3)
		{
			format = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;

		stbi_image_free(data);
	}

	return textureID;
}

/* loads a cubemap texture from 6 individual texture faces */
// ------------------------------
/*
 * order:
 * +X (right)
 * -X (left)
 * +Y (top)
 * -Y (bottom)
 * +Z (front)
 * -Z (back)
 */
unsigned int loadCubemap(const std::vector<std::string>& faces)
{
	unsigned int textureID;

	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;

	for (auto i = 0u; i < faces.size(); ++i)
	{
		const auto data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			             GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;

			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
