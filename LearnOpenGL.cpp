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

unsigned int loadTexture(const char* path);

unsigned int loadCubemap(const std::vector<std::string>& faces);

/* settings */
const auto scr_with = 1280;

const auto scr_height = 720;

/* camera */
Camera camera(glm::vec3(0.f, 0.f, 155.f));

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
	const Shader asteroidShader("Shaders/10.3.asteroids.vs", "Shaders/10.3.asteroids.fs");

	const Shader planetShader("Shaders/10.3.planet.vs", "Shaders/10.3.planet.fs");

	/* load models */
	// ------------------------------
	const Model rock("Objects/rock/rock.obj");

	const Model planet("Objects/planet/planet.obj");

	/* generate a large list of semi-random model transformation matrices */
	// ------------------------------
	const auto amount = 10000u;

	const auto modelMatrices = new glm::mat4[amount];

	/* initialize random seed */
	srand(glfwGetTime());

	const auto radius = 150.f;

	const auto offset = 25.f;

	for (auto i = 0u; i < amount; ++i)
	{
		auto model = glm::mat4(1.f);

		/* 1. translation: displace along circle with 'radius' in range [-offset, offset] */
		const auto angle = static_cast<float>(i) / amount * 360.f;

		auto displacement = rand() % static_cast<int>(2 * offset * 100) / 100.f - offset;

		const auto x = sin(angle) * radius + displacement;

		displacement = rand() % static_cast<int>(2 * offset * 100) / 100.f - offset;

		/* keep height of asteroid field smaller compared to width of x and z */
		const auto y = displacement * 0.4f;

		displacement = rand() % static_cast<int>(2 * offset * 100) / 100.f - offset;

		const auto z = cos(angle) * radius + displacement;

		model = translate(model, glm::vec3(x, y, z));

		/* 2. scale: Scale between 0.05 and 0.25f */
		const auto scale = rand() % 20 / 100.f + 0.05f;

		model = glm::scale(model, glm::vec3(scale));

		/* 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector */
		const auto rotAngle = static_cast<float>(rand() % 360);

		model = rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		/* 4. now add to list of matrices */
		modelMatrices[i] = model;
	}

	/* configure instanced array */
	// ------------------------------
	unsigned int buffer;

	glGenBuffers(1, &buffer);

	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	/*
	 * set transformation matrices as an instance vertex attribute (with divisor 1)
	 * note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
	 * normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
	 */
	for (const auto& mesh : rock.meshes)
	{
		const auto VAO = mesh.VAO;

		glBindVertexArray(VAO);

		/* set attribute pointers for matrix (4 times vec4) */
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(nullptr));

		glEnableVertexAttribArray(4);

		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(sizeof(glm::vec4)));

		glEnableVertexAttribArray(5);

		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
		                      reinterpret_cast<void*>(2 * sizeof(glm::vec4)));

		glEnableVertexAttribArray(6);

		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
		                      reinterpret_cast<void*>(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);

		glVertexAttribDivisor(4, 1);

		glVertexAttribDivisor(5, 1);

		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}

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

		/* configure transformation matrices */
		auto projection = glm::perspective(glm::radians(45.f), static_cast<float>(scr_with) / scr_height, 0.1f, 1000.f);

		auto view = camera.GetViewMatrix();

		asteroidShader.use();

		asteroidShader.setMat4("projection", projection);

		asteroidShader.setMat4("view", view);

		planetShader.use();

		planetShader.setMat4("projection", projection);

		planetShader.setMat4("view", view);

		/* draw planet */
		auto model = glm::mat4(1.f);

		model = translate(model, glm::vec3(0.f, -3.f, 0.f));

		model = scale(model, glm::vec3(4.f, 4.f, 4.f));

		planetShader.setMat4("model", model);

		planet.Draw(planetShader);

		/* draw meteorites */
		asteroidShader.use();

		asteroidShader.setInt("texture_diffuse0", 0);

		glActiveTexture(GL_TEXTURE0);

		/* note: we also made the textures_loaded vector public (instead of private) from the model class. */
		glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);

		for (const auto& mesh : rock.meshes)
		{
			glBindVertexArray(mesh.VAO);

			glDrawElementsInstanced(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr, amount);

			glBindVertexArray(0);
		}

		/* glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.) */
		// ------------------------------
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	/* optional: de-allocate all resources once they've outlived their purpose: */
	// ------------------------------
	delete[] modelMatrices;

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
