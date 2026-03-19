#include "pch.h"
#include "ShadowMapping.h"

void ShadowMapping::createResources() {
	glCreateFramebuffers(1, &shadowFBO);

	// grow-only allocation
	int need = maxShadowedLights;
	int have = (int)m_shadows.size();

	if (have < need) {
		m_shadows.resize(need);
		for (int i = have; i < need; ++i) {
			if (m_shadows[i].shadowTex == 0)
				createShadowTexture(m_shadows[i].shadowTex);
		}
	}
}

void ShadowMapping::createShadowTexture(GLuint& tex) {

	//depth texture(shadow map)
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);

	glTextureStorage2D(tex, 1, GL_DEPTH_COMPONENT24, shadowMapWidth, shadowMapHeight);
	
	glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	float border[4] = { 1,1,1,1 };
	glTextureParameterfv(tex, GL_TEXTURE_BORDER_COLOR, border);
	glTextureParameteri(tex, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTextureParameteri(tex, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}

void ShadowMapping::deleteResources() {
	if (shadowFBO) { glDeleteFramebuffers(1, &shadowFBO); shadowFBO = 0; }
	for (auto& s : m_shadows) { if (s.shadowTex) glDeleteTextures(1, &s.shadowTex); s.shadowTex = 0; }
	m_shadows.clear();
	m_activeShadowCount = 0;
}

void ShadowMapping::init() {
	shadowMapWidth = 4096;
	shadowMapHeight = 4096;
	deleteResources();
	createResources();
}

void ShadowMapping::resize(unsigned int width, unsigned int height) {
	if (width == shadowMapWidth && height == shadowMapHeight) return;
	for (auto& s : m_shadows) {
		if (s.shadowTex) glDeleteTextures(1, &s.shadowTex);
		createShadowTexture(s.shadowTex);
	}
}

void ShadowMapping::Destroy() {
	deleteResources();
	shadowMapWidth = shadowMapHeight = 0;
	//lightSpace = glm::mat4(1.0f);
}

//float deg2rad(float d) { return d * 3.1415926535f / 180.0f; }

void ShadowMapping::BuildShadowSet(const std::vector<Light>& lights) {
	
	//store indices of lights that cast a shadow
	std::vector<int> lightIndices;
	lightIndices.reserve(lights.size());
	for (size_t i = 0; i < lights.size(); ++i) {
		if (lights[i].castsShadow) {
			lightIndices.push_back(static_cast<int>(i));
		}
	}

	//max lights that cast shadow = 4
	m_activeShadowCount = std::min((int)lightIndices.size(), maxShadowedLights);

	for (int s = 0; s < m_activeShadowCount; ++s)
		m_shadows[s].lightIndex = lightIndices[s];

}

void ShadowMapping::BindForLighting(GLuint shader) {
	
	int const currNumShadow = std::min(static_cast<int>(m_activeShadowCount), maxShadowedLights);

	if (GLint loc = glGetUniformLocation(shader, "uNumShadowed"); loc != -1)
		glUniform1i(loc, currNumShadow);


	if (currNumShadow > 0) {
		std::vector<GLint> shadowTexUnits(currNumShadow);
		for (int i = 0; i < currNumShadow; ++i) {
			shadowTexUnits[i] = 3 + i;
			glActiveTexture(GL_TEXTURE0 + shadowTexUnits[i]);
			glBindTexture(GL_TEXTURE_2D, m_shadows[i].shadowTex);
		}

		if (GLint loc = glGetUniformLocation(shader, "uShadowMap[0]"); loc != -1)
			glUniform1iv(loc, currNumShadow, shadowTexUnits.data());
	}


	for (int i = 0; i < currNumShadow; ++i) {
		std::string mname = "uLightSpace[" + std::to_string(i) + "]";
		if (GLint loc = glGetUniformLocation(shader, mname.c_str()); loc != -1)
			glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_shadows[i].lightSpace));

		std::string iname = "uLightShadowIdx[" + std::to_string(i) + "]";
		if (GLint loc = glGetUniformLocation(shader, iname.c_str()); loc != -1)
			glUniform1i(loc, m_shadows[i].lightIndex);  // which uLights[] this shadow belongs to
	}
}

