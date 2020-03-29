#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
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

	/* build and compile our shader program */
	// ------------------------------
	const Shader ourShader("shaders/3.3.shader.vs", "shaders/3.3.shader.fs");

	/* set up vertex data (and buffer(s)) and configure vertex attributes */
	// ------------------------------
	float vertices[] = {
		/* positions */ /* colors */
		0.5f, -0.5f, 0.f, 1.f, 0.f, 0.f, /* bottom right */
		-0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f, /* bottom left */
		0.f, 0.5f, 0.f, 0.f, 0.f, 1.f /* top  */
	};

	unsigned int VBO, VAO;

	glGenVertexArrays(1, &VAO);

	glGenBuffers(1, &VBO);

	/* bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s). */
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	/* position attribute */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(nullptr));

	glEnableVertexAttribArray(0);

	/* color attribute */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

	glEnableVertexAttribArray(1);

	/* note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind */
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/*
	 * You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	 * VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	 */
	glBindVertexArray(0);

	/* render loop */
	// ------------------------------
	while (!glfwWindowShouldClose(window))
	{
		/* input */
		// ------------------------------
		process_input(window);

		/* render */
		// ------------------------------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT);

		ourShader.use();

		/* seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized */
		glBindVertexArray(VAO);

		glDrawArrays(GL_TRIANGLES, 0, 3);

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
