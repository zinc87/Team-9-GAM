#include "pch.h"
#include "DebugRenderer.h"

// PASTE THE FUNCTIONS HERE
namespace {
	static const char* vertexSrc() {
		return R"(#version 450 core
	layout(location=0) in vec3 aPos;
	layout(location=1) in vec2 aTexCoord;

	uniform mat4 P;
	uniform mat4 V;
	uniform mat4 M;
	uniform vec2 vTexOffset;
	out vec2 vTexCoord;

	void main() {
		vTexCoord = aTexCoord + vTexOffset;
		gl_Position = P * V * M * vec4(aPos, 1.0);
	})";
	}

	static const char* fragmentSrc() {
		return R"(#version 450 core
	out vec4 FragColor;

	in vec2 vTexCoord;

	uniform vec4 uColor;
	uniform bool uHasTexture;
	uniform sampler2D uTexture;

	void main() { 
		if (uHasTexture) {
			FragColor = texture(uTexture, vTexCoord);
		} else {
			FragColor = uColor; 
		}
	})";
	}
} // end anonymous namespace


DebugRenderer::DebugRenderer()
{
	Setup();
	InitPlane();
}

void DebugRenderer::Setup()
{
	program = createProgram(vertexSrc(), fragmentSrc());

	// Generate VAO/VBO/EBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// Bind VBO first!
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex) * 8, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex,texcoord));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	locP = glGetUniformLocation(program, "P");
	locV = glGetUniformLocation(program, "V");
	locM = glGetUniformLocation(program, "M");
	locColor = glGetUniformLocation(program, "uColor");
	locTexture = glGetUniformLocation(program, "uTexture");
	locTextureOffset = glGetUniformLocation(program, "vTexOffset");
	locHasTexture = glGetUniformLocation(program, "uHasTexture");
}

void DebugRenderer::UpdateCorner(std::array<DebugVertex, 8> vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugRenderer::DrawWireframe(const glm::mat4 P, const glm::mat4 V)
{
	auto camera = CameraManager::GetInstance().getCurrentCamera().lock();
	if (!camera)
		return;

	UpdateCorner(frustumCorner);

	// 12 edges (24 indices). Static topology.
	static const GLushort edges[24] = {
		// near plane rectangle
		0,1, 1,2, 2,3, 3,0,
		// far plane rectangle
		4,5, 5,6, 6,7, 7,4,
		// connect near/far
		0,4, 1,5, 2,6, 3,7
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(edges), edges, GL_DYNAMIC_DRAW);

	glUseProgram(program);
	if (locP != -1) glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
	if (locV != -1) glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	if (locM != -1) glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(glm::inverse(P * V)));
	if (locColor != -1) glUniform4f(locColor, 1.f, 1.f, 1.f, 1.f);
	if (locHasTexture != -1) glUniform1i(locHasTexture, false);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, (void*)0);
	glBindVertexArray(0);

	glUseProgram(0);
}