//void ShadowMapping::setLightMatrix(const Light& light) {
//	
//	glm::vec3 dir = glm::normalize(light.direction);
//
//	if (glm::length(dir) < 1e-6f) dir = glm::vec3(0, -1, 0); // avoid zero look vector
//	glm::vec3 up = (std::abs(dir.y) > 0.99f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
//
//	float outerAngleDeg = 100.0f;      
//	float fovy = glm::radians(outerAngleDeg * 2.0f);
//	float nearP = 0.1f;
//	float farP = light.range * 2.f; 
//
//	glm::mat4 lightProj = glm::perspective(fovy, 1.0f, nearP, farP);
//
//	glm::mat4 lightView = glm::lookAt(
//		light.position,
//		light.position + dir, // look along the lights direction
//		up               
//	);
//
//	lightSpace = lightProj * lightView;
//}

glm::mat4 ShadowMapping::setLightMatrix(const Light& light) {

	glm::vec3 dir = glm::normalize(light.direction);

	if (glm::length(dir) < 1e-6f) dir = glm::vec3(0, -1, 0); // avoid zero look vector
	glm::vec3 up = (std::abs(dir.y) > 0.99f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

	float fovy = glm::clamp(glm::radians(light.fovFloat), glm::radians(5.0f), glm::radians(120.0f));
	float nearP = 0.1f;                
	float farP = light.range * 2.f;          

	glm::mat4 lightProj = glm::perspective(fovy, 1.0f, nearP, farP);

	glm::mat4 lightView = glm::lookAt(
		light.position,
		light.position + dir, // look along the lights direction
		up
	);

	return lightProj * lightView;
}

void ShadowMapping::BeginShadowPass() {
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowFBO);
	glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	glEnable(GL_DEPTH_TEST);


	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.5f, 4.0f);
	
	SHADERMANAGER.Use("SHADOW_PASS");
	GLuint shadowProg = SHADERMANAGER.GetShaderProgram();

	const std::vector<Light>& lights = LightManager::GetInstance().getLightList();

	for (int s = 0; s < m_activeShadowCount; ++s) {
		auto& S = m_shadows[s];
		const Light& L = lights[S.lightIndex];
		S.lightSpace = setLightMatrix(L);

		glNamedFramebufferTexture(shadowFBO, GL_DEPTH_ATTACHMENT, S.shadowTex, 0);
		glNamedFramebufferDrawBuffer(shadowFBO, GL_NONE);
		glNamedFramebufferReadBuffer(shadowFBO, GL_NONE);

		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.5f, 4.0f);

		if (LightManager::GetInstance().getLightList().size() > 0) {
			GLint locLight = glGetUniformLocation(shadowProg, "uLightSpace");
			if (locLight != -1) {
				glUniformMatrix4fv(locLight, 1, GL_FALSE, glm::value_ptr(S.lightSpace));
			}
		}

		RenderPipeline::GetInstance().SetPipeline(RenderPipeline::SHADOW);
		COMPONENTMANAGER.LateUpdate();

		
	}
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void ShadowMapping::EndShadowPass() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapping::LightPass(GLuint shader) {

	glViewport(0, 0, AGWINDOW.getViewportSize().first, AGWINDOW.getViewportSize().second);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	BindForLighting(shader);
}

void ShadowMapping::CheckShadowMap() {
	std::vector<float> buf(shadowMapWidth * shadowMapHeight);
	glGetTextureImage(shadowTex, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		GLsizei(buf.size() * sizeof(float)), buf.data());

	auto [minIt, maxIt] = std::minmax_element(buf.begin(), buf.end());
	std::string debug = "ShadowMap depth min=" + std::to_string(*minIt) + " max= " + std::to_string(*minIt) + "\n";
	
	AG_CORE_INFO(debug);

}