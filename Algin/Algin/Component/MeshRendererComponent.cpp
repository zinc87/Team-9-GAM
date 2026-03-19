#include "pch.h"
#include "MeshRendererComponent.h"


#include <cstdint>

void AG::Component::MeshRendererComponent::EnsureMaterialDataValid()
{
	if (mesh_name.empty()) return;

	auto asset = AssetManager::GetInstance().GetAsset(model_hash).lock();
	auto mesh = std::dynamic_pointer_cast<StaticMeshAsset>(asset);
	if (!mesh) return;

	const size_t requiredCount = mesh->submeshes.size();

	if (material_hash_list.size() != requiredCount)
		material_hash_list.assign(requiredCount, 0);

	if (material_value_list.size() != requiredCount)
		material_value_list.assign(requiredCount, glm::vec3(0.1f));

	if (submesh_name_hashes.size() != mesh->submeshes.size())
	{
		submesh_name_hashes.clear();
		submesh_name_hashes.reserve(mesh->submeshes.size());
		for (auto& s : mesh->submeshes)
			submesh_name_hashes.push_back(HASH(s.name)); // or any hashing utility you already use
	}

}

void AG::Component::MeshRendererComponent::Awake()
{

}

void AG::Component::MeshRendererComponent::Start()
{
	if (!mesh_name.empty())
		model_hash = AssetManager::GetInstance().hasher(mesh_name);
	EnsureMaterialDataValid();
}

void AG::Component::MeshRendererComponent::Update()
{
	//if (Self::GetMeshItems().size() <= mesh_index) return;
	//e_MeshName = Self::GetMeshItems()[mesh_index];

}

void AG::Component::MeshRendererComponent::LateUpdate() 
{
	auto obj = GetObj().lock();
	if (!obj) return;

	auto trf = obj->GetComponent<Component::TransformComponent>().lock();
	if (!trf) return;

	auto asset = AssetManager::GetInstance().GetAsset(model_hash);
	auto assetLock = asset.lock();
	if (!assetLock) return;

	//AG_CORE_WARN("asset exist!");
	auto meshAsset = std::dynamic_pointer_cast<StaticMeshAsset>(assetLock);
	if (!meshAsset) return;

	auto state = RenderPipeline::GetInstance().GetPipeline();

	if (state == RenderPipeline::SHADOW) {
		//drawShadows
		DrawShadows(*meshAsset, *trf);
	}
	else if (state == RenderPipeline::LIGHT) {
		//drawLight
		if (trf)
			DrawLight(*meshAsset, *trf);
	}
	else if (state == RenderPipeline::OBJPICK) {
		DrawObjsColor(*meshAsset, *trf);
	}
}

void AG::Component::MeshRendererComponent::Free()
{
}