void DebugRenderer::UpdateCorner(const glm::mat4 m2w, const AABB aabb)
{
	// 8 corners of the AABB in local/object space
	glm::vec3 min = aabb.min;
	glm::vec3 max = aabb.max;

	glm::vec3 corners[8] = {
		{min.x, min.y, min.z},
		{max.x, min.y, min.z},
		{max.x, max.y, min.z},
		{min.x, max.y, min.z},
		{min.x, min.y, max.z},
		{max.x, min.y, max.z},
		{max.x, max.y, max.z},
		{min.x, max.y, max.z}
	};

	// Transform into world space
	for (int i = 0; i < 8; ++i)
	{
		glm::vec4 world = m2w * glm::vec4(corners[i], 1.0f);
		aabbCorner[i].position = glm::vec3(world);
	}

	// Upload to VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(aabbCorner), aabbCorner.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugRenderer::UpdateCorner(const OBB obb)
{
	glm::vec3 axisX = obb.rotation[0];
	glm::vec3 axisY = obb.rotation[1];
	glm::vec3 axisZ = obb.rotation[2];

	glm::vec3 he = obb.halfExtents;

	glm::vec3 corners[8] = {
		obb.center + (-he.x * axisX) + (-he.y * axisY) + (-he.z * axisZ),
		obb.center + (he.x * axisX) + (-he.y * axisY) + (-he.z * axisZ),
		obb.center + (he.x * axisX) + (he.y * axisY) + (-he.z * axisZ),
		obb.center + (-he.x * axisX) + (he.y * axisY) + (-he.z * axisZ),
		obb.center + (-he.x * axisX) + (-he.y * axisY) + (he.z * axisZ),
		obb.center + (he.x * axisX) + (-he.y * axisY) + (he.z * axisZ),
		obb.center + (he.x * axisX) + (he.y * axisY) + (he.z * axisZ),
		obb.center + (-he.x * axisX) + (he.y * axisY) + (he.z * axisZ)
	};

	// Copy into your debug vertex array
	for (int i = 0; i < 8; ++i)
		aabbCorner[i].position = corners[i];  // reuse aabbCorner for OBB too

	// Upload to GPU
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(aabbCorner), aabbCorner.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugRenderer::DrawAABBWireframe(const glm::mat4 m2w, const AABB aabb)
{
	auto camera = CameraManager::GetInstance().getSceneCamera();
	if (!camera)
		return;

	// Update VBO with transformed AABB corners
	UpdateCorner(m2w, aabb);

	// Line indices for the 12 edges of a box (static, upload once ideally)
	static const unsigned short indices[24] = {
		0,1, 1,2, 2,3, 3,0, // bottom face
		4,5, 5,6, 6,7, 7,4, // top face
		0,4, 1,5, 2,6, 3,7  // verticals
	};

	glBindVertexArray(VAO);

	// EBO setup (can also be initialized once in constructor/setup code)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Bind shader
	glUseProgram(program);

	if (locP != -1) glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
	if (locV != -1) glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	if (locM != -1) glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
	if (locColor != -1) glUniform4f(locColor, 1.f, 1.f, 0.f, 1.f); // yellow
	if (locHasTexture != -1) glUniform1i(locHasTexture, false);

	// Draw wireframe AABB
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, (void*)0);

	glBindVertexArray(0);
	glUseProgram(0);
}

void DebugRenderer::DrawOBBWireframe(const OBB& obb, const glm::vec4& color)
{
	auto cam = CameraManager::GetInstance().getCurrentCamera().lock();
	if (!cam) return;

	UpdateCorner(obb); // WORLD SPACE corners

	glUseProgram(program);

	glUniformMatrix4fv(locP, 1, GL_FALSE,
		glm::value_ptr(cam->GetProjectionMatrix()));

	glUniformMatrix4fv(locV, 1, GL_FALSE,
		glm::value_ptr(cam->GetViewMatrix()));

	// IMPORTANT: Model matrix must be IDENTITY
	glUniformMatrix4fv(locM, 1, GL_FALSE,
		glm::value_ptr(glm::mat4(1.0f)));

	glUniform4fv(locColor, 1, glm::value_ptr(color));
	glUniform1i(locHasTexture, false);

	glBindVertexArray(VAO);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);

	glUseProgram(0);
}



void DebugRenderer::DrawLine(const glm::vec3& a,
	const glm::vec3& b,
	const glm::vec4& color)
{
	// Convert line to OBB with tiny thickness
	glm::vec3 mid = (a + b) * 0.5f;
	float length = glm::length(b - a);

	// Create a 0.01-thick line box
	glm::vec3 half = { 0.01f, length * 0.5f, 0.01f };

	// Compute rotation
	glm::vec3 dir = glm::normalize(b - a);
	glm::vec3 up = { 0, 1, 0 };
	glm::vec3 right = glm::normalize(glm::cross(up, dir));
	glm::vec3 forward = glm::cross(dir, right);

	glm::mat3 rot(right, dir, forward);

	OBB obb;
	obb.center = mid;
	obb.halfExtents = half;
	obb.rotation = rot;

	DrawOBBWireframe(obb, color);
}

