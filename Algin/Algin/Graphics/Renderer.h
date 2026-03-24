#pragma once
#include "pch.h"

struct Vertex
{
	float position[3];
	float normal[3];
	float color[4];
	float tangent[3];
	float bitangent[3];
	float texcoord[2];
	float texid;

	/* for animation */
	int   boneIDs[4];
	float weights[4];

	Vertex()
		: position{ 0.0f, 0.0f, 0.0f },
		normal{ 0.0f, 0.0f, 0.0f },
		color{ 1.0f, 1.0f, 1.0f, 1.0f },
		tangent{ 0.0f, 0.0f, 0.0f },
		bitangent{ 0.0f, 0.0f, 0.0f },
		texcoord{ 0.0f, 0.0f },
		texid(0.0f),
		boneIDs{ 0, 0, 0, 0 },
		weights{ 0.0f, 0.0f, 0.0f, 0.0f }
	{
	}
	Vertex(float x, float y, float z, float s, float t) :
		position { x, y, z },
		normal{ 0.0f, 0.0f, 0.0f },
		color{ 1.0f, 1.0f, 1.0f, 1.0f },
		texcoord{ s, t },
		texid(0.0f),
		tangent{ 0.f,0.f,0.f },
		bitangent{ 0.f,0.f,0.f },
		boneIDs{ 0, 0, 0, 0 },
		weights{ 0.0f, 0.0f, 0.0f, 0.0f } {
	}

	inline void AddBoneData(int boneID, float weight)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (weights[i] == 0.0f)
			{
				boneIDs[i] = boneID;
				weights[i] = weight;
				return;
			}
		}

		// all 4 full → replace smallest weight if this one is stronger
		int smallest = 0;
		for (int i = 1; i < 4; ++i)
			if (weights[i] < weights[smallest])
				smallest = i;

		if (weight > weights[smallest])
		{
			boneIDs[smallest] = boneID;
			weights[smallest] = weight;
		}
	}

	// normalize weights to sum to 1.0 (optional but good for GPU precision)
	inline void NormalizeWeights()
	{
		float sum = weights[0] + weights[1] + weights[2] + weights[3];
		if (sum > 0.0f)
		{
			for (int i = 0; i < 4; ++i)
				weights[i] /= sum;
		}
	}
};

#define PIPELINE RenderPipeline::GetInstance()
class RenderPipeline : public AG::Pattern::ISingleton<RenderPipeline>
{
public:
	enum STATE
	{
		SHADOW, LIGHT, BATCH, OBJPICK
	};
	void SetPipeline(STATE state) { currentPass = state; }
	STATE GetPipeline() { return currentPass; };
	std::string GetStateStr(STATE state) {
		switch (state) {
		case LIGHT:
			return "LIGHT";
			break;
		case OBJPICK:
			return "OBJPICK";
			break;
		case SHADOW:
			return "SHADOW";
			break;
		case BATCH:
			return "BATCH";
			break;
		}
	}

private:
	STATE currentPass = SHADOW;
};
struct AABB
{
	glm::vec3 min;
	glm::vec3 max;

	AABB() : min(glm::vec3(-0.5f)), max(glm::vec3(0.5f)) {}
	AABB(const glm::vec3& min_, const glm::vec3& max_) : min(min_), max(max_) {}
	AABB(float x, float y, float z) : min(glm::vec3(-x * 0.5f, -y * 0.5f, -z * 0.5f)), max(glm::vec3(x * 0.5f, y * 0.5f, z * 0.5f)) {}
	AABB(const glm::vec3 size) : min(-size * 0.5f), max(size * 0.5f) {}
	bool contains(glm::vec3 point);
	AABB transform(glm::mat4 m2w);

};

struct OBB {
    glm::vec3 center;
    glm::vec3 halfExtents;
    glm::mat3 rotation; // 3x3 basis, from model matrix

	OBB() : center(glm::vec3(0.0f)), halfExtents(glm::vec3(0.5f)), rotation(glm::mat3(1.0f)) {}

	bool contains(const glm::vec3& p) const
	{
		// columns of rotation may be scaled
		glm::vec3 ax = rotation[0];
		glm::vec3 ay = rotation[1];
		glm::vec3 az = rotation[2];

		float sx = glm::length(ax);
		float sy = glm::length(ay);
		float sz = glm::length(az);
		const float eps = 1e-6f;
		if (sx < eps || sy < eps || sz < eps) return false;

		// normalized axes
		glm::vec3 ux = ax / sx;
		glm::vec3 uy = ay / sy;
		glm::vec3 uz = az / sz;

		// world-space half extents = local half extents scaled by axis lengths
		glm::vec3 he = halfExtents * glm::vec3(sx, sy, sz);

		glm::vec3 d = p - center;
		float px = glm::dot(d, ux);
		float py = glm::dot(d, uy);
		float pz = glm::dot(d, uz);

		return  std::abs(px) <= he.x &&
			std::abs(py) <= he.y &&
			std::abs(pz) <= he.z;
	}