void AG::Component::MeshRendererComponent::Inspector()
{
	auto obj = GetObj().lock();
	if (!obj) return;

	auto trf = obj->GetComponent<Component::TransformComponent>().lock();
	if (!trf) return;

	// ==========================================================
	// Mesh selection (SEARCHABLE)
	// ==========================================================
	auto& assetMgr = AssetManager::GetInstance();
	auto meshAssets = assetMgr.GetAssets(ASSET_TYPE::AGSTATICMESH);

	// Determine the current preview name
	std::string currentMeshName = mesh_name.empty() ? "No Mesh" : mesh_name;

	// Static filter to persist search text while the menu is open/interacted with
	static ImGuiTextFilter filter;

	if (ImGui::BeginCombo("Mesh", currentMeshName.c_str()))
	{
		filter.Draw("##MeshSearch", -1.0f);
		ImGui::Separator();

		if (filter.PassFilter("No Mesh"))
		{
			bool isSelected = mesh_name.empty();
			if (ImGui::Selectable("No Mesh", isSelected))
			{
				mesh_name.clear();
				model_hash = 0;
				material_hash_list.clear();
				material_value_list.clear();
				submesh_name_hashes.clear();
				EnsureMaterialDataValid();
			}
			if (isSelected) ImGui::SetItemDefaultFocus();
		}

		for (auto& [hash, asset] : meshAssets)
		{
			if (auto lock = asset.lock())
			{
				const std::string& name = lock->asset_name;
				if (filter.PassFilter(name.c_str()))
				{
					bool isSelected = (mesh_name == name);
					if (ImGui::Selectable(name.c_str(), isSelected))
					{
						mesh_name = name;
						model_hash = assetMgr.hasher(mesh_name);
						AG_CORE_INFO("[DEBUG] Set mesh '{}' with hash: {}", mesh_name, model_hash);
						material_hash_list.clear();

						auto meshPtr = std::dynamic_pointer_cast<StaticMeshAsset>(lock);
						if (meshPtr)
							material_hash_list.assign(meshPtr->submeshes.size(), 0);

						EnsureMaterialDataValid();
					}
					if (isSelected) ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}

	auto meshAsset = assetMgr.GetAsset(model_hash).lock();
	auto mesh = std::dynamic_pointer_cast<StaticMeshAsset>(meshAsset);
	if (!mesh) return;

	if (!ImGui::TreeNode("SubMeshes")) return;

	// Gather material names
	std::vector<const char*> materialNames;
	materialNames.push_back("Default/None"); // Use a clear name for index 0
	auto mAssets = assetMgr.GetAssets(ASSET_TYPE::AGMATERIAL);

	for (auto& [name, asset] : mAssets)
		if (auto a = asset.lock()) materialNames.push_back(a->asset_name.c_str());

	size_t subCount = mesh->submeshes.size();

	// Shared filter for material searches (clears typing per unique combo open)
	static ImGuiTextFilter matFilter;

	for (size_t i = 0; i < subCount; ++i)
	{
		SubMesh& sub = mesh->submeshes[i];
		std::string label = sub.name.empty() ? ("Submesh " + std::to_string(i)) : sub.name;

		ImGui::PushID((int)i);

		if (ImGui::TreeNode((void*)(intptr_t)i, "%s", label.c_str()))
		{
			// 1. Find Current Material Index
			int currentIdx = 0;
			for (int n = 0; n < materialNames.size(); ++n)
			{
				// Note: materialNames[0] maps to HASH("Default/None") if you hash it, 
				// but your logic usually treats 0 as 'no asset'. 
				// Adjust comparison if your "None" string creates a hash mismatch.
				// Assuming HASH of empty string or "Default/None" is 0 or handled.
				if (n == 0 && material_hash_list[i] == 0) { currentIdx = 0; break; }

				if (material_hash_list[i] == HASH(materialNames[n]))
				{
					currentIdx = n;
					break;
				}
			}

			// 2. Determine Preview Text
			const char* previewValue = (currentIdx >= 0 && currentIdx < materialNames.size())
				? materialNames[currentIdx] : "None";

			// 3. Searchable Dropdown
			std::string comboId = "Materials##" + std::to_string(i);
			if (ImGui::BeginCombo(comboId.c_str(), previewValue))
			{
				// Focus search bar on open
				if (ImGui::IsWindowAppearing())
					ImGui::SetKeyboardFocusHere();

				matFilter.Draw("##matSearch", -1.0f);
				ImGui::Separator();

				for (int n = 0; n < materialNames.size(); ++n)
				{
					if (matFilter.PassFilter(materialNames[n]))
					{
						const bool isSelected = (currentIdx == n);
						if (ImGui::Selectable(materialNames[n], isSelected))
						{
							if (n == 0) material_hash_list[i] = 0; // Handle None/Default
							else material_hash_list[i] = HASH(materialNames[n]);
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			// Drag Drop Support
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IMGUI_CONTENT"))
				{
					material_hash_list[i] = *(const size_t*)payload->Data;
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::TreePop();
	EnsureMaterialDataValid();
}

void AG::Component::MeshRendererComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
	ASSIGNFROM_FN_BODY(
		this->mesh_name = fromCmpPtr->mesh_name;
	this->model_hash = fromCmpPtr->model_hash;

	// Resize target containers before copying
	this->submesh_name_hashes.resize(fromCmpPtr->submesh_name_hashes.size());
	this->material_hash_list.resize(fromCmpPtr->material_hash_list.size());
	this->material_value_list.resize(fromCmpPtr->material_value_list.size());

	// Now copy
	std::copy(fromCmpPtr->submesh_name_hashes.begin(), fromCmpPtr->submesh_name_hashes.end(), this->submesh_name_hashes.begin());
	std::copy(fromCmpPtr->material_hash_list.begin(), fromCmpPtr->material_hash_list.end(), this->material_hash_list.begin());
	std::copy(fromCmpPtr->material_value_list.begin(), fromCmpPtr->material_value_list.end(), this->material_value_list.begin());

	this->shininess = fromCmpPtr->shininess;
		)
}


void AG::Component::MeshRendererComponent::DrawShadows(const StaticMeshAsset& mesh, TransformComponent& trf) {

	// SHADOW_PASS is already bound by BeginPass()
	SHADERMANAGER.Use("SHADOW_PASS");
	GLuint prog = SHADERMANAGER.GetShaderProgram(); // the currently bound program
	
	if (GLint locM = glGetUniformLocation(prog, "uModel"); locM != -1) {
		glm::mat4 model = trf.getM2W();
		glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(model));
	}

	// Draw all submeshes
	for (const auto& sub : mesh.submeshes) {
		glBindVertexArray(sub.VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sub.IBO);
		if (!sub.indices.empty())
			glDrawElements(GL_TRIANGLES, (GLsizei)sub.indices.size(), GL_UNSIGNED_INT, (void*)0);
		else
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sub.vertices.size());
	}
	glBindVertexArray(0);
}

void AG::Component::MeshRendererComponent::DrawLight(const StaticMeshAsset& mesh, TransformComponent& trf) {
	//glDisable(GL_CULL_FACE);
	auto camera = CAMERAMANAGER.getCurrentCamera().lock();
	if (!camera) return;


	if (!model_hash) {
		//AG_CORE_WARN("Mesh Asset does not exist : {}", e_MeshName);
		return;
	}

	size_t subIdx = 0;
	for (auto& submesh : mesh.submeshes) {

		//*===Material===*// -> lives in for loop for submesh
		auto mat_ass_wk = AssetManager::GetInstance().GetAsset(material_hash_list[subIdx]); // <- change 0 to n-submesh
		auto mat_ass_sp = mat_ass_wk.lock();
		auto material_sp = std::dynamic_pointer_cast<MaterialTemplate>(mat_ass_sp);

		// Frustum (your counters kept as-is)
		//BENCHMARKER.FrustumCheckInc();
		//if (!Camera::IsObjectInCamera(trf.getM2W(), submesh.minmax)) {
		//	BENCHMARKER.FrustumCulledInc();
		//	// (Optionally continue here if you really want to skip drawing)
		//	// continue;
		//}

		std::string shader_name;
		if (material_sp)	shader_name = material_sp->GetShaderName();
		else				shader_name = "Algin_3D";

		//AG_CORE_INFO("{}", shader_name);
		SHADERMANAGER.Use(shader_name);
		GLuint shader = ShaderManager::GetInstance().GetShaderProgram();


		if (!model_hash) {
			//AG_CORE_WARN("Mesh Asset does not exist : {}", e_MeshName);
			return;
		}

		// Camera transforms
		GLint locP = glGetUniformLocation(shader, "P");
		GLint locV = glGetUniformLocation(shader, "V");
		GLint locM = glGetUniformLocation(shader, "M");
		GLint locN = glGetUniformLocation(shader, "uNormalMat");

		if (locP != -1)
			glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
		if (locV != -1)
			glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

		// Camera position for specular
		if (GLint locViewPos = glGetUniformLocation(shader, "camPos"); locViewPos != -1) {
			glm::vec3 camPos = camera->getCameraPosition();
			glUniform3fv(locViewPos, 1, glm::value_ptr(camPos));
		}

		// Lights
		const auto lights = LightManager::GetInstance().getLightList();
		constexpr int MAX_LIGHTS = 16;
		int count = std::min<int>((int)lights.size(), MAX_LIGHTS);

		if (GLint locCount = glGetUniformLocation(shader, "uLightCount"); locCount != -1)
			glUniform1i(locCount, count);

		for (int i = 0; i < count; ++i) {
			const Light& L = lights[i];
			const std::string base = "uLights[" + std::to_string(i) + "].";

			if (GLint loc = glGetUniformLocation(shader, (base + "type").c_str()); loc != -1)
				glUniform1i(loc, (int)L.type);
			if (GLint loc = glGetUniformLocation(shader, (base + "position").c_str()); loc != -1)
				glUniform3fv(loc, 1, glm::value_ptr(L.position));
			if (GLint loc = glGetUniformLocation(shader, (base + "direction").c_str()); loc != -1)
				glUniform3fv(loc, 1, glm::value_ptr(glm::normalize(L.direction)));
			if (GLint loc = glGetUniformLocation(shader, (base + "range").c_str()); loc != -1)
				glUniform1f(loc, L.range);
			if (GLint loc = glGetUniformLocation(shader, (base + "intensity").c_str()); loc != -1)
				glUniform1f(loc, L.intensity);
			if (GLint loc = glGetUniformLocation(shader, (base + "outerCutoff").c_str()); loc != -1)
				glUniform1f(loc, L.outerCutoffCos);
			if (GLint loc = glGetUniformLocation(shader, (base + "innerCutoff").c_str()); loc != -1)
				glUniform1f(loc, L.innerCutoffCos);
		}

		const size_t base = subIdx * 3; // 0: albedo, 1: normal, 2: specular
		(void)base; // i dont think is used -brandon
		GLint loc_hasAlbedo = glGetUniformLocation(shader, "uMaterial.hasDiffuseTex");
		size_t albedoHash = 0;
		if(material_sp){
			albedoHash = material_sp->GetAlbedoHash();
		}
		auto weakTexAss = AssetManager::GetInstance().GetAsset(albedoHash);
		std::shared_ptr<TextureAsset> shrd_tex_ass;
		if (weakTexAss.lock()) {
			shrd_tex_ass = std::dynamic_pointer_cast<TextureAsset>(weakTexAss.lock());
		}
		if (shrd_tex_ass) {
			GLint loc_uAlbedoMap = glGetUniformLocation(shader, "uDiffuseTex");
			glBindTextureUnit(0, shrd_tex_ass->textureID);
			glProgramUniform1i(shader, loc_uAlbedoMap, 0);
			glProgramUniform1i(shader, loc_hasAlbedo, 1);
		}
		else {
			glProgramUniform1i(shader, loc_hasAlbedo, 0);

		}

		//normal map
		size_t normalHash = 0;
		if (material_sp) {
			normalHash = material_sp->GetNormalHash();
		}
		auto weakNormalTexAss = AssetManager::GetInstance().GetAsset(normalHash);
		std::shared_ptr<TextureAsset> shrd_tex_ass_normal;
		if (weakNormalTexAss.lock()) {
			shrd_tex_ass_normal = std::dynamic_pointer_cast<TextureAsset>(weakNormalTexAss.lock());
		}
		GLint loc_hasNormalMap = glGetUniformLocation(shader, "uMaterial.hasNormalTex");
		if (shrd_tex_ass_normal > 0) {
			GLint loc_uNormalMap = glGetUniformLocation(shader, "uNormalTex");
			glBindTextureUnit(1, shrd_tex_ass_normal->textureID);
			glProgramUniform1i(shader, loc_uNormalMap, 1);
			glProgramUniform1i(shader, loc_hasNormalMap, 1);
		}
		else {
			glProgramUniform1i(shader, loc_hasNormalMap, 0);
		}

		//material params
		GLint locBaseColor = glGetUniformLocation(shader, "uBaseColor");
		GLint locMetallic = glGetUniformLocation(shader, "uMetallic");
		GLint locRoughness = glGetUniformLocation(shader, "uRoughness");
		GLint locUvscale = glGetUniformLocation(shader, "uUvScale");
		GLint locTint = glGetUniformLocation(shader, "uTint");

		//default material
		glm::vec4 baseColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
		float metallic = 0.f;
		float roughness = 0.5f;
		float uvScale = 1.f;
		float tint = 1.f;

		if (material_sp)
		{
			baseColor = material_sp->GetBaseColor();
			metallic = material_sp->GetMetallic();
			roughness = material_sp->GetRoughness();
			uvScale = material_sp->GetUV_scale();
			tint = material_sp->GetTint();
		}

		if (locBaseColor != -1)
			glUniform4fv(locBaseColor, 1, glm::value_ptr(baseColor));
		if (locMetallic != -1)
			glUniform1f(locMetallic, metallic);
		if (locRoughness != -1)
			glUniform1f(locRoughness, roughness);
		if (locUvscale != -1)
			glUniform1f(locUvscale, uvScale);
		if (locTint != -1)
			glUniform1f(locTint, tint);
	

		// Matrices per-submesh
		if (locM != -1) glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(trf.getM2W()));
		if (locN != -1) glUniformMatrix3fv(locN, 1, GL_FALSE, glm::value_ptr(trf.getNormalMat()));

		// Draw
		//glBindBufferBase(GL_UNIFORM_BUFFER, 2, submesh.materialUBO);
		glBindVertexArray(submesh.VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, submesh.IBO);

		// Inside DrawLight loop, before glDrawElements...
		if (subIdx == 0 && !submesh.vertices.empty()) {
			const auto& v = submesh.vertices[0].position;

			// Fix 1: Construct vec4 by passing X, Y, Z manually from the array
			glm::vec4 localPos(v[0], v[1], v[2], 1.0f);

			// Calculate world space
			glm::vec4 worldPos = trf.getM2W() * localPos;

			// Fix 2: Log using array indices [0], [1], [2] instead of .x, .y, .z
			/*AG_CORE_WARN("DEBUG MESH: Local Pos: ({}, {}, {}) | World Pos: ({}, {}, {})",
				v[0], v[1], v[2],
				worldPos.x, worldPos.y, worldPos.z);*/
		}

		if (!submesh.indices.empty())
			glDrawElements(GL_TRIANGLES, (GLsizei)submesh.indices.size(), GL_UNSIGNED_INT, (void*)0);
		else
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)submesh.vertices.size());
		glBindVertexArray(0);

		++subIdx;

		if (mesh.submeshes.empty()) AG_CORE_ERROR("Submeshes is empty!");
		else if (mesh.submeshes[0].vertices.empty()) AG_CORE_ERROR("Vertices is empty!");

	}
}

static inline uint64_t fnv1a64(const char* data, size_t len) {
	uint64_t hash = 1469598103934665603ull;      // offset basis
	const uint64_t prime = 1099511628211ull;      // FNV prime
	for (size_t i = 0; i < len; ++i) {
		hash ^= static_cast<uint8_t>(data[i]);
		hash *= prime;
	}
	return hash;
}

void AG::Component::MeshRendererComponent::DrawObjsColor(const StaticMeshAsset& mesh, TransformComponent& trf) {
	
	/*
	For each object: 

		uMVP = proj * view * model

		uIdRgb = EncodeID24(object.GetID())

		draw submeshes (whole object = one solid ID color)
			-> make a new render pipeline
	*/

	SHADERMANAGER.Use("ObjPicking");
	GLuint prog = SHADERMANAGER.GetShaderProgram();

	auto camera = CAMERAMANAGER.getSceneCamera();
	if (!camera.get()) return;

	GLint locP = glGetUniformLocation(prog, "P");
	GLint locV = glGetUniformLocation(prog, "V");
	GLint locM = glGetUniformLocation(prog, "M");

	if (locP != -1)
		glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
	if (locV != -1)
		glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	if (locM != -1)
		glUniformMatrix4fv(locM, 1, GL_FALSE,
			glm::value_ptr(trf.getM2W()));
	
	Data::GUID objID = this->GetObjID();
	//uint64_t id64 = fnv1a64(objID.c_str(), objID.size());
	uint64_t id64 = std::hash<std::string>{}(objID);
	objectPicking::GetInstance().id64_to_guid[id64] = objID;
	GLuint low = static_cast<GLuint>(id64 & 0xFFFFFFFFull);
	GLuint high = static_cast<GLuint>((id64 >> 32) & 0xFFFFFFFFull);

	if (GLint loc = glGetUniformLocation(prog, "uId64"); loc != -1) {
		glUniform2ui(loc, low, high);
	}


	// Draw all submeshes
	for (const auto& sub : mesh.submeshes) {
		glBindVertexArray(sub.VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sub.IBO);
		if (!sub.indices.empty())
			glDrawElements(GL_TRIANGLES, (GLsizei)sub.indices.size(), GL_UNSIGNED_INT, (void*)0);
		else
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sub.vertices.size());
	}
	glBindVertexArray(0);

}

void AG::Component::MeshRendererComponent::SetMeshRendererSubmeshMaterial(std::string submeshName, std::string materialName)
{
	if (model_hash == 0) return;
	auto& assetMgr = AssetManager::GetInstance();
	auto mesh_asset = std::dynamic_pointer_cast<StaticMeshAsset>(assetMgr.GetAsset(model_hash).lock());
	if (!mesh_asset) return;

	for (size_t i = 0; i < mesh_asset->submeshes.size(); ++i)
	{
		const std::string& current_submesh_name = mesh_asset->submeshes[i].name;
		if (current_submesh_name == submeshName) //check prefix
		{
			if (i >= material_hash_list.size()) continue;
			size_t material_hash = HASH(materialName);
			auto material_asset = std::dynamic_pointer_cast<MaterialTemplate>(assetMgr.GetAsset(material_hash).lock());
			if (material_asset)
			{
				material_hash_list[i] = material_hash;
			}
		}
	}
}

void AG::Component::MeshRendererComponent::setMeshfromString(std::string str)
{
	if (str.empty()) return;

	auto asset = AssetManager::GetInstance().GetAsset(AssetManager::GetInstance().hasher(str));
	if (!asset.lock()) return;

	if (auto asset_shr = std::reinterpret_pointer_cast<StaticMeshAsset>(asset.lock()))
	{
		mesh_name = str;
		setMeshfromHash(AssetManager::GetInstance().hasher(mesh_name));
	}
}

void AG::Component::MeshRendererComponent::setMeshfromHash(size_t hash)
{
	model_hash = hash;
	auto asset = AssetManager::GetInstance().GetAsset(model_hash);
	auto assetLock = asset.lock();
	if (!assetLock) return;
	auto mesh = std::dynamic_pointer_cast<StaticMeshAsset>(assetLock);
	if (!mesh) return;
	material_hash_list.resize(mesh->submeshes.size());
	material_hash_list.assign(mesh->submeshes.size(), 0);
}


REGISTER_REFLECTED_TYPE(AG::Component::MeshRendererComponent)
