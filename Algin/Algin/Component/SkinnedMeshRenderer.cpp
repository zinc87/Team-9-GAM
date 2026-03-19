#include "pch.h"
#include "SkinnedMeshRenderer.h"

void AG::Component::SkinnedMeshRenderer::Awake()
{
}

void AG::Component::SkinnedMeshRenderer::Start()
{
}

void AG::Component::SkinnedMeshRenderer::Update()
{
	// ─────────────────────────────
	// Skip if not playing or no mesh
	// ─────────────────────────────
	if (!play || skinned_mesh_hash == 0)
		return;

	auto& assetMgr = AssetManager::GetInstance();
	auto locked = assetMgr.GetAsset(skinned_mesh_hash).lock();
	auto mesh = std::dynamic_pointer_cast<SkinnedMeshAsset>(locked);
	if (!mesh)
		return;

	float dt = static_cast<float>(BENCHMARKER.GetDeltaTime());

	// ─────────────────────────────
	// 1. Detect Animation Switch for Interpolation
	// ─────────────────────────────
	// If the name changed, move the current state into the 'transition' bucket
	if (animation_clip_name != last_clip_name && !last_clip_name.empty()) {
		transition.old_clip_name = last_clip_name;
		transition.old_time = current_time;
		transition.fade_time = 0.0f;
		transition.active = true;
		current_time = 0.0f; // Start new animation from the beginning
	}
	last_clip_name = animation_clip_name;

	// ─────────────────────────────
	// 2. Find Clips (Current and Old)
	// ─────────────────────────────
	const AnimationClip* clip = FindClip(mesh, animation_clip_name);
	const AnimationClip* oldClip = transition.active ? FindClip(mesh, transition.old_clip_name) : nullptr;

	if (!clip) return;

	// ─────────────────────────────
	// 3. Advance Animation Times
	// ─────────────────────────────
	current_time += dt;

	const float durationSec = clip->duration / clip->ticksPerSecond;
	if (current_time > durationSec)
	{
		if (loop)
			current_time = fmod(current_time, durationSec);
		else
		{
			current_time = durationSec;
			play = false;
		}
	}

	// Handle the fade progress
	if (transition.active) {
		transition.fade_time += dt;
		transition.old_time += dt; // Keep the old animation moving so it doesn't freeze

		if (transition.fade_time >= transition.fade_duration) {
			transition.active = false; // Transition complete
		}
	}

	// ─────────────────────────────
	// 4. Prepare bone transforms
	// ─────────────────────────────
	if (boneTransforms.size() != mesh->bones.size())
		boneTransforms.resize(mesh->bones.size(), glm::mat4(1.0f));

	// Calculate how much of the "New" animation to show (0.0 to 1.0)
	float blendWeight = transition.active ? (transition.fade_time / transition.fade_duration) : 1.0f;

	// ─────────────────────────────
	// 5. Build each bone’s global transform
	// ─────────────────────────────
	for (size_t i = 0; i < mesh->bones.size(); ++i)
	{
		const Bone& bone = mesh->bones[i];

		// Sample the primary (new) animation
		glm::mat4 currentLocal = SampleBone(clip, bone.name, current_time);
		glm::mat4 finalLocal;

		if (transition.active && oldClip)
		{
			// Sample the old animation
			glm::mat4 oldLocal = SampleBone(oldClip, bone.name, transition.old_time);

			// Blend the two matrices together
			finalLocal = MixMatrices(oldLocal, currentLocal, blendWeight);
		}
		else
		{
			finalLocal = currentLocal;
		}

		// Build model-space transform (apply parent if exists)
		if (bone.parentIndex >= 0)
			boneTransforms[i] = boneTransforms[bone.parentIndex] * finalLocal;
		else
			boneTransforms[i] = finalLocal;
	}

	// ─────────────────────────────
	// 6. Final skinning matrices (model-space → bone-space)
	// ─────────────────────────────
	for (size_t i = 0; i < mesh->bones.size(); ++i)
	{
		boneTransforms[i] = boneTransforms[i] * mesh->bones[i].inverseBindPose;
	}

	if (use_correction)
	{
		for (size_t i = 0; i < boneTransforms.size(); ++i)
		{
			boneTransforms[i] = mesh->correction * boneTransforms[i];
		}
	}
}