void DebugRenderer::DrawCapsuleWireframe(const glm::vec3& top,
	const glm::vec3& bottom,
	float radius,
	const glm::vec4& color)
{
	const int segments = 24;

	glm::vec3 axis = glm::normalize(top - bottom);

	// Build orthonormal basis around axis
	glm::vec3 temp = (fabs(axis.y) < 0.99f) ?
		glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

	glm::vec3 right = glm::normalize(glm::cross(axis, temp));
	glm::vec3 forward = glm::cross(right, axis);

	// --- Top circle ---
	for (int i = 0; i < segments; i++)
	{
		float a0 = (i)*glm::two_pi<float>() / segments;
		float a1 = (i + 1.0f) * glm::two_pi<float>() / segments;

		glm::vec3 p0 = top +
			right * (cos(a0) * radius) +
			forward * (sin(a0) * radius);

		glm::vec3 p1 = top +
			right * (cos(a1) * radius) +
			forward * (sin(a1) * radius);

		DrawLine(p0, p1, color);
	}

	// --- Bottom circle ---
	for (int i = 0; i < segments; i++)
	{
		float a0 = (i)*glm::two_pi<float>() / segments;
		float a1 = (i + 1.0f) * glm::two_pi<float>() / segments;

		glm::vec3 p0 = bottom +
			right * (cos(a0) * radius) +
			forward * (sin(a0) * radius);

		glm::vec3 p1 = bottom +
			right * (cos(a1) * radius) +
			forward * (sin(a1) * radius);

		DrawLine(p0, p1, color);
	}

	// --- Vertical edges ---
	DrawLine(top + right * radius, bottom + right * radius, color);
	DrawLine(top - right * radius, bottom - right * radius, color);
	DrawLine(top + forward * radius, bottom + forward * radius, color);
	DrawLine(top - forward * radius, bottom - forward * radius, color);
}

void DebugRenderer::DrawCube(const glm::mat4 m2w)
{
	auto camera = CameraManager::GetInstance().getSceneCamera();
	if (!camera)
		return;

	// Update cube corners
	UpdateCorner(cubeCorner);

	// Indices for solid cube (12 triangles = 36 indices)
	static const unsigned short cube_indices[36] = {
		// front face
		0, 1, 2, 2, 3, 0,
		// back face
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

	glBindVertexArray(VAO);

	// Upload triangle indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

	glUseProgram(program);

	if (locP != -1) glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
	if (locV != -1) glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	if (locM != -1) glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(m2w));
	if (locColor != -1) glUniform4f(locColor, 1.f, 1.f, 1.f, 1.f);
	if (locHasTexture != -1) glUniform1i(locHasTexture, false);

	// Draw solid cube
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

	glBindVertexArray(0);
	glUseProgram(0);
}


