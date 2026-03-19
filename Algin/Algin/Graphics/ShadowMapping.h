#pragma once
#include "../Header/pch.h"
#include "../AGEngine/Application.h"


struct ShadowEntry {
	GLuint shadowTex = 0;
	glm::mat4 lightSpace = glm::mat4(1.0f);
	int lightIndex = -1;
};

class ShadowMapping : public AG::Pattern::ISingleton<ShadowMapping>
{
public:
	void init();
	void resize(unsigned int width, unsigned int height);
	void Destroy();

	void BuildShadowSet(const std::vector<Light>& lights);

	void BindForLighting(GLuint shader);

	//per-frame usage:
	//void setLightMatrix(const Light& light);
	static glm::mat4 setLightMatrix(const Light& light);
	const glm::mat4& LightSpace() { return lightSpace; }

	void BeginShadowPass();
	void EndShadowPass();

	void LightPass(GLuint shader);

	GLuint ShadowTexID() { return shadowTex; }

	void CheckShadowMap();

private:
	void createResources();
	void deleteResources();
	void createShadowTexture(GLuint& tex);

	std::vector<ShadowEntry> m_shadows;
	int maxShadowedLights = 4;
	int m_activeShadowCount{};

	unsigned int shadowMapWidth = 2048;
	unsigned int shadowMapHeight = 2048;
	GLuint shadowFBO = 0;
	GLuint shadowTex = 0;
	glm::mat4 lightSpace = glm::mat4(1.0f);
};