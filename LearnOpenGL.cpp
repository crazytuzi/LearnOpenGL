#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void frame_buffer_size_callback(GLFWwindow* window, int width, int height);

void process_input(GLFWwindow* window);

/* settings */
const auto scr_with = 800;

const auto scr_height = 800;

const auto vertexShaderSource = "#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aColor;\n"
	"out vec3 ourColor;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(aPos, 1.f);\n"
	"	ourColor = aColor;"
	"}\0";

const auto fragmentShaderSource = "#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec3 ourColor;\n"
	"void main()\n"
	"{\n"
	"	FragColor = vec4(ourColor, 1.f);\n"
	"}\n\0";

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
	/* vertex shader */
	const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);

	glCompileShader(vertexShader);

	/* check for shader compile errors */
	int success;

	char infoLog[512];

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);

		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	/* fragment shader */
	const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);

	glCompileShader(fragmentShader);

	/* check for shader compile errors */
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);

		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	/* link shaders */
	const auto shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);

	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);

	/* check for linking errors */
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);

		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);

	glDeleteShader(fragmentShader);

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

		glUseProgram(shaderProgram);

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
