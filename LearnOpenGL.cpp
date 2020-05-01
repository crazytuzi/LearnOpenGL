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

void renderSphere();

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

    glfwWindowHint(GLFW_SAMPLES, 4);

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

    /* set depth function to less than AND equal for skybox depth trick. */
    glDepthFunc(GL_LEQUAL);

    /* build and compile our shader program */
    // ------------------------------
    const Shader pbrShader("Shaders/2.1.2.pbr.vs", "Shaders/2.1.2.pbr.fs");

    const Shader equirectangularToCubemapShader("Shaders/2.1.2.cubemap.vs",
                                                "Shaders/2.1.2.equirectangular_to_cubemap.fs");

    const Shader irradianceShader("Shaders/2.1.2.cubemap.vs", "Shaders/2.1.2.irradiance_convolution.fs");

    const Shader backgroundShader("Shaders/2.1.2.background.vs", "Shaders/2.1.2.background.fs");

    pbrShader.use();

    pbrShader.setInt("irradianceMap", 0);

    pbrShader.setVec3("albedo", 0.5f, 0.f, 0.f);

    pbrShader.setFloat("ao", 1.f);

    backgroundShader.use();

    backgroundShader.setInt("environmentMap", 0);


    /* lights */
    // ------------------------------
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.f, 10.f, 10.f),
        glm::vec3(10.f, 10.f, 10.f),
        glm::vec3(-10.f, -10.f, 10.f),
        glm::vec3(10.f, -10.f, 10.f),
    };

    glm::vec3 lightColors[] = {
        glm::vec3(300.f, 300.f, 300.f),
        glm::vec3(300.f, 300.f, 300.f),
        glm::vec3(300.f, 300.f, 300.f),
        glm::vec3(300.f, 300.f, 300.f)
    };

    const auto nrRows = 7;

    const auto nrColumns = 7;

    const auto spacing = 2.5f;

    /* pbr: setup framebuffer */
    // ------------------------------
    unsigned int captureFBO;

    unsigned int captureRBO;

    glGenFramebuffers(1, &captureFBO);

    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    /* pbr: load the HDR environment map */
    // ------------------------------
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrComponents;

    auto data = stbi_loadf("Textures/hdr/newport_loft.hdr", &width, &height, &nrComponents, 0);

    unsigned int hdrTexture;

    if (data)
    {
        glGenTextures(1, &hdrTexture);

        glBindTexture(GL_TEXTURE_2D, hdrTexture);

        /* note how we specify the texture's data value to be float */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }

    /* pbr: setup cubemap to render to and attach to framebuffer */
    // ------------------------------
    unsigned int envCubemap;

    glGenTextures(1, &envCubemap);

    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    for (auto i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions */
    // ------------------------------
    auto captureProjection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);

    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)),
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f))
    };

    /* pbr: convert HDR equirectangular environment map to cubemap equivalent */
    // ------------------------------
    equirectangularToCubemapShader.use();

    equirectangularToCubemapShader.setInt("equirectangularMap", 0);

    equirectangularToCubemapShader.setMat4("projection", captureProjection);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    /* don't forget to configure the viewport to the capture dimensions. */
    glViewport(0, 0, 512, 512);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    for (auto i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader.setMat4("view", captureViews[i]);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale. */
    // ------------------------------
    unsigned int irradianceMap;

    glGenTextures(1, &irradianceMap);

    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

    for (auto i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    /* pbr: solve diffuse integral by convolution to create an irradiance (cube)map. */
    // ------------------------------
    irradianceShader.use();

    irradianceShader.setInt("environmentMap", 0);

    irradianceShader.setMat4("projection", captureProjection);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    /* don't forget to configure the viewport to the capture dimensions. */
    glViewport(0, 0, 32, 32);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    for (auto i = 0; i < 6; ++i)
    {
        irradianceShader.setMat4("view", captureViews[i]);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap,
                               0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* initialize static shader uniforms before rendering */
    // ------------------------------
    auto projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(scr_width) / scr_height, 0.1f,
                                       100.f);

    pbrShader.use();

    pbrShader.setMat4("projection", projection);

    backgroundShader.use();

    backgroundShader.setMat4("projection", projection);

    /* then before rendering, configure the viewport to the original framebuffer's screen dimensions */
    int scrWidth, scrHeight;

    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);

    glViewport(0, 0, scrWidth, scrHeight);

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
        // ------------------------------
        glClearColor(0.2f, 0.3f, 0.3f, 1.f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* render scene, supplying the convoluted irradiance map to the final shader. */
        // ------------------------------
        pbrShader.use();

        auto view = camera.GetViewMatrix();

        pbrShader.setMat4("view", view);

        pbrShader.setVec3("cameraPos", camera.Position);

        /* bind pre-computed IBL data */
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

        /* render rows*column number of spheres with material properties defined by textures (they all have the same material properties) */
        auto model = glm::mat4(1.f);

        for (auto row = 0; row < nrRows; ++row)
        {
            pbrShader.setFloat("metallic", static_cast<float>(row) / nrRows);

            for (auto col = 0; col < nrColumns; ++col)
            {
                /* we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off */
                /* on direct lighting. */
                pbrShader.setFloat("roughness", glm::clamp(static_cast<float>(col) / nrColumns, 0.05f, 1.f));

                model = glm::mat4(1.f);

                model = glm::translate(model, glm::vec3(
                                           (col - (nrColumns / 2)) * spacing,
                                           (row - (nrRows / 2)) * spacing,
                                           -2.f));

                pbrShader.setMat4("model", model);

                renderSphere();
            }
        }

        /*
         * render light source (simply re-render sphere at light positions)
         * this looks a bit off as we use the same shader, but it'll make their positions obvious and
         * keeps the codeprint small.
         */
        for (auto i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);

            pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.f);

            model = glm::translate(model, lightPositions[i]);

            model = glm::scale(model, glm::vec3(0.5f));

            pbrShader.setMat4("model", model);

            renderSphere();
        }

        /* render skybox (render as last to prevent overdraw) */
        backgroundShader.use();

        backgroundShader.setMat4("view", view);

        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

        renderCube();

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

