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

void renderCube();

/* settings */
const auto scr_width = 1280;

const auto scr_height = 720;

/* camera */
Camera camera(glm::vec3(0.f, 0.f, 5.f));

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
	const Shader shaderGeometryPass("Shaders/8.2.g_buffer.vs", "Shaders/8.2.g_buffer.fs");

	const Shader shaderLightingPass("Shaders/8.2.deferred_shading.vs", "Shaders/8.2.deferred_shading.fs");

	const Shader shaderLightBox("Shaders/8.2.deferred_light_box.vs", "Shaders/8.2.deferred_light_box.fs");

	/* load models */
	// ------------------------------
	const Model nanosuit("Objects/nanosuit/nanosuit.obj");

	std::vector<glm::vec3> objectPositions;

	objectPositions.emplace_back(-3.0, -3.0, -3.0);

	objectPositions.emplace_back(0.0, -3.0, -3.0);

	objectPositions.emplace_back(3.0, -3.0, -3.0);

	objectPositions.emplace_back(-3.0, -3.0, 0.0);

	objectPositions.emplace_back(0.0, -3.0, 0.0);

	objectPositions.emplace_back(3.0, -3.0, 0.0);

	objectPositions.emplace_back(-3.0, -3.0, 3.0);

	objectPositions.emplace_back(0.0, -3.0, 3.0);

	objectPositions.emplace_back(3.0, -3.0, 3.0);

	/* configure g-buffer framebuffer */
	// ------------------------------
	unsigned int gBuffer;

	glGenFramebuffers(1, &gBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	unsigned int gPosition, gNormal, gAlbedoSpec;

	/* position color buffer */
	glGenTextures(1, &gPosition);

	glBindTexture(GL_TEXTURE_2D, gPosition);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scr_width, scr_height, 0, GL_RGB, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	/* normal color buffer */
	glGenTextures(1, &gNormal);

	glBindTexture(GL_TEXTURE_2D, gNormal);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scr_width, scr_height, 0, GL_RGB, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	/* color + specular color buffer */
	glGenTextures(1, &gAlbedoSpec);

	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scr_width, scr_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	/* tell OpenGL which color attachments we'll use (of this framebuffer) for rendering */
	unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

	glDrawBuffers(3, attachments);

	/* create and attach depth buffer (renderbuffer) */
	unsigned int rboDepth;

	glGenRenderbuffers(1, &rboDepth);

	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scr_width, scr_height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	/* finally check if framebuffer is complete */
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* lighting info */
	// ------------------------------
	const auto NR_LIGHTS = 32u;

	std::vector<glm::vec3> lightPositions;

	std::vector<glm::vec3> lightColors;

	srand(13);

	for (auto i = 0u; i < NR_LIGHTS; i++)
	{
		/* calculate slightly random offsets */
		const auto xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;

		const auto yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;

		const auto zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;

		lightPositions.emplace_back(xPos, yPos, zPos);

		/* also calculate random color */
		/* between 0.5 and 1.0 */
		const auto rColor = (rand() % 100) / 200.0f + 0.5;

		const auto gColor = (rand() % 100) / 200.0f + 0.5;

		const auto bColor = (rand() % 100) / 200.0f + 0.5;

		lightColors.emplace_back(rColor, gColor, bColor);
	}

	/* shader configuration */
	// ------------------------------
	shaderLightingPass.use();

	shaderLightingPass.setInt("gPosition", 0);

	shaderLightingPass.setInt("gNormal", 1);

	shaderLightingPass.setInt("gAlbedoSpec", 2);

	shaderLightingPass.use();

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
		glClearColor(0.0f, 0.0f, 0.0f, 1.f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* 1. geometry pass: render scene's geometry/color data into gbuffer */
		// ------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(scr_width) / scr_height, 0.1f,
		                                   100.0f);

		auto view = camera.GetViewMatrix();

		auto model = glm::mat4(1.0f);

		shaderGeometryPass.use();

		shaderGeometryPass.setMat4("projection", projection);

		shaderGeometryPass.setMat4("view", view);

		for (auto objectPosition : objectPositions)
		{
			model = glm::mat4(1.0f);

			model = translate(model, objectPosition);

			model = scale(model, glm::vec3(0.25f));

			shaderGeometryPass.setMat4("model", model);

			nanosuit.Draw(shaderGeometryPass);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/* 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content. */
		// ------------------------------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderLightingPass.use();

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, gPosition);

		glActiveTexture(GL_TEXTURE1);

		glBindTexture(GL_TEXTURE_2D, gNormal);

		glActiveTexture(GL_TEXTURE2);

		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

		/* send light relevant uniforms */
		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{
			shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);

			shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);

			/* update attenuation parameters and calculate radius */
			/* note that we don't send this to the shader, we assume it is always 1.0 (in our case) */
			const auto constant = 1.0f;

			const auto linear = 0.7f;

			const auto quadratic = 1.8f;

			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Linear", linear);

			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);

			/* then calculate radius of light volume/sphere */
			const auto maxBrightness = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);

			const auto radius = (-linear + std::sqrt(
				linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);

			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Radius", radius);
		}

		shaderLightingPass.setVec3("viewPos", camera.Position);

		/* finally render quad */
		renderQuad();

		/* 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer */
		// ------------------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);

		/* write to default framebuffer */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		/*
		 * blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
		 * the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the
		 * depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
		 */
		glBlitFramebuffer(0, 0, scr_width, scr_height, 0, 0, scr_width, scr_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/* 3. render lights on top of scene */
		// ------------------------------
		shaderLightBox.use();

		shaderLightBox.setMat4("projection", projection);

		shaderLightBox.setMat4("view", view);

		for (auto i = 0u; i < lightPositions.size(); i++)
		{
			model = glm::mat4(1.0f);

			model = translate(model, lightPositions[i]);

			model = scale(model, glm::vec3(0.125f));

			shaderLightBox.setMat4("model", model);

			shaderLightBox.setVec3("lightColor", lightColors[i]);

			renderCube();
		}

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

		/* fill buffer  */
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

/* renders a 1x1 quad in NDC with manually calculated tangent vectors */
// ------------------------------
auto quadVAO = 0u;

auto quadVBO = 0u;

void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			/* positions */ /* texture Coords */
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		/* setup plane VAO */
		glGenVertexArrays(1, &quadVAO);

		glGenBuffers(1, &quadVBO);

		glBindVertexArray(quadVAO);

		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(nullptr));

		glEnableVertexAttribArray(1);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	}

	glBindVertexArray(quadVAO);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
}
