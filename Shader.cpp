#include "Shader.h"
#include <iostream>
#include <fstream>

Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
{
	/* 1. retrieve the vertex/fragment source code from filePath */
	// ------------------------------
	std::string vertexCode;

	std::string fragmentCode;

	std::ifstream vShaderFile;

	std::ifstream fShaderFile;

	/* ensure ifstream objects can throw exceptions: */
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		/* open files */
		vShaderFile.open(vertexPath);

		fShaderFile.open(fragmentPath);

		std::stringstream vShaderStream, fShaderStream;

		/* read file's buffer contents into streams */
		vShaderStream << vShaderFile.rdbuf();

		fShaderStream << fShaderFile.rdbuf();

		/* close file handlers */
		vShaderFile.close();

		fShaderFile.close();

		/* convert stream into string */
		vertexCode = vShaderStream.str();

		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}

	const auto vShaderCode = vertexCode.c_str();

	const auto fShaderCode = fragmentCode.c_str();

	/* 2. compile shaders */
	// ------------------------------
	unsigned int vertex, fragment;

	/* vertex shader */
	vertex = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertex, 1, &vShaderCode, nullptr);

	glCompileShader(vertex);

	checkCompileErrors(vertex, "VERTEX");

	/* fragment Shader */
	fragment = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragment, 1, &fShaderCode, nullptr);

	glCompileShader(fragment);

	checkCompileErrors(fragment, "FRAGMENT");

	ID = glCreateProgram();

	glAttachShader(ID, vertex);

	glAttachShader(ID, fragment);

	glLinkProgram(ID);

	checkCompileErrors(ID, "PROGRAM");

	/* delete the shaders as they're linked into our program now and no longer necessary */
	glDeleteShader(vertex);

	glDeleteShader(fragment);
}

void Shader::use() const
{
	glUseProgram(ID);
}

void Shader::setBool(const std::string& name, const bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, const int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, const float value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::checkCompileErrors(const unsigned shader, const std::string type)
{
	int success;

	char infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, nullptr, infoLog);

			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog <<
				"\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);

		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, nullptr, infoLog);

			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog <<
				"\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}
