#include "pch.h"
#include "Shaders.h"


void ShaderManager::LoadAllShaders() {

	SHADERMANAGER.LoadShader("Assets/Shaders/Algin_3D.vert", "Assets/Shaders/Algin_3D.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/Algin_3D_Exclude.vert", "Assets/Shaders/Algin_3D_Exclude.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/Algin_Default.vert", "Assets/Shaders/Algin_Default.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/Algin_CubeMap.vert", "Assets/Shaders/Algin_CubeMap.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/SHADOW_PASS.vert", "Assets/Shaders/SHADOW_PASS.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/Algin_Batch.vert", "Assets/Shaders/Algin_Batch.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/ObjPicking.vert", "Assets/Shaders/ObjPicking.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/Particle.vert", "Assets/Shaders/Particle.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/SkinnedShader.vert", "Assets/Shaders/SkinnedShader.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/SkinnedShader_Discard.vert", "Assets/Shaders/SkinnedShader_Discard.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/ShaderTemplate.vert", "Assets/Shaders/ShaderTemplate.frag");
	SHADERMANAGER.LoadShader("Assets/Shaders/VideoYUV.vert", "Assets/Shaders/VideoYUV.frag");

}

std::vector<std::string> ShaderManager::getShaderNames()
{
	std::vector<std::string> names;
	for (auto& shader : programMap)
	{
		names.push_back(shader.first);
	}
	return names;
}

void ShaderManager::LoadShader(std::string vertex_path, std::string fragment_path)
{
	std::string shader_name = fragment_path.substr(fragment_path.find_last_of('/') + 1, fragment_path.find_last_of('.') - fragment_path.find_last_of('/') - 1);
	createCompileLink(vertex_path.c_str(), fragment_path.c_str());
	programMap[shader_name] = program;
}

bool ShaderManager::createCompileLink(const char* vertex_path, const char* fragment_path)
{
	program = glCreateProgram();

	std::ifstream vertex_file(vertex_path);
	if (!vertex_file) {
		AG_CORE_WARN("Unable to open Vertex ShaderManager File!");
		return false;
	}

	std::ifstream fragment_file(fragment_path);
	if (!fragment_file) {
		AG_CORE_WARN("Unable to open Fragment ShaderManager File!");
		return false;
	}

	std::stringstream buffer_vertex, buffer_fragment;
	buffer_vertex << vertex_file.rdbuf();
	buffer_fragment << fragment_file.rdbuf();

	vertex_file.close();
	fragment_file.close();

	std::string vertex_str = buffer_vertex.str();
	std::string fragment_str = buffer_fragment.str();

	GLchar const* vertex_code[] = { vertex_str.c_str() };
	GLchar const* fragment_code[] = { fragment_str.c_str() };

	GLuint vertexShader = createCompileShader(vertex_code, GL_VERTEX_SHADER);
	GLuint fragmentShader = createCompileShader(fragment_code, GL_FRAGMENT_SHADER);

	LinkShader(program, vertexShader, fragmentShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return true;
}

GLuint ShaderManager::createCompileShader(GLchar const* shader_code[], int shader_type)
{
	GLuint temp_shader{};
	GLint success;
	if (shader_type != GL_VERTEX_SHADER && shader_type != GL_FRAGMENT_SHADER)
	{
		AG_CORE_WARN("Invalid ShaderManager Type!");
		return temp_shader;
	}
	temp_shader = glCreateShader(shader_type);
	glShaderSource(temp_shader, 1, shader_code, NULL);
	glCompileShader(temp_shader);
	glGetShaderiv(temp_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		int length = 0;
		switch (shader_type)
		{
		case GL_VERTEX_SHADER:
			AG_CORE_WARN("Vertex ShaderManager failed to compile!");
			glGetShaderiv(temp_shader, GL_INFO_LOG_LENGTH, &length);  // Get error log length.
			if (length > 0)
			{
				std::string log(length, ' ');
				int written = 0;
				glGetShaderInfoLog(temp_shader, length, &written, &log[0]);  // Get compilation error log.
				std::cout << log << std::endl;
			}
			break;
		case GL_FRAGMENT_SHADER:
			AG_CORE_WARN("Fragment ShaderManager failed to compile!");
			glGetShaderiv(temp_shader, GL_INFO_LOG_LENGTH, &length);  // Get error log length.
			if (length > 0)
			{
				std::string log(length, ' ');
				int written = 0;
				glGetShaderInfoLog(temp_shader, length, &written, &log[0]);  // Get compilation error log.
				AG_CORE_WARN(log);
			}
			break;
		}
	}
	else
	{
		switch (shader_type)
		{
		case GL_VERTEX_SHADER:
			//AG_CORE_WARN("Vertex ShaderManager compiled successfully!");
			break;
		case GL_FRAGMENT_SHADER:
			//AG_CORE_WARN("Fragment ShaderManager compiled successfully!");
			break;
		}
	}
	return temp_shader;
}

void ShaderManager::LinkShader(GLuint& shader_program, GLuint& vert_shader, GLuint& frag_shader)
{
	GLint success;
	glAttachShader(shader_program, vert_shader);
	glAttachShader(shader_program, frag_shader);
	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		std::cout << "ShaderManager program failed to link!";
	}
}

void ShaderManager::Use(std::string shader_name)
{
	program = programMap[shader_name];
	glUseProgram(programMap[shader_name]);
}

void ShaderManager::Use()
{
	glUseProgram(program);
}

GLuint ShaderManager::GetShaderProgram()
{
	return program;
}

void ShaderManager::PrintOut()
{
	AG_CORE_WARN("============SHADERS===========");
	for (auto shader : programMap)
	{
		AG_CORE_WARN("ShaderManager Name: {}", shader.first);
		AG_CORE_WARN("Program Number: {}", shader.second);
	}
	AG_CORE_WARN("==============================");
}
