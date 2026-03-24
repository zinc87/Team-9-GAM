#include "pch.h"
#include "BatchRenderer.h"

const float quadVertices[4][2] = {
	{-0.5f,  0.5f},
	{ 0.5f,  0.5f},
	{ 0.5f, -0.5f},
	{-0.5f, -0.5f}
};

const float texCoords[4][2] = {
	{0.f, 1.f},
	{1.f, 1.f},
	{1.f, 0.f},
	{0.f, 0.f}
};

void BatchRenderer::Init()
{
	quadBuffer = new BatchVertex[MAX_VERTICE];
	quadBuffer_ite = quadBuffer;
	indiceCount = 0;

	sampler = new int[32];
	for (int i = 0; i < 32; i++)
		sampler[i] = i;

	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	glCreateBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTICE * sizeof(BatchVertex), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (const void*)offsetof(BatchVertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (const void*)offsetof(BatchVertex, color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (const void*)offsetof(BatchVertex, texcoord));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (const void*)offsetof(BatchVertex, texid));

	std::vector<GLushort> indices;
	for (GLushort i = 0; i < static_cast<GLushort>(MAX_VERTICE); i += 4)
	{
		indices.push_back(i);     // 0
		indices.push_back(i + 1); // 1
		indices.push_back(i + 2); // 2

		indices.push_back(i + 2); // 2
		indices.push_back(i + 3); // 3
		indices.push_back(i);     // 0
	}

	glCreateBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * (MAX_VERTICE / 4) * 6, indices.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void BatchRenderer::Terminate()
{
	delete[] quadBuffer;
	delete[] sampler;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
}

void BatchRenderer::BeginBatch()
{
	quadBuffer_ite = quadBuffer;
	indiceCount = 0;
	quadCount = 0;
}

void BatchRenderer::EndBatch()
{
	/* Sort by Z */
	if (quadCount > 0) {
		// a vector size of quad list
		std::vector<size_t> quadIndices(quadCount);
		for (size_t i = 0; i < quadCount; ++i) {
			quadIndices[i] = i;
		}

		// sort indices index by z
		std::sort(quadIndices.begin(), quadIndices.end(), [&](size_t a, size_t b) {
			return quadBuffer[a * 4].position[2] < quadBuffer[b * 4].position[2];
			});

		// temp buffer
		std::vector<BatchVertex> sortedVertices(quadCount * 4);

		// re org
		for (size_t i = 0; i < quadCount; ++i) {
			size_t quadIndex = quadIndices[i];
			for (size_t j = 0; j < 4; ++j) {
				sortedVertices[i * 4 + j] = quadBuffer[quadIndex * 4 + j]; // Copy all 4 vertices
			}
		}

		std::copy(sortedVertices.begin(), sortedVertices.end(), quadBuffer);
	}
}

void BatchRenderer::Render()
{ 
	ShaderManager::GetInstance().Use("Algin_Batch");

	BatchVertex* renderBuffer;
	BatchVertex* renderBuffer_ite;

	renderBuffer = quadBuffer;


	auto [width, height] = AGWINDOW.getWindowSize();

	float aspect = static_cast<float>(height) / static_cast<float>(width);
	glm::mat4 ProjectionMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(aspect, 1.0f, 1.0f));

	auto& asset_map = ASSET_MANAGER.GetAssets(DDS); // map of string to IAsset
	for (int i = 0; i < static_cast<int>(quadCount); i++)
	{
		GLuint texID = 0;
		renderBuffer_ite = quadBuffer + (i * 4);
		auto tex_weak = asset_map[renderBuffer_ite->tex_hash];
		auto tex_shr = tex_weak.lock();
		if (tex_shr)
		{
			auto tex_asset = std::dynamic_pointer_cast<TextureAsset>(tex_shr);
			if (tex_asset)
			{
				texID = (tex_asset ? tex_asset->textureID : 0);
			}
		}
		else
		{
			auto font_asset_map = ASSET_MANAGER.GetAssets(AGFONT);
			tex_weak = font_asset_map[renderBuffer_ite->tex_hash];
			tex_shr = tex_weak.lock();
			if (tex_shr)
			{
				auto tex_asset = std::dynamic_pointer_cast<FontAsset>(tex_shr);
				texID = (tex_asset ? tex_asset->m_textureID : 0);
			}
		}

		int texSlot = -1;
		if (texID == 0) {
			texSlot = -1;
		}
		else {
			// check if texture already assigned a slot
			auto it = textureHashMap.find(texID);
			if (it == textureHashMap.end()) {
				// assign new slot
				texSlot = static_cast<int>(textureHashMap.size());
				textureHashMap[texID] = float(texSlot);
				glBindTextureUnit(texSlot, texID);
			}
			else {
				texSlot = static_cast<int>(it->second);
			}
		}

		// assign slot to all 4 vertices of the quad
		for (int j = 0; j < 4; j++)
			(renderBuffer_ite + j)->texid = static_cast<float>(texSlot);


		renderCount++;

		if (textureHashMap.size() > 31 || i == static_cast<int>(quadCount - 1))
		{


			glViewport(0, 0, width, height);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, renderCount * 4 * sizeof(BatchVertex), renderBuffer);


			glBindVertexArray(VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

			GLuint P = glGetUniformLocation(SHADERMANAGER.GetShaderProgram(), "P");
			if (P == -1) std::cout << "P not found." << std::endl;
			//GLuint V = glGetUniformLocation(SHADERMANAGER.GetShaderProgram(), "V");
			//if (V == -1) std::cout << "V not found." << std::endl;
			GLuint T = glGetUniformLocation(SHADERMANAGER.GetShaderProgram(), "uTextures");
			if (T == -1) std::cout << "uTextures not found." << std::endl;

			glUniformMatrix4fv(P, 1, GL_FALSE, glm::value_ptr(ProjectionMatrix));
			//glUniformMatrix4fv(V, 1, GL_FALSE, glm::value_ptr(CAMERAMANAGER.getSceneCamera()->GetViewMatrix()));
			glUniform1iv(T, 32, sampler);

			glDrawElements(GL_TRIANGLES, renderCount * 6, GL_UNSIGNED_SHORT, (const void*)0);

			renderCount = 0;
			textureHashMap.clear();
			renderBuffer = renderBuffer_ite + 4;
		}
	}

}