void AG::Component::SkinnedMeshRenderer::LateUpdate()
{	

	auto obj = GetObj().lock();
	if (obj == nullptr) return;

	auto trf = GetObj().lock()->GetComponent<Component::TransformComponent>().lock();
	if (!trf) return;

	auto asset = AssetManager::GetInstance().GetAsset(skinned_mesh_hash);
	auto assetLock = asset.lock();
	if (!assetLock) return;

	auto meshAsset = std::dynamic_pointer_cast<SkinnedMeshAsset>(assetLock);
	if (!meshAsset) return;

	auto state = RenderPipeline::GetInstance().GetPipeline();

	if (state == RenderPipeline::SHADOW) {
		//drawShadows
		DrawShadows(*meshAsset, *trf);
	}
	else if (state == RenderPipeline::LIGHT) {
		DrawMesh(*meshAsset, *trf);
	}
	else if (state == RenderPipeline::OBJPICK) {
		DrawForObjPick(*meshAsset, *trf);
	}

}


void AG::Component::SkinnedMeshRenderer::Free()
{

}

void AG::Component::SkinnedMeshRenderer::Inspector()
{
	auto& assetMgr = AssetManager::GetInstance();

	// =========================
	// Mesh selection (SEARCHABLE)
	// =========================
	auto meshAssets = assetMgr.GetAssets(ASSET_TYPE::AGSKINNEDMESH);

	std::string currentMeshName = "No Mesh";
	if (skinned_mesh_hash != 0)
	{
		auto currentPtr = assetMgr.GetAsset(skinned_mesh_hash).lock();
		if (currentPtr) currentMeshName = currentPtr->asset_name;
	}

	static ImGuiTextFilter filter;

	if (ImGui::BeginCombo("Mesh", currentMeshName.c_str()))
	{
		filter.Draw("##SkinnedSearch", -1.0f);
		ImGui::Separator();

		if (filter.PassFilter("No Mesh"))
		{
			bool isSelected = (skinned_mesh_hash == 0);
			if (ImGui::Selectable("No Mesh", isSelected))
			{
				skinned_mesh_hash = 0;
				material_hash_list.clear();
				material_value_list.clear();
			}
			if (isSelected) ImGui::SetItemDefaultFocus();
		}

		for (auto& [hash, sp] : meshAssets)
		{
			if (auto p = sp.lock())
			{
				const std::string& name = p->asset_name;
				if (filter.PassFilter(name.c_str()))
				{
					bool isSelected = (skinned_mesh_hash == HASH(name));
					if (ImGui::Selectable(name.c_str(), isSelected))
					{
						skinned_mesh_hash = HASH(name);
						material_hash_list.clear();
						material_value_list.clear();

						auto meshPtr = std::dynamic_pointer_cast<SkinnedMeshAsset>(p);
						if (meshPtr)
						{
							const size_t subCount = meshPtr->submeshes.size();
							material_hash_list.resize(subCount * 3, 0);
							material_value_list.resize(subCount * 3, glm::vec3(0.1f));
						}
					}
					if (isSelected) ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}

	auto meshAssetPtr = assetMgr.GetAsset(skinned_mesh_hash).lock();
	auto meshPtr = std::dynamic_pointer_cast<SkinnedMeshAsset>(meshAssetPtr);
	if (!meshPtr) return;

	// =========================
	// Animations table (guarded)
	// =========================
	if (ImGui::BeginTable("AnimationClipsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("Clip Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Duration (s)", ImGuiTableColumnFlags_WidthFixed, 120.0f);
		ImGui::TableHeadersRow();

		for (const auto& ani : meshPtr->animations)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(ani.name.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.2f", ani.duration);
		}
		ImGui::EndTable();
	}

	// =========================
	// Animation selection
	// =========================
	std::vector<const char*> clip_names;
	clip_names.push_back("No Animation");
	for (const auto& ani : meshPtr->animations)
		clip_names.push_back(ani.name.c_str());

	int clip_index = 0;
	for (int i = 0; i < (int)clip_names.size(); ++i)
		if (animation_clip_name == clip_names[i]) { clip_index = i; break; }

	if (clip_names.size() > 1)
	{
		if (ImGui::Combo("Animation Clip", &clip_index, clip_names.data(), (int)clip_names.size()))
		{
			animation_clip_name = (clip_index == 0) ? "" : clip_names[clip_index];
			current_time = 0.f;
		}

		ImGui::SeparatorText("Playback Controls");
		ImGui::Text("Ticks: %.2f", current_time);
		ImGui::Checkbox("Loop Animation", &loop);
		ImGui::Checkbox("Use Correction Matrix", &use_correction);

		if (ImGui::Button(play ? "Pause" : "Play")) play = !play;
		if (!play && current_time > 0.f)
		{
			ImGui::SameLine();
			if (ImGui::Button("Reset")) current_time = 0.f;
		}
	}

	// =========================
	// Materials / SubMeshes
	// =========================
	ImGui::SeparatorText("Materials");

	if (ImGui::TreeNode("SubMeshes"))
	{
		const size_t subCount = meshPtr->submeshes.size();

		// Build material list
		std::vector<const char*> materialNames;
		materialNames.push_back("Default/None");
		auto mAssets = assetMgr.GetAssets(ASSET_TYPE::AGMATERIAL);

		for (auto& [name, asset] : mAssets)
			if (auto a = asset.lock()) materialNames.push_back(a->asset_name.c_str());

		// Ensure sizes
		if (material_hash_list.size() < subCount * 3) material_hash_list.resize(subCount * 3, 0);
		if (material_value_list.size() < subCount * 3) material_value_list.resize(subCount * 3, glm::vec3(0.1f));

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		// Shared filter
		static ImGuiTextFilter matFilter;

		for (size_t i = 0; i < subCount; ++i)
		{
			SubMeshSkinned& sub = meshPtr->submeshes[i];
			std::string label = sub.name.empty() ? ("Submesh " + std::to_string(i)) : sub.name;

			ImGui::PushID((int)i);

			if (ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s", label.c_str()))
			{
				// 1. Find current index
				int currentIdx = 0;
				for (int n = 0; n < materialNames.size(); ++n)
				{
					if (n == 0 && material_hash_list[i] == 0) { currentIdx = 0; break; }

					if (material_hash_list[i] == HASH(materialNames[n]))
					{
						currentIdx = n;
						break;
					}
				}

				// 2. Preview Text
				const char* previewValue = (currentIdx >= 0 && currentIdx < materialNames.size())
					? materialNames[currentIdx] : "None";

				// 3. Searchable Dropdown
				std::string comboId = "Materials##Skinned" + std::to_string(i);
				if (ImGui::BeginCombo(comboId.c_str(), previewValue))
				{
					if (ImGui::IsWindowAppearing())
						ImGui::SetKeyboardFocusHere();

					matFilter.Draw("##matSearchSkin", -1.0f);
					ImGui::Separator();

					for (int n = 0; n < materialNames.size(); ++n)
					{
						if (matFilter.PassFilter(materialNames[n]))
						{
							bool isSelected = (currentIdx == n);
							if (ImGui::Selectable(materialNames[n], isSelected))
							{
								if (n == 0) material_hash_list[i] = 0;
								else material_hash_list[i] = HASH(materialNames[n]);
							}
							if (isSelected) ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				// Debugging info (preserved from your original code)
				auto currMatAsset = assetMgr.GetAsset(material_hash_list[i]).lock();
				auto matAsset = std::dynamic_pointer_cast<MaterialTemplate>(currMatAsset);
				if (matAsset)
				{
					ImGui::Text("Material Name: %s", matAsset->asset_name.c_str());
					ImGui::Text("material_hash_list.size(): %d", (int)material_hash_list.size());
				}

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	if (ImGui::Button("Test"))
	{
		SetSubmeshMaterial("Body", "P1_Body.agmat");
	}

	if (ImGui::Button("Scan")) {
		ScanSetup::GetInstance().scanning = !ScanSetup::GetInstance().scanning;
	}
	ImGui::SliderFloat("Radius", &SP.radius, 0.0f, 1.0f);
	if (ScanSetup::GetInstance().scanning)
		ImGui::Text("SCAN ON");
}


void AG::Component::SkinnedMeshRenderer::AssignFrom([[maybe_unused]] const std::shared_ptr<IComponent>& fromCmp)
{
	ASSIGNFROM_FN_BODY(
		this->skinned_mesh_hash = fromCmpPtr->skinned_mesh_hash;
		this->animation_clip_name = fromCmpPtr->animation_clip_name;
		this->loop = fromCmpPtr->loop;
		this->play = fromCmpPtr->play;
		this->use_correction = fromCmpPtr->use_correction;
		this->shininess = fromCmpPtr->shininess;

		this->material_hash_list.resize(fromCmpPtr->material_hash_list.size());
		this->material_value_list.resize(fromCmpPtr->material_value_list.size());

		std::copy(fromCmpPtr->material_hash_list.begin(), fromCmpPtr->material_hash_list.end(), this->material_hash_list.begin());
		std::copy(fromCmpPtr->material_value_list.begin(), fromCmpPtr->material_value_list.end(), this->material_value_list.begin());
		)
}

void AG::Component::SkinnedMeshRenderer::DrawShadows(const SkinnedMeshAsset& mesh, TransformComponent& trf) {

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

void AG::Component::SkinnedMeshRenderer::ChangeModel(std::string model_name)
{
	size_t new_model_hash = HASH(model_name);

	// check if model exist or different
	if (skinned_mesh_hash != new_model_hash)
	{
		auto& assetMgr = AssetManager::GetInstance();
		auto modelAsset = assetMgr.GetAsset(new_model_hash).lock();
		if (modelAsset)
		{
			skinned_mesh_hash = new_model_hash;
			auto meshPtr = std::dynamic_pointer_cast<SkinnedMeshAsset>(modelAsset);
			if (meshPtr)
			{
				const size_t subCount = meshPtr->submeshes.size();
				material_hash_list.resize(subCount, 0);
				material_value_list.resize(subCount * 3, glm::vec3(0.1f));
			}
		}
	}

}

void AG::Component::SkinnedMeshRenderer::ChangeShaderExcept(std::string shader_name, std::string submesh_name)
{
	if (skinned_mesh_hash == 0 || shader_name.empty()) return;

	auto& assetMgr = AssetManager::GetInstance();
	auto mesh_asset = std::dynamic_pointer_cast<SkinnedMeshAsset>(assetMgr.GetAsset(skinned_mesh_hash).lock());

	if (!mesh_asset) return;

	for (size_t i = 0; i < mesh_asset->submeshes.size(); ++i)
	{
		const std::string& current_submesh_name = mesh_asset->submeshes[i].name;

		if (current_submesh_name.find(submesh_name) != 0) //check prefix
		{
			if (i >= material_hash_list.size()) continue;

			auto material_asset = std::dynamic_pointer_cast<MaterialTemplate>(assetMgr.GetAsset(material_hash_list[i]).lock());

			if (material_asset)
			{
				material_asset->shader_name = shader_name;
			}
		}
	}

}

void AG::Component::SkinnedMeshRenderer::SetSubmeshMaterial(std::string submesh_name, std::string material_name)
{

	AG_CORE_INFO("start of setsubmeshmaterial");
	if (skinned_mesh_hash == 0) return;

	AG_CORE_INFO("skinned_mesh_hash != 0");
	auto& assetMgr = AssetManager::GetInstance();
	auto mesh_asset = std::dynamic_pointer_cast<SkinnedMeshAsset>(assetMgr.GetAsset(skinned_mesh_hash).lock());
	if (!mesh_asset) return;
	AG_CORE_INFO("mesh_asset exist");
	for (size_t i = 0; i < mesh_asset->submeshes.size(); ++i)
	{
		const std::string& current_submesh_name = mesh_asset->submeshes[i].name;
		AG_CORE_INFO("searching for submesh name");
		if (current_submesh_name == submesh_name) //check prefix
		{
			AG_CORE_INFO("Found submesh: {}", current_submesh_name);
			if (i >= material_hash_list.size()) continue;
			size_t material_hash = HASH(material_name);
			auto material_asset = std::dynamic_pointer_cast<MaterialTemplate>(assetMgr.GetAsset(material_hash).lock());
			if (material_asset)
			{
				material_hash_list[i] = material_hash;
				AG_CORE_INFO("Set Material = {}", material_name);
			}
		}
	}
}

void AG::Component::SkinnedMeshRenderer::DrawMesh(SkinnedMeshAsset& meshAsset, TransformComponent& trf) {
	auto camera = CAMERAMANAGER.getCurrentCamera().lock();
	if (!camera) return;

	// Draw selection wireframe if a submesh is selected
	if (submesh_selected_index >= 0 && submesh_selected_index < (int)meshAsset.submeshes.size())
	{
		DebugRenderer::GetInstance().DrawAABBWireframe(trf.getM2W(), meshAsset.submeshes[submesh_selected_index].minmax);
	}

	// --- Pre-calculate Scan System Params (Global for this Mesh) ---
	if (ScanSetup::GetInstance().scanning)
	{
		glm::mat4 Proj = camera->GetProjectionMatrix();
		glm::mat4 view = camera->GetViewMatrix();
		viewProj = Proj * view;

		float vX = static_cast<float>(AGWINDOW.getWindowSize().first);
		float vY = static_cast<float>(AGWINDOW.getWindowSize().second);
		aspect = vX / vY;

		glm::vec2 mouse = glm::vec2(ScanSetup::GetInstance().scanMousePos.x, ScanSetup::GetInstance().scanMousePos.y);
		mouse.x = mouse.x * (vX / ScanSetup::GetInstance().viewDims.x);
		mouse.y = mouse.y * (vY / ScanSetup::GetInstance().viewDims.y);

		Ray ray = ScanSetup::GetInstance().BuildRay(mouse.x, mouse.y, vX, vY, view, Proj, camera->getCameraPosition());

		glm::vec3 hitPosWS = glm::vec3(0.f);
		float outRef;
		if (ScanSetup::GetInstance().ScanHit(ray, hitPosWS, outRef, meshAsset.submeshes, trf.getM2W()))
		{
			SP.active = 1;
		}
		SP.centerNDC = glm::vec2(0.f, 0.f); // Center of screen for gameplay mode
		SP.radNDC = 0.1f; // Adjust as needed
	}

	size_t subIdx = 0;
	for (auto& submesh : meshAsset.submeshes) {

		// 1. FRUSTUM CULLING
		BENCHMARKER.FrustumCheckInc();
		if (!Camera::IsObjectInCamera(trf.getM2W(), submesh.minmax))
		{
			BENCHMARKER.FrustumCulledInc();
			// Optional: skip drawing if culled
			// ++subIdx; continue; 
		}

		// 2. MATERIAL & SHADER SELECTION
		// Fetch material assigned to this specific submesh index
		auto mat_ass_wk = AssetManager::GetInstance().GetAsset(material_hash_list[subIdx]);
		auto mat_ass_sp = mat_ass_wk.lock();
		auto material_sp = std::dynamic_pointer_cast<MaterialTemplate>(mat_ass_sp);

		// Use material shader or fallback to default skinned shader
		std::string shader_name = material_sp ? material_sp->GetShaderName() : "SkinnedShader";
		SHADERMANAGER.Use(shader_name);
		GLuint shader = ShaderManager::GetInstance().GetShaderProgram();

		auto windowSize = AGWindow::GetInstance().getWindowSize();
		if (GLint locWidth = glGetUniformLocation(shader, "uScreenWidth"); locWidth != -1) {
			glUniform1i(locWidth, windowSize.first);
		}
		if (GLint locHeight = glGetUniformLocation(shader, "uScreenHeight"); locHeight != -1) {
			glUniform1i(locHeight, windowSize.second);
		}

		// 3. UPLOAD CAMERA & LIGHT UNIFORMS
		GLint locP = glGetUniformLocation(shader, "P");
		GLint locV = glGetUniformLocation(shader, "V");
		GLint locM = glGetUniformLocation(shader, "M");
		GLint locN = glGetUniformLocation(shader, "uNormalMat");

		if (locP != -1) glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
		if (locV != -1) glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
		if (locM != -1) glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(trf.getM2W()));
		if (locN != -1) glUniformMatrix3fv(locN, 1, GL_FALSE, glm::value_ptr(trf.getNormalMat()));

		if (GLint locViewPos = glGetUniformLocation(shader, "camPos"); locViewPos != -1)
			glUniform3fv(locViewPos, 1, glm::value_ptr(camera->getCameraPosition()));

		// Lights
		const auto lights = LightManager::GetInstance().getLightList();
		int lightCount = std::min<int>((int)lights.size(), 16);
		if (GLint locCount = glGetUniformLocation(shader, "uLightCount"); locCount != -1)
			glUniform1i(locCount, lightCount);

		for (int i = 0; i < lightCount; ++i) {
			const Light& L = lights[i];
			const std::string base = "uLights[" + std::to_string(i) + "].";
			glUniform1i(glGetUniformLocation(shader, (base + "type").c_str()), (int)L.type);
			glUniform3fv(glGetUniformLocation(shader, (base + "position").c_str()), 1, glm::value_ptr(L.position));
			glUniform3fv(glGetUniformLocation(shader, (base + "direction").c_str()), 1, glm::value_ptr(glm::normalize(L.direction)));
			glUniform1f(glGetUniformLocation(shader, (base + "range").c_str()), L.range);
			glUniform1f(glGetUniformLocation(shader, (base + "intensity").c_str()), L.intensity);
		}

		// 4. UPLOAD BONE TRANSFORMS (Skinned Mesh specific)
		GLint locB = glGetUniformLocation(shader, "uBones[0]");
		GLint locNumB = glGetUniformLocation(shader, "uNumBones");
		if (!boneTransforms.empty()) {
			int uploadCount = std::min<int>((int)boneTransforms.size(), 250);
			if (locB != -1) glUniformMatrix4fv(locB, uploadCount, GL_FALSE, glm::value_ptr(boneTransforms[0]));
			if (locNumB != -1) glUniform1i(locNumB, uploadCount);
		}
		else if (locNumB != -1) {
			glUniform1i(locNumB, 1);
			static const glm::mat4 identity(1.0f);
			if (locB != -1) glUniformMatrix4fv(locB, 1, GL_FALSE, glm::value_ptr(identity));
		}

		// 5. UPLOAD MATERIAL TEXTURES & PARAMS
		GLint loc_hasAlbedo = glGetUniformLocation(shader, "uMaterial.hasDiffuseTex");
		GLint loc_hasNormal = glGetUniformLocation(shader, "uMaterial.hasNormalTex");

		if (material_sp) {
			// Albedo
			auto albedoTex = std::dynamic_pointer_cast<TextureAsset>(AssetManager::GetInstance().GetAsset(material_sp->GetAlbedoHash()).lock());
			if (albedoTex) {
				glBindTextureUnit(0, albedoTex->textureID);
				glUniform1i(glGetUniformLocation(shader, "uDiffuseTex"), 0);
				glUniform1i(loc_hasAlbedo, 1);
			}
			else glUniform1i(loc_hasAlbedo, 0);

			// Normal
			auto normalTex = std::dynamic_pointer_cast<TextureAsset>(AssetManager::GetInstance().GetAsset(material_sp->GetNormalHash()).lock());
			if (normalTex) {
				glBindTextureUnit(1, normalTex->textureID);
				glUniform1i(glGetUniformLocation(shader, "uNormalTex"), 1);
				glUniform1i(loc_hasNormal, 1);
			}
			else glUniform1i(loc_hasNormal, 0);

			// Material Constants
			glUniform4fv(glGetUniformLocation(shader, "uBaseColor"), 1, glm::value_ptr(material_sp->GetBaseColor()));
			glUniform1f(glGetUniformLocation(shader, "uMetallic"), material_sp->GetMetallic());
			glUniform1f(glGetUniformLocation(shader, "uRoughness"), material_sp->GetRoughness());
			glUniform1f(glGetUniformLocation(shader, "uUvScale"), material_sp->GetUV_scale());
			glUniform1f(glGetUniformLocation(shader, "uTint"), material_sp->GetTint());
		}
		else {
			// Fallback for submesh with no material
			glUniform1i(loc_hasAlbedo, 0);
			glUniform1i(loc_hasNormal, 0);
			glUniform4f(glGetUniformLocation(shader, "uBaseColor"), 1, 1, 1, 1);
		}

		// 6. SCAN SYSTEM UNIFORMS (Submesh specific)
		submesh.scanAffected = (submesh.name == "PANTS.001" || submesh.name == "SHIRT.001");
		glUniform1i(glGetUniformLocation(shader, "uScanAffect"), submesh.scanAffected ? 1 : 0);
		glUniform1i(glGetUniformLocation(shader, "uScanActive"), SP.active);
		glUniform1i(glGetUniformLocation(shader, "uScanActiveInt"), (ScanSetup::GetInstance().scanning && SP.active) ? 1 : 0);
		glUniformMatrix4fv(glGetUniformLocation(shader, "uViewProj"), 1, GL_FALSE, glm::value_ptr(viewProj));
		glUniform2f(glGetUniformLocation(shader, "uScanCenterNDC"), SP.centerNDC.x, SP.centerNDC.y);
		glUniform1f(glGetUniformLocation(shader, "uScanRadNDC"), SP.radNDC);
		glUniform1f(glGetUniformLocation(shader, "uAspect"), aspect);

		// 7. DRAW CALL
		glBindVertexArray(submesh.VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, submesh.IBO);
		if (!submesh.indices.empty())
			glDrawElements(GL_TRIANGLES, (GLsizei)submesh.indices.size(), GL_UNSIGNED_INT, (void*)0);
		else
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)submesh.vertices.size());

		glBindVertexArray(0);
		++subIdx;
	}
}

void AG::Component::SkinnedMeshRenderer::DrawForObjPick(const SkinnedMeshAsset& meshAsset, TransformComponent& trf) {
	
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
	for (const auto& sub : meshAsset.submeshes) {
		glBindVertexArray(sub.VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sub.IBO);
		if (!sub.indices.empty())
			glDrawElements(GL_TRIANGLES, (GLsizei)sub.indices.size(), GL_UNSIGNED_INT, (void*)0);
		else
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sub.vertices.size());
	}
	glBindVertexArray(0);
}

glm::vec3 AG::Component::SkinnedMeshRenderer::InterpolateTranslation(const BoneAnimation& boneAnim, float time)
{
	if (boneAnim.keyframes.empty())
		return glm::vec3(0.0f);

	if (boneAnim.keyframes.size() == 1)
		return boneAnim.keyframes[0].translation;

	for (size_t i = 0; i < boneAnim.keyframes.size() - 1; ++i)
	{
		const auto& k1 = boneAnim.keyframes[i];
		const auto& k2 = boneAnim.keyframes[i + 1];
		if (time < k2.time)
		{
			float t = (time - k1.time) / (k2.time - k1.time);
			return glm::mix(k1.translation, k2.translation, t);
		}
	}
	return boneAnim.keyframes.back().translation;
}

glm::quat AG::Component::SkinnedMeshRenderer::InterpolateRotation(const BoneAnimation& boneAnim, float time)
{
	if (boneAnim.keyframes.empty())
		return glm::quat(1, 0, 0, 0);

	if (boneAnim.keyframes.size() == 1)
		return boneAnim.keyframes[0].rotation;

	for (size_t i = 0; i < boneAnim.keyframes.size() - 1; ++i)
	{
		const auto& k1 = boneAnim.keyframes[i];
		const auto& k2 = boneAnim.keyframes[i + 1];
		if (time < k2.time)
		{
			float t = (time - k1.time) / (k2.time - k1.time);
			return glm::slerp(k1.rotation, k2.rotation, t);
		}
	}
	return boneAnim.keyframes.back().rotation;
}

glm::vec3 AG::Component::SkinnedMeshRenderer::InterpolateScale(const BoneAnimation& boneAnim, float time)
{
	if (boneAnim.keyframes.empty())
		return glm::vec3(1.0f);

	if (boneAnim.keyframes.size() == 1)
		return boneAnim.keyframes[0].scale;

	for (size_t i = 0; i < boneAnim.keyframes.size() - 1; ++i)
	{
		const auto& k1 = boneAnim.keyframes[i];
		const auto& k2 = boneAnim.keyframes[i + 1];
		if (time < k2.time)
		{
			float t = (time - k1.time) / (k2.time - k1.time);
			return glm::mix(k1.scale, k2.scale, t);
		}
	}
	return boneAnim.keyframes.back().scale;
}

const AnimationClip* AG::Component::SkinnedMeshRenderer::FindClip(const std::shared_ptr<SkinnedMeshAsset>& mesh, const std::string& name) {
	for (const auto& c : mesh->animations) {
		if (c.name == name) return &c;
	}
	return nullptr;
}

glm::mat4 AG::Component::SkinnedMeshRenderer::SampleBone(const AnimationClip* clip, const std::string& boneName, float time) {
	for (const auto& bAnim : clip->boneAnimations) {
		if (bAnim.boneName == boneName) {
			glm::vec3 T = InterpolateTranslation(bAnim, time);
			glm::quat R = InterpolateRotation(bAnim, time);
			glm::vec3 S = InterpolateScale(bAnim, time);
			return glm::translate(glm::mat4(1.0f), T) * glm::mat4_cast(R) * glm::scale(glm::mat4(1.0f), S);
		}
	}
	return glm::mat4(1.0f); // Default/Rest Pose
}

glm::mat4 AG::Component::SkinnedMeshRenderer::MixMatrices(const glm::mat4& a, const glm::mat4& b, float t) {
	// Simple matrix lerp. For higher quality, decompose and use Slerp.
	glm::mat4 res;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			res[i][j] = glm::mix(a[i][j], b[i][j], t);
	return res;
}

REGISTER_REFLECTED_TYPE(AG::Component::SkinnedMeshRenderer)
