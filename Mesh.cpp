#include "Mesh.h"
#include "glad/glad.h"
#include "Shader.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
           const std::vector<Texture>& textures)
{
	this->vertices = vertices;

	this->indices = indices;

	this->textures = textures;

	/* now that we have all the required data, set the vertex buffers and its attribute pointers. */
	setupMesh();
}

void Mesh::Draw(const Shader& shader) const
{
	/* bind appropriate textures */
	unsigned int diffuseNr = 0;

	unsigned int specularNr = 0;

	unsigned int normalNr = 0;

	unsigned int heightNr = 0;

	for (auto i = 0u; i < textures.size(); ++i)
	{
		/* active proper texture unit before binding */
		glActiveTexture(GL_TEXTURE0 + i);

		/* retrieve texture number (the N in diffuse_textureN) */
		std::string number;

		auto name = textures[i].type;

		if (name == "texture_diffuse")
		{
			/* transfer unsigned int to stream */
			number = std::to_string(diffuseNr++);
		}
		else if (name == "texture_specular")
		{
			/* transfer unsigned int to stream */
			number = std::to_string(specularNr++);
		}
		else if (name == "texture_normal")
		{
			/* transfer unsigned int to stream */
			number = std::to_string(normalNr++);
		}
		else if (name == "texture_height")
		{
			/* transfer unsigned int to stream */
			name = std::to_string(heightNr++);
		}

		/* now set the sampler to the correct texture unit */
		shader.setInt(name + number, i);

		/* and finally bind the texture */
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	/* draw mesh */
	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(0);

	/* always good practice to set everything back to defaults once configured. */
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh()
{
	/* create buffers/arrays */
	glGenVertexArrays(1, &VAO);

	glGenBuffers(1, &VBO);

	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	/* load data into vertex buffers */
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	/*
	 * A great thing about structs is that their memory layout is sequential for all its items.
	 * The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	 * again translates to 3/2 floats which translates to a byte array.
	 */
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	/* set the vertex attribute pointers */
	// ------------------------------
	/* vertex Positions */
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void*>(nullptr));

	/* vertex normals */
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Normal)));

	/* vertex texture coords */
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      reinterpret_cast<void*>(offsetof(Vertex, TexCoords)));

	/* vertex tangent */
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Tangent)));

	/* vertex bitangent */
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      reinterpret_cast<void*>(offsetof(Vertex, Bitangent)));

	glBindVertexArray(0);
}