void BatchRenderer::Flush()
{
	indiceCount = 0;
	quadCount = 0;
	textureSlotAssign = 0.f;
	textureSlotCounter = 0;
	textureHashMap.clear();
}

void BatchRenderer::DrawQuad(Color clr, Transform2D& trf, size_t tex_hash_id, TextureCoordinate texCoord)
{
	if (quadBuffer_ite - quadBuffer >= MAX_VERTICE || textureHashMap.size() >= 32)
	{
		EndBatch();
		Render();
		Flush();
		BeginBatch();
	}

	float radians = trf.rotation * (static_cast<float>(3.1415) / 180.0f);
	float cosTheta = cos(radians);
	float sinTheta = sin(radians);

	for (int i = 0; i < 4; i++)
	{
		// S
		float scaledX = quadVertices[i][0] * trf.scale.x;
		float scaledY = quadVertices[i][1] * trf.scale.y;

		// R
		float rotatedX = scaledX * cosTheta - scaledY * sinTheta;
		float rotatedY = scaledX * sinTheta + scaledY * cosTheta;

		// T
		float transformedX = rotatedX + trf.position.x;
		float transformedY = rotatedY + trf.position.y;

		quadBuffer_ite->position[0] = transformedX;
		quadBuffer_ite->position[1] = transformedY;
		quadBuffer_ite->position[2] = trf.position[2];

		quadBuffer_ite->color[0] = clr.rgba[0];
		quadBuffer_ite->color[1] = clr.rgba[1];
		quadBuffer_ite->color[2] = clr.rgba[2];
		quadBuffer_ite->color[3] = clr.rgba[3];

		quadBuffer_ite->texcoord[0] = texCoord.textureCoord[i][0];
		quadBuffer_ite->texcoord[1] = texCoord.textureCoord[i][1];

		quadBuffer_ite->texid = -1.f;
		quadBuffer_ite->tex_hash = tex_hash_id;
		quadBuffer_ite++;
	}

	indiceCount += 6;
	quadCount++;

	//AG_CORE_WARN("Add Quad!!");
}

