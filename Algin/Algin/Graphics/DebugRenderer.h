#pragma once
#include "pch.h"

//static const char* vertexSrc() {
//	return R"(#version 450 core
//layout(location=0) in vec3 aPos;
//layout(location=1) in vec2 aTexCoord;
//
//uniform mat4 P;
//uniform mat4 V;
//uniform mat4 M;
//uniform vec2 vTexOffset;
//out vec2 vTexCoord;
//
//void main() {
//    vTexCoord = aTexCoord + vTexOffset;
//    gl_Position = P * V * M * vec4(aPos, 1.0);
//})";
//}
//
//static const char* fragmentSrc() {
//	return R"(#version 450 core
//out vec4 FragColor;
//
//in vec2 vTexCoord;
//
//uniform vec4 uColor;
//uniform bool uHasTexture;
//uniform sampler2D uTexture;
//
//void main() { 
//    if (uHasTexture) {
//        FragColor = texture(uTexture, vTexCoord);
//    } else {
//        FragColor = uColor; 
//    }
//})";
//}

struct DebugVertex
{
	glm::vec3 position;
	glm::vec2 texcoord;

	DebugVertex(float x, float y, float z, float s, float t) : position(x,y,z) , texcoord(s,t) {}
};

class DebugRenderer : public AG::Pattern::ISingleton<DebugRenderer>
{
private:
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint program;
	std::array<DebugVertex, 8> frustumCorner =
	{
		DebugVertex(-1.f,-1.f,-1.f, 0.f, 0.f),
		DebugVertex(1.f,-1.f,-1.f, 0.f, 0.f),
		DebugVertex(1.f, 1.f,-1.f, 0.f, 0.f),
		DebugVertex(-1.f, 1.f,-1.f, 0.f, 0.f),
		DebugVertex(-1.f,-1.f, 1.f, 0.f, 0.f),
		DebugVertex(1.f,-1.f, 1.f, 0.f, 0.f),
		DebugVertex(1.f, 1.f, 1.f, 0.f, 0.f),
		DebugVertex(-1.f, 1.f, 1.f, 0.f, 0.f)
	};

	std::array<DebugVertex, 8> aabbCorner =
	{
		DebugVertex(-1.f,-1.f,-1.f, 0.f, 0.f),
		DebugVertex(1.f,-1.f,-1.f, 0.f, 0.f),
		DebugVertex(1.f, 1.f,-1.f, 0.f, 0.f),
		DebugVertex(-1.f, 1.f,-1.f, 0.f, 0.f),
		DebugVertex(-1.f,-1.f, 1.f, 0.f, 0.f),
		DebugVertex(1.f,-1.f, 1.f, 0.f, 0.f),
		DebugVertex(1.f, 1.f, 1.f, 0.f, 0.f),
		DebugVertex(-1.f, 1.f, 1.f, 0.f, 0.f)
	};
	std::array<DebugVertex, 8> 	cubeCorner =
	{
		DebugVertex(-0.2f, -0.2f, -0.2f, 0.f, 0.f),
		DebugVertex(0.2f, -0.2f, -0.2f, 0.f, 0.f),
		DebugVertex(0.2f,  0.2f, -0.2f, 0.f, 0.f),
		DebugVertex(-0.2f,  0.2f, -0.2f, 0.f, 0.f),
		DebugVertex(-0.2f, -0.2f,  0.2f, 0.f, 0.f),
		DebugVertex(0.2f, -0.2f,  0.2f, 0.f, 0.f),
		DebugVertex(0.2f,  0.2f,  0.2f, 0.f, 0.f),
		DebugVertex(-0.2f,  0.2f,  0.2f, 0.f, 0.f)
	};


	GLint locP = -1;
	GLint locV = -1;
	GLint locM = -1;
	GLint locColor = -1;
	GLint locTexture = -1;
	GLint locTextureOffset = -1;
	GLint locHasTexture = -1;

	/* --- Y=0 Plane --- */
	GLuint planeVAO = 0;
	GLuint planeVBO = 0;
	GLuint planeEBO = 0;
	std::array<DebugVertex, 4> planeCorner =
	{
		DebugVertex(-1000.f,  0.f, -1000.f, 1000.f, 0.f),
		DebugVertex( 1000.f,  0.f, -1000.f, 0.f, 0.f),
		DebugVertex( 1000.f,  0.f,  1000.f, 0.f, 1000.f),
		DebugVertex(-1000.f,  0.f,  1000.f, 1000.f, 1000.f)
	};

public:
	DebugRenderer();

	~DebugRenderer()
	{
		if (EBO) { glDeleteBuffers(1, &EBO); EBO = 0; }
		if (VBO) { glDeleteBuffers(1, &VBO); VBO = 0; }
		if (VAO) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
	}

	void Setup();
	/* -- For Frustum Wireframe and Cube Block --- */
	void UpdateCorner(std::array<DebugVertex, 8> vertices);

	/* --- Frustum Wireframe --- */
	void DrawWireframe(const glm::mat4 P, const glm::mat4 V);
	/* --- Frustum Wireframe --- */

	/* --- Cube Block --- */
	void DrawCube(const glm::mat4 m2w);
	void DrawCubeColor(const glm::mat4 m2w, AG::Data::GUID objID);
	/* --- Cube Block --- */



	/* --- AABB & OBB Wireframe--- */
	void UpdateCorner(const glm::mat4 m2w, const AABB aabb);
	void UpdateCorner(const OBB obb);

	void DrawAABBWireframe(const glm::mat4 m2w, const AABB aabb);
	void DrawOBBWireframe(const OBB& obb, const glm::vec4& color);
	/* --- AABB & OBB Wireframe--- */
	
	/* --- AABB & OBB Wireframe--- */
	void DrawLine(const glm::vec3& a, const glm::vec3& b, const glm::vec4& color);
	void DrawCapsuleWireframe(const glm::vec3& top, const glm::vec3& bottom, float radius, const glm::vec4& color);


	/* --- Y=0 Plane --- */
	void InitPlane();

	void DrawPlane();
	/* --- Y=0 Plane --- */


	GLuint compile(GLenum type, const char* src);

	GLuint createProgram(const char* vs, const char* fs);
};