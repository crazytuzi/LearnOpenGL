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
#include "Model.h"

void frame_buffer_size_callback(GLFWwindow* window, int width, int height);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void process_input(GLFWwindow* window);

unsigned int loadTexture(const char* path, bool gammaCorrection = false);

unsigned int loadCubemap(const std::vector<std::string>& faces);

void renderQuad();

/* settings */
const auto scr_width = 1280;

const auto scr_height = 720;

auto heightScale = 0.1f;

/* camera */
Camera camera(glm::vec3(0.f, 0.f, 3.f));

auto lastX = scr_width / 2.f;

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
	const auto window = glfwCreateWindow(scr_width, scr_height, "LearnOpenGL", nullptr, nullptr);

	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;

		glfwTerminate();

		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, frame_buffer_size_callback);

	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetScrollCallback(window, scroll_callback);

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
	const Shader shader("Shaders/5.1.parallax_mapping.vs", "Shaders/5.3.parallax_mapping.fs");

	/* load textures */
	// ------------------------------
	const auto diffuseMap = loadTexture("Textures/bricks2.jpg");

	const auto normalMap = loadTexture("Textures/bricks2_normal.jpg");

	const auto heightMap = loadTexture("Textures/bricks2_disp.jpg");

	/* shader configuration */
	// ------------------------------
	shader.use();

	shader.setInt("diffuseMap", 0);

	shader.setInt("normalMap", 1);

	shader.setInt("depthMap", 2);

	/* lighting info */
	// ------------------------------
	const glm::vec3 lightPos(0.5f, 1.f, 0.3f);

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

		/* configure view/projection matrices */
		auto projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(scr_width) / scr_height, 0.1f,
		                                   100.f);

		auto view = camera.GetViewMatrix();

		shader.use();

		shader.setMat4("projection", projection);

		shader.setMat4("view", view);

		/* render parallax-mapped quad */
		auto model = glm::mat4(1.f);

		/* rotate the quad to show parallax mapping from multiple directions */
		model = rotate(model, glm::radians(static_cast<float>(glfwGetTime()) * -10.f),
		               normalize(glm::vec3(1.f, 0.f, 1.f)));

		shader.setMat4("model", model);

		shader.setVec3("viewPos", camera.Position);

		shader.setVec3("lightPos", lightPos);

		shader.setFloat("heightScale", heightScale);

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, diffuseMap);

		glActiveTexture(GL_TEXTURE1);

		glBindTexture(GL_TEXTURE_2D, normalMap);

		glActiveTexture(GL_TEXTURE2);

		glBindTexture(GL_TEXTURE_2D, heightMap);

		renderQuad();

		/* render light source (simply re-renders a smaller plane at the light's position for debugging/visualization) */
		model = glm::mat4(1.f);

		model = translate(model, lightPos);

		model = scale(model, glm::vec3(0.1f));

		shader.setMat4("model", model);

		renderQuad();

		/* glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.) */
		// ------------------------------
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	/* optional: de-allocate all resources once they've outlived their purpose: */
	// ------------------------------

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

	if (glfwGetKey(window,GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (heightScale > 0.f)
		{
			heightScale -= 0.0005f;
		}
		else
		{
			heightScale = 0.f;
		}
	}
	else if (glfwGetKey(window,GLFW_KEY_E) == GLFW_PRESS)
	{
		if (heightScale < 1.f)
		{
			heightScale += 0.0005f;
		}
		else
		{
			heightScale = 1.f;
		}
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
unsigned int loadTexture(char const* path, bool gammaCorrection)
{
	unsigned int textureID;

	glGenTextures(1, &textureID);

	int width, height, nrComponents;

	const auto data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum internalFormat;

		GLenum dataFormat;

		if (nrComponents == 1)
		{
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3)
		{
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;

			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;

			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

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

/* renders a 1x1 quad in NDC with manually calculated tangent vectors */
// ------------------------------
auto quadVAO = 0u;

auto quadVBO = 0u;

void renderQuad()
{
	if (quadVAO == 0)
	{
		/* positions */
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);

		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);

		glm::vec3 pos3(1.0f, -1.0f, 0.0f);

		glm::vec3 pos4(1.0f, 1.0f, 0.0f);

		/* texture coordinates */
		glm::vec2 uv1(0.0f, 1.0f);

		glm::vec2 uv2(0.0f, 0.0f);

		glm::vec2 uv3(1.0f, 0.0f);

		glm::vec2 uv4(1.0f, 1.0f);

		/* normal vector */
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		/* calculate tangent/bitangent vectors of both triangles */
		glm::vec3 tangent1, bitangent1;

		glm::vec3 tangent2, bitangent2;

		/* triangle 1 */
		// ------------------------------
		auto edge1 = pos2 - pos1;

		auto edge2 = pos3 - pos1;

		auto deltaUV1 = uv2 - uv1;

		auto deltaUV2 = uv3 - uv1;

		auto f = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);

		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);

		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		tangent1 = normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		bitangent1 = normalize(bitangent1);

		/* triangle 2 */
		// ------------------------------
		edge1 = pos3 - pos1;

		edge2 = pos4 - pos1;

		deltaUV1 = uv3 - uv1;

		deltaUV2 = uv4 - uv1;

		f = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);

		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);

		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		tangent2 = normalize(tangent2);

		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);

		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);

		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		bitangent2 = normalize(bitangent2);

		float quadVertices[] = {
			/* positions */ /* normal */ /* texcoords */ /* tangent */ /* bitangent */
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x,
			bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x,
			bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x,
			bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x,
			bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x,
			bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x,
			bitangent2.y, bitangent2.z
		};

		/* configure plane VAO */

		glGenVertexArrays(1, &quadVAO);

		glGenBuffers(1, &quadVBO);

		glBindVertexArray(quadVAO);

		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), static_cast<void*>(nullptr));

		glEnableVertexAttribArray(1);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

		glEnableVertexAttribArray(2);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));

		glEnableVertexAttribArray(3);

		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(8 * sizeof(float)));

		glEnableVertexAttribArray(4);

		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	}

	glBindVertexArray(quadVAO);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
}