void BatchRenderer::RenderText(std::string content, Color clr, Transform2D& trf,
	size_t font_hash_id, bool wrapped, float wrap_limit,
	bool left_aligned, bool centered, bool right_aligned,
	float font_size, float line_spacing, float letter_spacing)
{
	// --- 1. Get Asset ---
	std::shared_ptr<IAsset> asset_shared = AssetManager::GetInstance().GetAsset(font_hash_id).lock();
	if (!asset_shared)
	{
		static bool logged = false;
		if (!logged) {
			AG_CORE_WARN("[RenderText] Font asset NOT FOUND for hash={}", font_hash_id);
			logged = true;
		}
		return;
	}

	std::shared_ptr<FontAsset> font_shared = std::dynamic_pointer_cast<FontAsset>(asset_shared);
	if (!font_shared) return;

	// --- 2. Setup Metrics ---
	float scale = (font_size / 128.0f);
	float lineHeight = line_spacing * scale;
	float extraLetterSpacing = letter_spacing * scale;
	float textRotation = trf.rotation;

	auto GetEffectiveAdvance = [&](const Glyphs& g) {
		float adv = g.advanceX > 0.0f ? g.advanceX : (g.width + 1.0f);
		return adv * scale;
		};

	// --- 3. Word Splitting & Layout Calculation ---
	std::vector<std::string> words;
	{
		std::stringstream ss(content);
		std::string word;
		while (ss >> word) words.push_back(word);
	}

	std::vector<std::pair<std::string, float>> layoutLines;
	std::string currentLine;
	float currentLineWidth = 0.0f;

	for (const std::string& currentWord : words)
	{
		float wordWidth = 0.0f;
		for (char c : currentWord)
		{
			auto it = font_shared->m_glyphs_data.find(c);
			if (it != font_shared->m_glyphs_data.end())
			{
				wordWidth += GetEffectiveAdvance(it->second);
				wordWidth += extraLetterSpacing;
			}
		}

		float spaceWidth = 0.0f;
		if (font_shared->m_glyphs_data.count(' '))
			spaceWidth = GetEffectiveAdvance(font_shared->m_glyphs_data.at(' '));

		bool shouldWrap = (wrapped && wrap_limit > 0.0f) &&
			(currentLineWidth + wordWidth + (currentLineWidth > 0 ? spaceWidth : 0.0f) > wrap_limit);

		if (shouldWrap)
		{
			layoutLines.push_back({ currentLine, currentLineWidth });
			currentLine.clear();
			currentLineWidth = 0.0f;
		}

		if (!currentLine.empty())
		{
			currentLine += " ";
			currentLineWidth += spaceWidth;
		}
		currentLine += currentWord;
		currentLineWidth += wordWidth;
	}

	if (!currentLine.empty())
		layoutLines.push_back({ currentLine, currentLineWidth });

	// --- 4. Rendering Loop ---
	float cursorY = 0.0f;

	for (const auto& lineData : layoutLines)
	{
		const std::string& line = lineData.first;
		float lineWidth = lineData.second;

		// --- Alignment Logic ---
		float cursorX = 0.0f;
		if (right_aligned)
		{
			cursorX = -lineWidth;
		}
		else if (centered)
		{
			cursorX = -lineWidth * 0.5f;
		}
		else // default or left_aligned
		{
			cursorX = 0.0f;
		}

		float rad = glm::radians(textRotation);
		float cos_theta = glm::cos(rad);
		float sin_theta = glm::sin(rad);

		for (char c : line)
		{
			auto it = font_shared->m_glyphs_data.find(c);
			if (it == font_shared->m_glyphs_data.end()) continue;

			const Glyphs& g = it->second;

			float w = g.width * scale;
			float h = g.height * scale;

			float x_topLeft = cursorX + g.bearingX * scale;
			float y_topLeft = -cursorY + g.bearingY * scale;

			float x_center = x_topLeft + w * 0.5f;
			float y_center = y_topLeft - h * 0.5f;

			glm::vec2 rotated_pos;
			if (textRotation != 0.0f)
			{
				rotated_pos.x = x_center * cos_theta - y_center * sin_theta;
				rotated_pos.y = x_center * sin_theta + y_center * cos_theta;
			}
			else
			{
				rotated_pos.x = x_center;
				rotated_pos.y = y_center;
			}

			Transform2D charTrf = trf;
			charTrf.position.x += rotated_pos.x;
			charTrf.position.y += rotated_pos.y;
			charTrf.rotation = textRotation;
			charTrf.scale.x = w;
			charTrf.scale.y = h;

			TextureCoordinate tex;
			for (int i = 0; i < 4; ++i)
			{
				tex.textureCoord[i][0] = g.texCoord[i][0];
				tex.textureCoord[i][1] = g.texCoord[i][1];
			}

			DrawQuad(clr, charTrf, font_hash_id, tex);

			cursorX += GetEffectiveAdvance(g);
			cursorX += extraLetterSpacing;
		}

		cursorY += lineHeight;
	}
}