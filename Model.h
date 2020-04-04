#pragma once

#ifndef MODEL_H
#define MODEL_H
#include <string>
#include <vector>
#include "Mesh.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
struct Texture;
class Shader;

class Model
{
public:
	/* Model Data */
	// ------------------------------
	/* stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once. */
	std::vector<Texture> textures_loaded;

	std::vector<Mesh> meshes;

	std::string directory;

	bool gammaCorrection;

	/* Functions */
	// ------------------------------
	/* constructor, expects a filepath to a 3D model. */
	Model(const std::string& path, bool gamma = false);

	/* draws the model, and thus all its meshes */
	void Draw(const Shader& shader) const;

private:
	/* Functions */
	// ------------------------------
	/* loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector. */
	void loadModel(const std::string& path);

	/*
	 * processes a node in a recursive fashion.
	 * Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	 */
	void processNode(aiNode* node, const aiScene* scene);

	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	/*
	 * checks all material textures of a given type and loads the textures if they're not loaded yet.
	 * the required info is returned as a Texture struct.
	 */
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
};
#endif
