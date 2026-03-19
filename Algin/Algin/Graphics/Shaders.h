#pragma once
#include "pch.h"

class ShaderManager : public AG::Pattern::ISingleton<ShaderManager>
{
public:

	void LoadShader(std::string vertex_path, std::string fragment_path);
	bool createCompileLink(const char* vertex_path, const char* fragment_path);
	GLuint createCompileShader(GLchar const* shader_code[], int shader_type);
	void LinkShader(GLuint& shader_program, GLuint& vert_shader, GLuint& frag_shader);
	void Use(std::string shader_name);
	void Use();
	GLuint GetShaderProgram();
	void PrintOut();

	void LoadAllShaders();

	std::vector<std::string> getShaderNames();

private:
	GLuint program{};
	std::map<std::string, GLuint> programMap;

};

#define SHADERMANAGER ShaderManager::GetInstance()