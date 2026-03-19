#include "pch.h"
#include "Cube.h"




void CubeRenderer::computeTangent(CubeVertex* vertices, size_t vertexCount,
	const std::vector<GLuint>& indices, size_t indexCount) {

	// Clear accumulators
	for (size_t i = 0; i < vertexCount; ++i) {
		vertices[i].tangent = glm::vec3(0.0f);
	}

	// Accumulate per triangle
	for (size_t i = 0; i < indexCount; i += 3) {
		uint32_t i0 = indices[i + 0];
		uint32_t i1 = indices[i + 1];
		uint32_t i2 = indices[i + 2];

		CubeVertex& v0 = vertices[i0];
		CubeVertex& v1 = vertices[i1];
		CubeVertex& v2 = vertices[i2];

		glm::vec3 p0 = v0.position;
		glm::vec3 p1 = v1.position;
		glm::vec3 p2 = v2.position;

		glm::vec2 uv0 = v0.texcoord;
		glm::vec2 uv1 = v1.texcoord;
		glm::vec2 uv2 = v2.texcoord;

		glm::vec3 deltaPos1 = p1 - p0;
		glm::vec3 deltaPos2 = p2 - p0;

		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f /
			(deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent;
	}
}


void CubeRenderer::Setup() 
{
	std::vector<GLuint> indices = {
		// +Z
		0,1,2,  0,2,3,
		// -Z
		4,5,6,  4,6,7,
		// -X
		8,9,10, 8,10,11,
		// +X
		12,13,14, 12,14,15,
		// +Y
		16,17,18, 16,18,19,
		// -Y
		20,21,22, 20,22,23
	};

	////initialize the space needed for UBO
	////change values for default material params
	//matParams.baseColor = { 1.f, 1.f, 1.f, 1.f };
	//matParams.mru_pad = { 0.0f, 0.0f, 1.0f, 1.0f };

	computeTangent(verts.data(), verts.size(), indices, indices.size());

	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);

	glCreateBuffers(1, &materialUBO);
	glNamedBufferData(materialUBO, sizeof(MaterialParams), 0, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, materialUBO);

	glNamedBufferData(VBO, static_cast<GLsizeiptr>(verts.size() * sizeof(CubeVertex)),
		verts.data(), GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(CubeVertex));

	//position
	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(CubeVertex, position)));
	glVertexArrayAttribBinding(VAO, 0, 0);

	//uv
	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(CubeVertex, texcoord)));
	glVertexArrayAttribBinding(VAO, 1, 0);

	//normal
	glEnableVertexArrayAttrib(VAO, 2);
	glVertexArrayAttribFormat(VAO, 2, 3, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(CubeVertex, normal)));
	glVertexArrayAttribBinding(VAO, 2, 0);

	//tangent
	glEnableVertexArrayAttrib(VAO, 3);
	glVertexArrayAttribFormat(VAO, 3, 3, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(CubeVertex, tangent)));
	glVertexArrayAttribBinding(VAO, 3, 0);

	glNamedBufferData(EBO, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	glVertexArrayElementBuffer(VAO, EBO);
}

void CubeRenderer::drawCube(const glm::mat4 m2w, const matAsset& mat)
{
	auto camera = CameraManager::GetInstance().getSceneCamera();
	if (!camera)
		return;

	glBindVertexArray(VAO);

	SHADERMANAGER.Use("ShaderTemplate");
	GLuint program = SHADERMANAGER.GetShaderProgram();


	// Lights
	const auto lights = LightManager::GetInstance().getLightList();
	constexpr int MAX_LIGHTS = 16;
	int count = std::min<int>((int)lights.size(), MAX_LIGHTS);

	if (GLint locCount = glGetUniformLocation(program, "uLightCount"); locCount != -1)
		glUniform1i(locCount, count);

	for (int i = 0; i < count; ++i) {
		const Light& L = lights[i];
		const std::string base = "uLights[" + std::to_string(i) + "].";

		if (GLint loc = glGetUniformLocation(program, (base + "type").c_str()); loc != -1)
			glUniform1i(loc, (int)L.type);
		if (GLint loc = glGetUniformLocation(program, (base + "position").c_str()); loc != -1)
			glUniform3fv(loc, 1, glm::value_ptr(L.position));
		if (GLint loc = glGetUniformLocation(program, (base + "direction").c_str()); loc != -1)
			glUniform3fv(loc, 1, glm::value_ptr(glm::normalize(L.direction)));
		if (GLint loc = glGetUniformLocation(program, (base + "range").c_str()); loc != -1)
			glUniform1f(loc, L.range);
		if (GLint loc = glGetUniformLocation(program, (base + "intensity").c_str()); loc != -1)
			glUniform1f(loc, L.intensity);
	}
	//Lights

	// Camera position for specular
	if (GLint locViewPos = glGetUniformLocation(program, "camPos"); locViewPos != -1) {
		glm::vec3 camPos = camera->getCameraPosition();
		glUniform3fv(locViewPos, 1, glm::value_ptr(camPos));
	}

	GLint locP = glGetUniformLocation(program, "P");
	GLint locV = glGetUniformLocation(program, "V");
	GLint locM = glGetUniformLocation(program, "M");

	//albedo map
	auto weakTexAss = AssetManager::GetInstance().GetAsset(mat.albedoHashID);
	std::shared_ptr<TextureAsset> shrd_tex_ass;
	if (weakTexAss.lock()) {
		shrd_tex_ass = std::dynamic_pointer_cast<TextureAsset>(weakTexAss.lock());
	}

	GLint loc_hasAlbedo = glGetUniformLocation(program, "hasAlbedoMap");
	if (mat.albedoHashID > 0) {
		GLint loc_uAlbedoMap = glGetUniformLocation(program, "uAlbedoMap");
		glBindTextureUnit(0, shrd_tex_ass->textureID);
		glProgramUniform1i(program, loc_uAlbedoMap, 0);
		glProgramUniform1i(program, loc_hasAlbedo, 1);
	}
	else {
		glProgramUniform1i(program, loc_hasAlbedo, 0);

	}

	//normal map
	auto weakNormalTexAss = AssetManager::GetInstance().GetAsset(mat.normalMapHashID);
	std::shared_ptr<TextureAsset> shrd_tex_ass_normal;
	if (weakNormalTexAss.lock()) {
		shrd_tex_ass_normal = std::dynamic_pointer_cast<TextureAsset>(weakNormalTexAss.lock());
	}
	GLint loc_hasNormalMap = glGetUniformLocation(program, "hasNormalMap");
	if (mat.normalMapHashID > 0) {
		GLint loc_uNormalMap = glGetUniformLocation(program, "uNormalMap");
		glBindTextureUnit(1, shrd_tex_ass_normal->textureID);
		glProgramUniform1i(program, loc_uNormalMap, 1);
		glProgramUniform1i(program, loc_hasNormalMap, 1);
	}
	else {
		glProgramUniform1i(program, loc_hasNormalMap, 0);
	}



	glNamedBufferSubData(materialUBO, 0, sizeof(mat.params), &mat.params);



	if (locP != -1) glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
	if (locV != -1) glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	if (locM != -1) glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(m2w));
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, materialUBO);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}