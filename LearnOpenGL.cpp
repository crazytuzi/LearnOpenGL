#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stb_image.h>
#include "Shader.h"

void frame_buffer_size_callback(GLFWwindow* window, int width, int height);

void process_input(GLFWwindow* window);

/* settings */
const auto scr_with = 800;

const auto scr_height = 800;

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
	const Shader ourShader("Shaders/3.3.shader.vs", "Shaders/3.3.shader.fs");

	/* set up vertex data (and buffer(s)) and configure vertex attributes */
	// ------------------------------
	float vertices[] = {
		-0.5f, -0.5f, -0.5f, 0.f, 0.f,
		0.5f, -0.5f, -0.5f, 1.f, 0.f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.f,
		-0.5f, -0.5f, -0.5f, 0.f, 0.f,

		-0.5f, -0.5f, 0.5f, 0.f, 0.f,
		0.5f, -0.5f, 0.5f, 1.f, 0.f,
		0.5f, 0.5f, 0.5f, 1.f, 1.f,
		0.5f, 0.5f, 0.5f, 1.f, 1.f,
		-0.5f, 0.5f, 0.5f, 0.f, 1.f,
		-0.5f, -0.5f, 0.5f, 0.f, 0.f,

		-0.5f, 0.5f, 0.5f, 1.f, 0.f,
		-0.5f, 0.5f, -0.5f, 1.f, 1.f,
		-0.5f, -0.5f, -0.5f, 0.f, 1.f,
		-0.5f, -0.5f, -0.5f, 0.f, 1.f,
		-0.5f, -0.5f, 0.5f, 0.f, 0.f,
		-0.5f, 0.5f, 0.5f, 1.f, 0.f,

		0.5f, 0.5f, 0.5f, 1.f, 0.f,
		0.5f, 0.5f, -0.5f, 1.f, 1.f,
		0.5f, -0.5f, -0.5f, 0.f, 1.f,
		0.5f, -0.5f, -0.5f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f, 0.f, 0.f,
		0.5f, 0.5f, 0.5f, 1.f, 0.f,

		-0.5f, -0.5f, -0.5f, 0.f, 1.f,
		0.5f, -0.5f, -0.5f, 1.f, 1.f,
		0.5f, -0.5f, 0.5f, 1.f, 0.f,
		0.5f, -0.5f, 0.5f, 1.f, 0.f,
		-0.5f, -0.5f, 0.5f, 0.f, 0.f,
		-0.5f, -0.5f, -0.5f, 0.f, 1.f,

		-0.5f, 0.5f, -0.5f, 0.f, 1.f,
		0.5f, 0.5f, -0.5f, 1.f, 1.f,
		0.5f, 0.5f, 0.5f, 1.f, 0.f,
		0.5f, 0.5f, 0.5f, 1.f, 0.f,
		-0.5f, 0.5f, 0.5f, 0.f, 0.f,
		-0.5f, 0.5f, -0.5f, 0.f, 1.f
	};

	/* world space positions of our cubes */
	const glm::vec3 cubePositions[] = {
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(2.f, 5.f, -15.f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f, 3.f, -7.5f),
		glm::vec3(1.3f, -2.f, -2.5f),
		glm::vec3(1.5f, 2.f, -2.5f),
		glm::vec3(1.5f, 0.2f, -1.5f),
		glm::vec3(-1.3f, 1.f, -1.5f)
	};

	unsigned int VBO, VAO;

	glGenVertexArrays(1, &VAO);

	glGenBuffers(1, &VBO);

	/* bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s). */
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	/* position attribute */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(nullptr));

	glEnableVertexAttribArray(0);

	/* texture coord attribute */
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

	glEnableVertexAttribArray(1);

	/* note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind */
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/*
	 * You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	 * VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	 */
	glBindVertexArray(0);

	/* load and create a texture */
	// ------------------------------
	unsigned int texture0, texture1;

	/* texture 0 */
	// ------------------------------
	glGenTextures(1, &texture0);

	/* all upcoming GL_TEXTURE_2D operations now have effect on this texture object */
	glBindTexture(GL_TEXTURE_2D, texture0);

	/* set the texture wrapping parameters */
	// ------------------------------
	/* set texture wrapping to GL_REPEAT (default wrapping method) */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* set texture filtering parameters */
	// ------------------------------
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* load image, create texture and generate mipmaps */
	// ------------------------------
	int width, height, nrChannels;

	/* tell stb_image.h to flip loaded texture's on the y-axis. */
	stbi_set_flip_vertically_on_load(true);

	const auto data0 = stbi_load("Textures/container.jpg", &width, &height, &nrChannels, 0);

	if (data0 != nullptr)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data0);

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data0);

	/* texture 1 */
	// ------------------------------
	glGenTextures(1, &texture1);

	glBindTexture(GL_TEXTURE_2D, texture1);

	/* set the texture wrapping parameters */
	// ------------------------------
	/* set texture wrapping to GL_REPEAT (default wrapping method) */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* set texture filtering parameters */
	// ------------------------------
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* load image, create texture and generate mipmaps */
	const auto data1 = stbi_load("Textures/awesomeface.png", &width, &height, &nrChannels, 0);

	if (data1 != nullptr)
	{
		/* note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data1);

	/* tell opengl for each sampler to which texture unit it belongs to (only has to be done once) */
	// ------------------------------
	/* don't forget to activate/use the shader before setting uniforms! */
	ourShader.use();

	/* either set it manually like so: */
	glUniform1i(glGetUniformLocation(ourShader.ID, "texture0"), 0);

	/* or set it via the texture class */
	ourShader.setInt("texture1", 1);

	/* render loop */
	// ------------------------------
	while (!glfwWindowShouldClose(window))
	{
		/* input */
		// ------------------------------
		process_input(window);

		/* render */
		// ------------------------------
		glClearColor(0.2f, 0.3f, 0.3f, 1.f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* bind textures on corresponding texture units */
		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, texture0);

		glActiveTexture(GL_TEXTURE1);

		glBindTexture(GL_TEXTURE_2D, texture1);


		/* activate shader */
		ourShader.use();

		/* create transformations */
		// ------------------------------
		/* make sure to initialize matrix to identity matrix first */
		glm::mat4 view = glm::mat4(1.f);

		glm::mat4 projection = glm::mat4(1.f);

		view = translate(view, glm::vec3(0.f, 0.f, -3.f));

		projection = glm::perspective(glm::radians(45.f), static_cast<float>(scr_with) / static_cast<float>(scr_height),
		                              0.1f, 100.f);

		/* retrieve the matrix uniform locations */

		ourShader.setMat4("view", view);

		ourShader.setMat4("projection", projection);

		/* render boxes */
		glBindVertexArray(VAO);

		for (auto i = 0; i < 10; ++i)
		{
			/* calculate the model matrix for each object and pass it to shader before drawing */
			auto model = glm::mat4(1.f);

			model = translate(model, cubePositions[i]);

			const auto angle = 20.f * i;

			model = rotate(model, glm::radians(angle), glm::vec3(1.f, 0.3f, 0.5f));

			ourShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		/* glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.) */
		// ------------------------------
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	/* optional: de-allocate all resources once they've outlived their purpose: */
	// ------------------------------
	glDeleteVertexArrays(1, &VAO);

	glDeleteBuffers(1, &VBO);

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
