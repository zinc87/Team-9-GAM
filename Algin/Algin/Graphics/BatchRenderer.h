#pragma once
#include "pch.h"

#define MAX_VERTICE 30000

struct Color
{
	union
	{
		glm::vec4 rgba;

		float rgba_f[4];
	};

	Color() : rgba{ 1.f,0.f,1.f,1.f } {}
	Color(float r, float g, float b, float a = 1.f) : rgba{ r,g,b,a} {}
};

struct TextureCoordinate
{
	float textureCoord[4][2];

	TextureCoordinate()
	{
		textureCoord[0][0] = 0.f;
		textureCoord[0][1] = 1.f;//Top left

		textureCoord[1][0] = 1.f;
		textureCoord[1][1] = 1.f;//Top Right

		textureCoord[2][0] = 1.f;
		textureCoord[2][1] = 0.f;//Btm Left

		textureCoord[3][0] = 0.f;
		textureCoord[3][1] = 0.f;//Btm right
	}
};

struct Transform2D
{
	glm::vec3 position{ 0.0f, 0.0f, 1.0f };
	glm::vec2 scale{ 1.0f, 1.0f };
	float rotation = 0.0f; // in degrees

	glm::mat3 getM2W() const
	{
		float rad = glm::radians(rotation);
		float cosR = cos(rad);
		float sinR = sin(rad);
		glm::mat3 translationMat = {
			1.0f, 0.0f, position.x,
			0.0f, 1.0f, position.y,
			0.0f, 0.0f, 1.0f
		};
		glm::mat3 rotationMat = {
			cosR, -sinR, 0.0f,
			sinR,  cosR, 0.0f,
			0.0f,  0.0f, 1.0f
		};
		glm::mat3 scaleMat = {
			scale.x, 0.0f,    0.0f,
			0.0f,    scale.y, 0.0f,
			0.0f,    0.0f,    1.0f
		};
		return translationMat * rotationMat * scaleMat;
	}

	glm::mat4 getM2W4() const
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1));
		glm::mat4 scaling = glm::scale(glm::mat4(1.0f), glm::vec3(scale, 1.0f));
		return translation * rotationZ * scaling;
	}

	Transform2D() : position(0.f, 0.f, 0.f), scale{ 1.f,1.f }, rotation(0.f) {}
};

struct BatchVertex
{
	float position[3];
	float normal[3];     // nx, ny, nz   <-- added for lighting
	float color[4];
	float texcoord[2];
	float texid;
	size_t tex_hash;
};

class BatchRenderer : public AG::Pattern::ISingleton<BatchRenderer>
{
public:
	void Init();
	void Terminate();
	void BeginBatch();
	void EndBatch();
	void Render();
	void Flush();

	void DrawQuad(Color clr, Transform2D &trf, size_t tex_hash_id, TextureCoordinate texCoord = TextureCoordinate());

	void RenderText(std::string content, Color clr, Transform2D& trf,
		size_t font_hash_id, bool wrapped, float wrap_limit,
		bool left_aligned, bool centered, bool right_aligned,
		float font_size, float line_spacing, float letter_spacing);

private:
	BatchVertex* quadBuffer=nullptr;
	BatchVertex* quadBuffer_ite=nullptr;

	GLushort indiceCount{};
	GLuint		quadCount{};
	GLuint		renderCount{};


	GLuint VAO=0;
	GLuint VBO=0;
	GLuint IBO=0;
	std::unordered_map<size_t, float> textureHashMap;
	int textureSlotCounter = 0;
	float textureSlotAssign = 0.f;

	int* sampler{};
};