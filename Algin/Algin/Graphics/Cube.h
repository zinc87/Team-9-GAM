#pragma once
#include "../Header/pch.h"
#include "MaterialAsset.h"

class CubeRenderer;


struct CubeVertex
{
	glm::vec3 position;
	glm::vec2 texcoord;
	glm::vec3 normal;
    glm::vec3 tangent;

	CubeVertex(float x, float y, float z, float s, float t, glm::vec3 norm) : 
        position(x, y, z), texcoord(s, t), normal(norm),
        tangent{} {
    }
};




class CubeRenderer : public AG::Pattern::ISingleton<CubeRenderer>{
public:
	
    //MaterialParams mat;

	~CubeRenderer()
	{
		if (EBO) { glDeleteBuffers(1, &EBO); EBO = 0; }
		if (VBO) { glDeleteBuffers(1, &VBO); VBO = 0; }
		if (VAO) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
	}

	void Setup();
    void computeTangent(CubeVertex* vertices, size_t vertCount,
        const std::vector<GLuint>& indicies, size_t indexCount);
	void drawCube(const glm::mat4 m2w, const matAsset& params);

private:
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint materialUBO;
	
	const float s = 0.2f;

    std::array<CubeVertex, 24> verts = {
        // +Z (0..3)
        CubeVertex(-s,-s, s, 0,0, { 0, 0, 1}),
        CubeVertex(s,-s, s, 1,0, { 0, 0, 1}),
        CubeVertex(s, s, s, 1,1, { 0, 0, 1}),
        CubeVertex(-s, s, s, 0,1, { 0, 0, 1}),

        // -Z (4..7)
        CubeVertex(-s,-s,-s, 0,0, { 0, 0,-1}),
        CubeVertex(-s, s,-s, 0,1, { 0, 0,-1}),
        CubeVertex(s, s,-s, 1,1, { 0, 0,-1}),
        CubeVertex(s,-s,-s, 1,0, { 0, 0,-1}),

        // -X (8..11)
        CubeVertex(-s,-s,-s, 0,0, {-1, 0, 0}),
        CubeVertex(-s,-s, s, 1,0, {-1, 0, 0}),
        CubeVertex(-s, s, s, 1,1, {-1, 0, 0}),
        CubeVertex(-s, s,-s, 0,1, {-1, 0, 0}),

        // +X (12..15)
        CubeVertex(s,-s,-s, 0,0, { 1, 0, 0}),
        CubeVertex(s, s,-s, 0,1, { 1, 0, 0}),
        CubeVertex(s, s, s, 1,1, { 1, 0, 0}),
        CubeVertex(s,-s, s, 1,0, { 1, 0, 0}),

        // +Y (16..19)
        CubeVertex(-s, s,-s, 0,0, { 0, 1, 0}),
        CubeVertex(-s, s, s, 0,1, { 0, 1, 0}),
        CubeVertex(s, s, s, 1,1, { 0, 1, 0}),
        CubeVertex(s, s,-s, 1,0, { 0, 1, 0}),

        // -Y (20..23)
        CubeVertex(-s,-s,-s, 0,0, { 0,-1, 0}),
        CubeVertex(s,-s,-s, 1,0, { 0,-1, 0}),
        CubeVertex(s,-s, s, 1,1, { 0,-1, 0}),
        CubeVertex(-s,-s, s, 0,1, { 0,-1, 0}),
    };
};




