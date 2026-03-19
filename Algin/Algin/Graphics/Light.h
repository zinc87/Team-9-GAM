#pragma once
#include "pch.h"


enum LightType : int
{
	Point = 0,
	Spot = 1
};

struct Light
{
	/* --- Light Properties --- */
	LightType type{ LightType::Spot };
	glm::vec3 position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 direction{ 0.0f, -1.0f, 0.0f };
	float range = 10.f;
	float intensity = 1.f;

	//if spot light
	float fovFloat = 100.f;
	float outerCutoffCos = 0.0f;
	float innerCutoffCos = 0.0f;


	// post JAR
	bool disabled = false;
	bool castsShadow = true;
	
	/* --- Operator Overload --- */
	bool operator==(const Light& other) const;
};



class LightManager : public AG::Pattern::ISingleton<LightManager>
{
private:
	std::vector<std::weak_ptr<Light>> lightList;

public:
	bool addLight(std::shared_ptr<Light> light);

	void removeLight(std::shared_ptr<Light> light);

	/**
	 * @brief - Retrieve a std::vector of lights, for shader input
	 */
	std::vector<Light> getLightList();

	std::vector<std::weak_ptr<Light>>& getAllLights() { return lightList; }
};