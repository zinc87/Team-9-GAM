#pragma once
#include "pch.h"


struct ScanParams {
	glm::vec3 centerWS;
	float radius;
	float refRad;
	int active;
	glm::vec2 centerNDC;
	float radNDC;
	

	ScanParams() : centerWS{ glm::vec3(0.0f, 5.f, 0.0f) },
		radius{ 1.0f }, refRad{ 0.01f }, active{ 0 } {
	}
};

struct Ray
{
	glm::vec3 origin;
	glm::vec3 dir;  // normalized
};

class ScanSetup : public AG::Pattern::ISingleton<ScanSetup>
{
public:
	Ray BuildRay(float mouseX, float mouseY,
		float viewportW, float viewportH,
		const glm::mat4& view,
		const glm::mat4& proj,
		const glm::vec3& camPos);

	bool RayAABB(const Ray& ray, const glm::vec3& bmin, const glm::vec3& bmax, float& tHit);

	bool RayTriangle(const Ray& ray,
		const glm::vec3& v0,
		const glm::vec3& v1,
		const glm::vec3& v2,
		float& tOut);
	
	bool ScanHit(const Ray& ray, glm::vec3& outHitPos, float& outRef,
		std::vector<SubMeshSkinned>& gSubMeshes, const glm::mat4& modelMat);



	ImVec2 scanMousePos;
	ImVec2 viewDims;

	inline static bool scanning = false;
};