	bool intersect(const OBB b)
	{
		// Local axes of each OBB
		glm::vec3 A[3] = {
			glm::normalize(rotation[0]),
			glm::normalize(rotation[1]),
			glm::normalize(rotation[2])
		};
		glm::vec3 B[3] = {
			glm::normalize(b.rotation[0]),
			glm::normalize(b.rotation[1]),
			glm::normalize(b.rotation[2])
		};

		// Half extents
		glm::vec3 aExt = halfExtents;
		glm::vec3 bExt = b.halfExtents;

		// Rotation matrix expressing b in a's coordinate frame
		float R[3][3], AbsR[3][3];
		const float EPSILON = 1e-5f;

		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				R[i][j] = glm::dot(A[i], B[j]);
				AbsR[i][j] = std::abs(R[i][j]) + EPSILON; // add epsilon to counter FP errors
			}
		}

		// Compute translation vector t from a to b, in a’s coordinates
		glm::vec3 t = b.center - center;
		t = glm::vec3(glm::dot(t, A[0]), glm::dot(t, A[1]), glm::dot(t, A[2]));

		float ra, rb;

		// Test axes L = A0, A1, A2
		for (int i = 0; i < 3; ++i)
		{
			ra = aExt[i];
			rb = bExt.x * AbsR[i][0] + bExt.y * AbsR[i][1] + bExt.z * AbsR[i][2];
			if (std::abs(t[i]) > ra + rb) return false;
		}

		// Test axes L = B0, B1, B2
		for (int i = 0; i < 3; ++i)
		{
			ra = aExt.x * AbsR[0][i] + aExt.y * AbsR[1][i] + aExt.z * AbsR[2][i];
			rb = bExt[i];
			if (std::abs(t.x * R[0][i] + t.y * R[1][i] + t.z * R[2][i]) > ra + rb) return false;
		}

		// Test cross products of axes
		// L = A0 x B0
		ra = aExt.y * AbsR[2][0] + aExt.z * AbsR[1][0];
		rb = bExt.y * AbsR[0][2] + bExt.z * AbsR[0][1];
		if (std::abs(t.z * R[1][0] - t.y * R[2][0]) > ra + rb) return false;

		// L = A0 x B1
		ra = aExt.y * AbsR[2][1] + aExt.z * AbsR[1][1];
		rb = bExt.x * AbsR[0][2] + bExt.z * AbsR[0][0];
		if (std::abs(t.z * R[1][1] - t.y * R[2][1]) > ra + rb) return false;

		// L = A0 x B2
		ra = aExt.y * AbsR[2][2] + aExt.z * AbsR[1][2];
		rb = bExt.x * AbsR[0][1] + bExt.y * AbsR[0][0];
		if (std::abs(t.z * R[1][2] - t.y * R[2][2]) > ra + rb) return false;

		// L = A1 x B0
		ra = aExt.x * AbsR[2][0] + aExt.z * AbsR[0][0];
		rb = bExt.y * AbsR[1][2] + bExt.z * AbsR[1][1];
		if (std::abs(t.x * R[2][0] - t.z * R[0][0]) > ra + rb) return false;

		// L = A1 x B1
		ra = aExt.x * AbsR[2][1] + aExt.z * AbsR[0][1];
		rb = bExt.x * AbsR[1][2] + bExt.z * AbsR[1][0];
		if (std::abs(t.x * R[2][1] - t.z * R[0][1]) > ra + rb) return false;

		// L = A1 x B2
		ra = aExt.x * AbsR[2][2] + aExt.z * AbsR[0][2];
		rb = bExt.x * AbsR[1][1] + bExt.y * AbsR[1][0];
		if (std::abs(t.x * R[2][2] - t.z * R[0][2]) > ra + rb) return false;

		// L = A2 x B0
		ra = aExt.x * AbsR[1][0] + aExt.y * AbsR[0][0];
		rb = bExt.y * AbsR[2][2] + bExt.z * AbsR[2][1];
		if (std::abs(t.y * R[0][0] - t.x * R[1][0]) > ra + rb) return false;

		// L = A2 x B1
		ra = aExt.x * AbsR[1][1] + aExt.y * AbsR[0][1];
		rb = bExt.x * AbsR[2][2] + bExt.z * AbsR[2][0];
		if (std::abs(t.y * R[0][1] - t.x * R[1][1]) > ra + rb) return false;

		// L = A2 x B2
		ra = aExt.x * AbsR[1][2] + aExt.y * AbsR[0][2];
		rb = bExt.x * AbsR[2][1] + bExt.y * AbsR[2][0];
		if (std::abs(t.y * R[0][2] - t.x * R[1][2]) > ra + rb) return false;

		// No separating axis found → boxes intersect
		return true;
	}
};

