#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <map>
#include <stb_image.h>
#include <vector>
#include <string>

#include "Shader.h"

/* settings */
const auto scr_width = 800;

const auto scr_height = 600;

/* Holds all state information relevant to a character as loaded using FreeType */
struct Character
{
    /* ID handle of the glyph texture */
    GLuint TextureID;

    /* Size of glyph */
    glm::ivec2 Size;

    /* Offset from baseline to left/top of glyph */
    glm::ivec2 Bearing;

    /* Horizontal offset to advance to next glyph */
    GLuint Advance;
};

std::map<GLchar, Character> Characters;

GLuint VAO, VBO;

void RenderText(const Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

int main()
{
    /* glfw: initialize and configure */
    // ------------------------------
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);

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

    /* glad: load all OpenGL function pointers */
    // ------------------------------
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    glViewport(0, 0, scr_width, scr_height);

    /* configure global opengl state */
    // ------------------------------
    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    /* build and compile our shader program */
    // ------------------------------
    const Shader shader("Shaders/text.vs", "Shaders/text.fs");

    const auto projection = glm::ortho(0.f, static_cast<GLfloat>(scr_width), 0.f, static_cast<GLfloat>(scr_height));

    shader.use();

    shader.setMat4("projection", projection);

    /* FreeType */
    FT_Library ft;

    /* All functions return a value different than 0 whenever an error occurred */
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    /* Load font as face */
    FT_Face face;
    if (FT_New_Face(ft, "Fonts/arial.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    /* Set size to load glyphs as */
    FT_Set_Pixel_Sizes(face, 0, 48);

    /* Disable byte-alignment restriction */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Load first 128 characters of ASCII set */
    for (auto c = 0; c < 128; ++c)
    {
        /* Load character glyph */
        if (FT_Load_Char(face, c,FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }

        /* Generate texture */
        GLuint texture;

        glGenTextures(1, &texture);

        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        /* Set texture options */
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

        /* Now store character for later use */
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };

        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    /* Destroy FreeType once we're finished */
    FT_Done_Face(face);

    FT_Done_FreeType(ft);

    /* Configure VAO/VBO for texture quads */
    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr,GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 4,GL_FLOAT,GL_FALSE, 4 * sizeof(GLfloat), nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    /* render loop */
    // ------------------------------
    while (!glfwWindowShouldClose(window))
    {
        /* Check and call events */
        glfwPollEvents();

        /* render */
        // ------------------------------
        glClearColor(0.2f, 0.3f, 0.3f, 1.f);

        glClear(GL_COLOR_BUFFER_BIT);

        RenderText(shader, "This is a sample text", 25.f, 25.f, 1.f, glm::vec3(0.5f, 0.8f, 0.2f));

        RenderText(shader, "(C) LearnOpenGL.com", 540.f, 570.f, 0.5f, glm::vec3(0.3f, 0.7f, 0.9f));

        /* glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.) */
        // ------------------------------
        glfwSwapBuffers(window);
    }

    /* optional: de-allocate all resources once they've outlived their purpose: */
    // ------------------------------

    /* glfw: terminate, clearing all previously allocated GLFW resources. */
    // ------------------------------
    glfwTerminate();

    return 0;
}

void RenderText(const Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    /* Activate corresponding render state */
    shader.use();

    shader.setVec3("textColor", color.x, color.y, color.z);

    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);

    /* Iterate through all characters */
    for (const auto c : text)
    {
        const auto ch = Characters[c];

        const auto xpos = x + ch.Bearing.x * scale;

        const auto ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        const auto w = ch.Size.x * scale;

        const auto h = ch.Size.y * scale;

        /* Update VBO for each character */
        GLfloat vertices[6][4] = {
            {xpos, ypos + h, 0.f, 0.f},
            {xpos, ypos, 0.f, 1.f},
            {xpos + w, ypos, 1.f, 1.f},
            {xpos, ypos + h, 0.f, 0.f},
            {xpos + w, ypos, 1.f, 1.f},
            {xpos + w, ypos + h, 1.f, 0.f}
        };

        /* Render glyph texture over quad */
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        /* Update content of VBO memory */
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        /*  Be sure to use glBufferSubData and not glBufferData*/
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        /* Render quad */
        glDrawArrays(GL_TRIANGLES, 0, 6);

        /* Now advance cursors for next glyph (note that advance is number of 1/64 pixels) */
        /* Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels)) */
        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}
