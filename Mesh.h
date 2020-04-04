#pragma once

#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

class Shader;

struct Vertex
{
	glm::vec3 Position;

	glm::vec3 Normal;

	glm::vec2 TexCoords;

	glm::vec3 Tangent;

	glm::vec3 Bitangent;
};

struct Texture
{
	unsigned int id;

	std::string type;

	std::string path;
};

class Mesh
{
public:
	/* Mesh Data */
	std::vector<Vertex> vertices;

	std::vector<unsigned int> indices;

	std::vector<Texture> textures;

	unsigned int VAO{};

	/* Functions */
	/* constructor */
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
	     const std::vector<Texture>& textures);

	/* render the mesh */
	void Draw(const Shader& shader) const;

private:
	/* Render data */
	unsigned int VBO{}, EBO{};

	/* Functions */
	/* initializes all the buffer objects/arrays */
	void setupMesh();
};
#endif