class SkyBoxRenderer : public AG::Pattern::ISingleton<SkyBoxRenderer>
{
public:
	SkyBoxRenderer();

	void RenderSkyBox(const glm::mat4& P, const glm::mat4& V, size_t cubemap_texture);

private:
	GLuint SB_VAO;
	GLuint SB_VBO;
	GLuint SB_EBO;

	GLuint locP = (GLuint)-1;
	GLuint locV = (GLuint)-1;
	GLuint locSkyBox = (GLuint) - 1;

	std::array<glm::vec3, 8> skyboxCorner =
	{
		glm::vec3(-1.f, -1.f, -1.f), // bottom-left-near
		glm::vec3( 1.f, -1.f, -1.f), // bottom-right-near
		glm::vec3( 1.f,  1.f, -1.f), // top-right-near
		glm::vec3(-1.f,  1.f, -1.f), // top-left-near
		glm::vec3(-1.f, -1.f,  1.f), // bottom-left-far
		glm::vec3( 1.f, -1.f,  1.f), // bottom-right-far
		glm::vec3( 1.f,  1.f,  1.f), // top-right-far
		glm::vec3(-1.f,  1.f,  1.f)  // top-left-far
	};

	std::array<unsigned int, 36> skyboxIndices =
	{
		// near face
		0, 1, 2, 2, 3, 0,
		// far face
		4, 5, 6, 6, 7, 4,
		// left face
		0, 3, 7, 7, 4, 0,
		// right face
		1, 5, 6, 6, 2, 1,
		// top face
		3, 2, 6, 6, 7, 3,
		// bottom face
		0, 1, 5, 5, 4, 0
	};
};

#define PP_PATH "Assets/Shaders/"
// idk if actually wants this or not
namespace AG
{

	class PostProcesser : public Pattern::ISingleton<PostProcesser>
	{
	public:
		void Init();
		void BeginScene();
		void EndScene();
		void Apply();
		void RenderEffectedScene();
		void FinalPass();
		void ApplyOverlay(int vpX, int vpY, int vpW, int vpH);
		void AddEffect(const std::string& name, GLuint shader);
		void QueueEffect(const std::string& name);
		GLuint LoadShader(const std::string& vertexPath, const std::string& fragmentPath);
		void OnResize(int width, int height); 
		std::vector<std::string> GetEffectNames()
		{
			std::vector<std::string> ret;
			for (auto& pp : PP_MAP)
			{
				if (pp.first == "final") continue;
				ret.push_back(pp.first);
			}
			return ret;
		}

		float GetGammaScaling() const { return gammaScaling; }
		void SetGammaScaling(float gamma) { gammaScaling = gamma; }
		bool IsTonemapEnabled() const { return m_enableTonemap; }
		void SetTonemapEnabled(bool enabled) { m_enableTonemap = enabled; }

	private:
		// HDR framebuffer
		GLuint hdrFBO = 0;
		GLuint hdrColorTex = 0;
		GLuint rboDepth = 0;

		// Ping-pong buffers
		GLuint pingFBO = 0, pongFBO = 0;
		GLuint pingTex = 0, pongTex = 0;
		GLuint m_currentResultTex = 0; // holds the result after all filters, for final pass to tonemap
		float gammaScaling = 2.2f;	// default gamma scaling
		bool m_enableTonemap = true;
		GLuint finalPassthrough = 0;

		std::map<std::string, GLuint> PP_MAP;   // effect name -> shader
		std::queue<std::string> process_queue;  // queued effects
		int m_width = 0, m_height = 0;
		void RenderFullscreenQuad();
	};

	enum PASS_TYPE : int
	{
		PASS_TRANSLUCENT,
		PASS_OPAQUE,
		PASS_SKYBOX,
	};

	struct Pass
	{
		std::string shader;
		std::vector<std::function<void()>> funcPtr;
	};

	class RenderPass : public Pattern::ISingleton<RenderPass>
	{
	public:
		RenderPass();

		void AddToPass(PASS_TYPE pt, std::function<void()> renderFunc);

		void Render();
		std::array<Pass, 3 >& GetPasses() { return passes; }
	private:
		std::array<Pass, 3> passes;
	};
}