void DebugRenderer::DrawCubeColor(const glm::mat4 m2w, AG::Data::GUID objID)
{
	auto camera = CameraManager::GetInstance().getSceneCamera();
	if (!camera)
		return;

	// Update cube corners
	UpdateCorner(cubeCorner);

	// Indices for solid cube (12 triangles = 36 indices)
	static const unsigned short cube_indices[36] = {
		// front face
		0, 1, 2, 2, 3, 0,
		// back face
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

	glBindVertexArray(VAO);

	// Upload triangle indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

	SHADERMANAGER.Use("ObjPicking");
	GLuint prog = SHADERMANAGER.GetShaderProgram();
	

	GLint pickingLocP = glGetUniformLocation(prog, "P");
	GLint pickingLocV = glGetUniformLocation(prog, "V");
	GLint pickingLocM = glGetUniformLocation(prog, "M");

	if (pickingLocP != -1)
		glUniformMatrix4fv(pickingLocP, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
	if (pickingLocV != -1)
		glUniformMatrix4fv(pickingLocV, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	if (pickingLocM != -1)
		glUniformMatrix4fv(pickingLocM, 1, GL_FALSE, glm::value_ptr(m2w));

	//uint64_t id64 = fnv1a64(objID.c_str(), objID.size());
	uint64_t id64 = std::hash<std::string>{}(objID);
	objectPicking::GetInstance().id64_to_guid[id64] = objID;
	GLuint low = static_cast<GLuint>(id64 & 0xFFFFFFFFull);
	GLuint high = static_cast<GLuint>((id64 >> 32) & 0xFFFFFFFFull);
	
	if (GLint loc = glGetUniformLocation(prog, "uId64"); loc != -1) {
		glUniform2ui(loc, low, high);
	}

	// Draw solid cube
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

	glBindVertexArray(0);
	//glUseProgram(0);
}

void DebugRenderer::InitPlane()
{
	if (planeVAO) return;

	std::array<unsigned int, 6> indices = { 0, 1, 2, 2, 3, 0 };

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glGenBuffers(1, &planeEBO);

	glBindVertexArray(planeVAO);

	// Upload vertex data
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER,
		planeCorner.size() * sizeof(DebugVertex),
		planeCorner.data(),
		GL_STATIC_DRAW);

	// Upload index data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		indices.size() * sizeof(unsigned int),
		indices.data(),
		GL_STATIC_DRAW);

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex),
		(void*)offsetof(DebugVertex, position));

	// Texcoord attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(DebugVertex),
		(void*)offsetof(DebugVertex, texcoord));

	// Only unbind VAO (keeps EBO binding inside VAO)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void DebugRenderer::DrawPlane()
{
	if (!planeVAO) InitPlane();

	glViewport(0, 0, AGWINDOW.getWindowSize().first, AGWINDOW.getWindowSize().second);


	auto camera = CAMERAMANAGER.getCurrentCamera().lock();
	if (!camera) return;

	AG::Benchmarker_StartMarker("Draw Plane");
	const glm::mat4& P = camera->GetProjectionMatrix();
	const glm::mat4& V = camera->GetViewMatrix();
	const glm::vec3& camPos = camera->getCameraPosition();
	glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(camPos.x, 0.0f, camPos.z));

	glm::vec2 texOffset = glm::vec2(-camPos.x, camPos.z) * 0.5f;


	glUseProgram(program);

	glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(P));
	glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(V));
	glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(M));
	glUniform4f(locColor, 0.3f, 0.3f, 0.3f, 1.0f);
	glUniform1i(locHasTexture, true);
	glUniform2fv(locTextureOffset, 1, glm::value_ptr(texOffset));

	glActiveTexture(GL_TEXTURE0);
	// Get the ID as its native type first
	ImTextureID imgui_id = AG::AGImGui::ContentBrowser::GetInstance().GetIcon("floor grid");

	// Safely cast the pointer to an integer of the same size, then to GLuint
	GLuint tex_id = (GLuint)(uintptr_t)imgui_id;
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glUniform1i(locTexture, 0);   // 0 = GL_TEXTURE0

	glBindVertexArray(planeVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	AG::Benchmarker_EndMarker("Draw Plane");
}


GLuint DebugRenderer::compile(GLenum type, const char* src)
{
	GLuint s = glCreateShader(type);
	glShaderSource(s, 1, &src, nullptr);
	glCompileShader(s);
	GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[2048]; GLsizei n = 0; glGetShaderInfoLog(s, 2048, &n, log);
		glDeleteShader(s);
		throw std::runtime_error(std::string("Shader compile failed: ") + log);
	}
	return s;
}

GLuint DebugRenderer::createProgram(const char* vs, const char* fs)
{
	GLuint v = compile(GL_VERTEX_SHADER, vs);
	GLuint f = compile(GL_FRAGMENT_SHADER, fs);
	GLuint p = glCreateProgram();
	glAttachShader(p, v);
	glAttachShader(p, f);
	glLinkProgram(p);
	glDeleteShader(v);
	glDeleteShader(f);
	GLint ok = 0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[2048]; GLsizei n = 0; glGetProgramInfoLog(p, 2048, &n, log);
		glDeleteProgram(p);
		throw std::runtime_error(std::string("Program link failed: ") + log);
	}
	return p;
}