/* renders (and builds at first invocation) a sphere */
// ------------------------------
auto sphereVAO = 0u;

auto indexCount = 0u;

void renderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        auto vbo = 0u, ebo = 0u;

        glGenBuffers(1, &vbo);

        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;

        std::vector<glm::vec2> uv;

        std::vector<glm::vec3> normals;

        std::vector<unsigned int> indices;

        const auto x_segments = 64u;

        const auto y_segments = 64u;

        const auto PI = 3.14159265359f;

        for (auto y = 0; y <= y_segments; ++y)
        {
            for (auto x = 0; x <= x_segments; ++x)
            {
                const auto xSegment = static_cast<float>(x) / x_segments;

                const auto ySegment = static_cast<float>(y) / y_segments;

                const auto xPos = std::cos(xSegment * 2.f * PI) * std::sin(ySegment * PI);

                const auto yPos = std::cos(ySegment * PI);

                const auto zPos = std::sin(xSegment * 2.f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));

                uv.push_back(glm::vec2(xSegment, ySegment));

                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        auto oddRow = false;

        for (auto y = 0; y < y_segments; ++y)
        {
            /* even rows: y == 0, y == 2; and so on */
            if (!oddRow)
            {
                for (auto x = 0; x <= x_segments; ++x)
                {
                    indices.push_back(y * (x_segments + 1) + x);

                    indices.push_back((y + 1) * (x_segments + 1) + x);
                }
            }
            else
            {
                for (int x = x_segments; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (x_segments + 1) + x);
                    indices.push_back(y * (x_segments + 1) + x);
                }
            }

            oddRow = !oddRow;
        }

        indexCount = indices.size();

        std::vector<float> data;

        for (auto i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);

            data.push_back(positions[i].y);

            data.push_back(positions[i].z);

            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);

                data.push_back(uv[i].y);
            }

            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);

                data.push_back(normals[i].y);

                data.push_back(normals[i].z);
            }
        }

        glBindVertexArray(sphereVAO);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0],GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0],GL_STATIC_DRAW);

        const auto stride = (3 + 2 + 3) * sizeof(float);

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, static_cast<void*>(nullptr));

        glEnableVertexAttribArray(1);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));

        glEnableVertexAttribArray(2);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(5 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);

    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
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
            -1.f, -1.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, /* bottom-left */
            1.f, 1.f, -1.f, 0.f, 0.f, -1.f, 1.f, 1.f, /* top-right */
            1.f, -1.f, -1.f, 0.f, 0.f, -1.f, 1.f, 0.f, /* bottom-right */
            1.f, 1.f, -1.f, 0.f, 0.f, -1.f, 1.f, 1.f, /* top-right */
            -1.f, -1.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, /* bottom-left */
            -1.f, 1.f, -1.f, 0.f, 0.f, -1.f, 0.f, 1.f, /* top-left */
            /* front face */
            -1.f, -1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, /* bottom-left */
            1.f, -1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, /* bottom-right */
            1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f, /* top-right */
            1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f, /* top-right */
            -1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 1.f, /* top-left */
            -1.f, -1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, /* bottom-left */
            /* left face */
            -1.f, 1.f, 1.f, -1.f, 0.f, 0.f, 1.f, 0.f, /* top-right */
            -1.f, 1.f, -1.f, -1.f, 0.f, 0.f, 1.f, 1.f, /* top-left */
            -1.f, -1.f, -1.f, -1.f, 0.f, 0.f, 0.f, 1.f, /* bottom-left */
            -1.f, -1.f, -1.f, -1.f, 0.f, 0.f, 0.f, 1.f, /* bottom-left */
            -1.f, -1.f, 1.f, -1.f, 0.f, 0.f, 0.f, 0.f, /* bottom-right */
            -1.f, 1.f, 1.f, -1.f, 0.f, 0.f, 1.f, 0.f, /* top-right */
            /* right face */
            1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, /* top-left */
            1.f, -1.f, -1.f, 1.f, 0.f, 0.f, 0.f, 1.f, /* bottom-right */
            1.f, 1.f, -1.f, 1.f, 0.f, 0.f, 1.f, 1.f, /* top-right */
            1.f, -1.f, -1.f, 1.f, 0.f, 0.f, 0.f, 1.f, /* bottom-right */
            1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, /* top-left */
            1.f, -1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, /* bottom-left */
            /* bottom face */
            -1.f, -1.f, -1.f, 0.f, -1.f, 0.f, 0.f, 1.f, /* top-right */
            1.f, -1.f, -1.f, 0.f, -1.f, 0.f, 1.f, 1.f, /* top-left */
            1.f, -1.f, 1.f, 0.f, -1.f, 0.f, 1.f, 0.f, /* bottom-left */
            1.f, -1.f, 1.f, 0.f, -1.f, 0.f, 1.f, 0.f, /* bottom-left */
            -1.f, -1.f, 1.f, 0.f, -1.f, 0.f, 0.f, 0.f, /* bottom-right */
            -1.f, -1.f, -1.f, 0.f, -1.f, 0.f, 0.f, 1.f, /* top-right */
            /* top face */
            -1.f, 1.f, -1.f, 0.f, 1.f, 0.f, 0.f, 1.f, /* top-left */
            1.f, 1.f, 1.f, 0.f, 1.f, 0.f, 1.f, 0.f, /* bottom-right */
            1.f, 1.f, -1.f, 0.f, 1.f, 0.f, 1.f, 1.f, /* top-right */
            1.f, 1.f, 1.f, 0.f, 1.f, 0.f, 1.f, 0.f, /* bottom-right */
            -1.f, 1.f, -1.f, 0.f, 1.f, 0.f, 0.f, 1.f, /* top-left */
            -1.f, 1.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f /* bottom-left */
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
