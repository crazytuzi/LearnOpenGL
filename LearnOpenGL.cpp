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

void renderScene(const Shader& shader);

void renderCube();

/* settings */
const auto scr_width = 1280;

const auto scr_height = 720;

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

	glEnable(GL_CULL_FACE);

	/* build and compile our shader program */
	// ------------------------------
	const Shader shader("Shaders/3.2.1.point_shadows.vs", "Shaders/3.2.1.point_shadows.fs");

	const Shader simpleDepthShader("Shaders/3.2.1.point_shadows_depth.vs", "Shaders/3.2.1.point_shadows_depth.fs",
	                               "Shaders/3.2.1.point_shadows_depth.gs");

	/* load textures */
	// ------------------------------
	const auto woodTexture = loadTexture("Textures/wood.png");

	/* configure depth map FBO */
	// ------------------------------
	const auto shadow_width = 1024u, shadow_height = 1024u;

	unsigned int depthMapFBO;

	glGenFramebuffers(1, &depthMapFBO);

	/* create depth texture */
	// ------------------------------
	unsigned int depthCubeMap;

	glGenTextures(1, &depthCubeMap);

	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

	for (auto i = 0u; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, 0,
		             GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	/* attach depth texture as FBO's depth buffer */
	// ------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);

	glDrawBuffer(GL_NONE);

	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* shader configuration */
	// ------------------------------
	shader.use();

	shader.setInt("diffuseTexture", 0);

	shader.setInt("depthMap", 1);

	/* lighting info */
	// ------------------------------
	glm::vec3 lightPos(0.f, 0.f, 0.f);

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

		/* move light position over time */
		// ------------------------------
		lightPos.z = sin(glfwGetTime() * 0.5f) * 3.f;

		/* render */
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* 0. create depth cubemap transformation matrices */
		// ------------------------------
		const auto near_plane = 1.f;

		const auto far_plane = 25.f;

		const auto shadowProj = glm::perspective(glm::radians(90.f), static_cast<float>(shadow_width) / shadow_height,
		                                         near_plane, far_plane);

		std::vector<glm::mat4> shadowTransforms;

		shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + glm::vec3(1.f, 0.f, 0.f),
		                                               glm::vec3(0.f, -1.f, 0.f)));

		shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + glm::vec3(-1.f, 0.f, 0.f),
		                                               glm::vec3(0.f, -1.f, 0.f)));

		shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.f, 1.f, 0.f),
		                                               glm::vec3(0.f, 0.f, 1.f)));

		shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.f, -1.f, 0.f),
		                                               glm::vec3(0.f, 0.f, -1.f)));

		shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.f, 0.f, 1.f),
		                                               glm::vec3(0.f, -1.f, 0.f)));

		shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.f, 0.f, -1.f),
		                                               glm::vec3(0.f, -1.f, 0.f)));

		/* 1. render depth of scene to texture (from light's perspective) */
		// ------------------------------
		glViewport(0, 0, shadow_width, shadow_height);

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

		glClear(GL_DEPTH_BUFFER_BIT);

		simpleDepthShader.use();

		for (auto i = 0u; i < 6; ++i)
		{
			simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		}

		simpleDepthShader.setFloat("far_plane", far_plane);

		simpleDepthShader.setVec3("lightPos", lightPos);

		renderScene(simpleDepthShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/* 2. render scene as normal */
		// ------------------------------
		glViewport(0, 0, scr_width, scr_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		const auto projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(scr_width) / scr_height,
		                                         0.1f, 100.f);

		const auto view = camera.GetViewMatrix();

		shader.setMat4("projection", projection);

		shader.setMat4("view", view);

		/* set lighting uniforms */
		shader.setVec3("lightPos", lightPos);

		shader.setVec3("viewPos", camera.Position);

		shader.setFloat("far_plane", far_plane);

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, woodTexture);

		glActiveTexture(GL_TEXTURE1);

		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

		renderScene(shader);

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

/* renders the 3D scene */
// ------------------------------
void renderScene(const Shader& shader)
{
	/* room cube */
	auto model = glm::mat4(1.f);

	model = scale(model, glm::vec3(5.f));

	shader.setMat4("model", model);

	/* note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods. */
	glDisable(GL_CULL_FACE);

	/* A small little hack to invert normals when drawing cube from the inside so lighting still works. */
	shader.setInt("reverse_normals", 1);

	renderCube();

	/* and of course disable it */
	shader.setInt("reverse_normals", 0);

	glEnable(GL_CULL_FACE);

	/* cubes */
	model = glm::mat4(1.f);

	model = translate(model, glm::vec3(4.f, -3.5f, 0.f));

	model = scale(model, glm::vec3(0.5f));

	shader.setMat4("model", model);

	renderCube();

	model = glm::mat4(1.f);

	model = translate(model, glm::vec3(2.f, 3.f, 1.f));

	model = scale(model, glm::vec3(0.75f));

	shader.setMat4("model", model);

	renderCube();

	model = glm::mat4(1.f);

	model = translate(model, glm::vec3(-3.f, -1.f, 0.f));

	model = scale(model, glm::vec3(0.5f));

	shader.setMat4("model", model);

	renderCube();

	model = glm::mat4(1.f);

	model = translate(model, glm::vec3(-1.5f, 1.f, 1.5f));

	model = scale(model, glm::vec3(0.5f));

	shader.setMat4("model", model);

	renderCube();

	model = glm::mat4(1.f);

	model = translate(model, glm::vec3(-1.5f, 2.f, -3.f));

	model = rotate(model, glm::radians(60.f), normalize(glm::vec3(1.f, 0.f, 1.f)));

	model = scale(model, glm::vec3(0.75f));

	shader.setMat4("model", model);

	renderCube();
}

/* renderCube() renders a 1x1 3D cube in NDC. */
// ------------------------------
auto cubeVAO = 0u;

auto cubeVBO = 0u;

void renderCube()
{
	/* initialize (if necessary) */
	if (cubeVAO == 0)
	{
		float vertices[] = {
			/* back face */
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, /* bottom-left */
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, /* top-right */
			1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, /* bottom-right */
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, /* top-right */
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, /* bottom-left */
			-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, /* top-left */
			/* front face */
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, /* bottom-left */
			1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, /* bottom-right */
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, /* top-right */
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, /* top-right */
			-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, /* top-left */
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, /* bottom-left */
			/* left face */
			-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, /* top-right */
			-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, /* top-left */
			-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, /* bottom-left */
			-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, /* bottom-left */
			-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, /* bottom-right */
			-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, /* top-right */
			/* right face */
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, /* top-left */
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, /* bottom-right */
			1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, /* top-right */
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, /* bottom-right */
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, /* top-left */
			1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, /* bottom-left */
			/* bottom face */
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, /* top-right */
			1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, /* top-left */
			1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, /* bottom-left */
			1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, /* bottom-left */
			-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, /* bottom-right */
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, /* top-right */
			/* top face */
			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, /* top-left */
			1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, /* bottom-right */
			1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, /* top-right */
			1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, /* bottom-right */
			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, /* top-left */
			-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f /* bottom-left */
		};

		glGenVertexArrays(1, &cubeVAO);

		glGenBuffers(1, &cubeVBO);

		/* fill buffer */
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		/* link vertex attributes */
		glBindVertexArray(cubeVAO);

		glEnableVertexAttribArray(0);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void*>(nullptr));

		glEnableVertexAttribArray(1);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

		glEnableVertexAttribArray(2);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);
	}

	/* render Cube */
	glBindVertexArray(cubeVAO);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
}
