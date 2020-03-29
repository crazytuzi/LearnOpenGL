#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <sstream>

class Shader
{
public:
	unsigned int ID;

	/* constructor generates the shader on the fly */
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

	/* activate the shader */
	void use() const;

	/* utility uniform functions */
	void setBool(const std::string& name, bool value) const;

	void setInt(const std::string& name, int value) const;

	void setFloat(const std::string& name, float value) const;

private:
	/* utility function for checking shader compilation/linking errors. */
	static void checkCompileErrors(unsigned int shader, std::string type);
};

#endif
