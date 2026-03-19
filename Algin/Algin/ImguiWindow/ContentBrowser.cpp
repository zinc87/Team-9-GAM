#include "pch.h"
#include "ContentBrowser.h"

void AG::AGImGui::ContentBrowser::RenderMeshSettingsPopup()
{
	if (ImGui::BeginPopupModal("Mesh Import Settings", &m_ShowMeshSettingsPopup, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Import settings for: %s", m_Popup_SelectedMeshPath.filename().string().c_str());
		ImGui::Separator();

		ImGui::Text("Mesh Type:");
		ImGui::RadioButton("Static Mesh", &m_MeshImportType, 0);
		ImGui::SameLine();


		ImGui::RadioButton("Dynamic Mesh", &m_MeshImportType, 1);


		//ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Dynamic Mesh support coming soon.");

		ImGui::Separator();

		if (ImGui::Button("Configure", ImVec2(120, 0)))
		{
			if (m_MeshImportType == 0) // Static Mesh
			{
				AG::AssetCompiler::GetInstance().OpenStaticMeshImport(m_Popup_SelectedMeshPath);
			}
			// [CHANGE 3] Add logic for Dynamic Mesh (Type 1)
			else if (m_MeshImportType == 1)
			{
				AG::AssetCompiler::GetInstance().OpenSkinnedMeshImport(m_Popup_SelectedMeshPath);
			}

			m_ShowMeshSettingsPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			m_ShowMeshSettingsPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

AG::AGImGui::ContentBrowser::~ContentBrowser()
{
}

void AG::AGImGui::ContentBrowser::Render()
{
	ImGui::Begin("Asset Browser");

	/* --- Directories Render --- */
	ImGui::BeginChild("Directory", ImVec2(ImGui::GetContentRegionAvail().x / 4.5f, 0), ImGuiChildFlags_ResizeX);

	{ /* TO DO : TEMPORARY, REMOVE LATER*/
		if (ImGui::Button("Show Loaded Assets")) show_loaded_assets = true;
	}

	if (ImGui::TreeNodeEx("Assets", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen, "Assets"))
	{
		if (ImGui::IsItemClicked())
		{
			SelectDirectory(ASSET_PATH);
		}
		RenderDirectories(asset_file_path);
		ImGui::TreePop();
	}


	ImGui::EndChild(); ImGui::SameLine();

	/* --- Content Render --- */
	ImGui::BeginChild("Content");

	float search_bar_location = ImGui::GetContentRegionAvail().x * 0.8f;

	ImGui::Text("");
	/* --- Back Button --- */
	if (ImGui::ImageButton("##aa", 0, ImVec2(10, 10), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.f, 0.f, 0.f, 0.f)))
	{
		if (!previous_path.empty())
		{
			selected_directory = previous_path.top();
			previous_path.pop();
		}
	}
	ImGui::SameLine();

	/* --- Content Path --- */
	ImGui::Text(selected_directory.generic_string().c_str());
	ImGui::SameLine();

	/* --- Search Bar --- */
	ImGui::SetCursorPosX(search_bar_location);

	// Calculate space
	float available_width = ImGui::GetContentRegionAvail().x;
	float clear_button_width = 22.0f; // Width of the 'X' button

	// If text exists, reserve space for the button
	if (!search_bar.empty())
	{
		ImGui::PushItemWidth(available_width - clear_button_width - ImGui::GetStyle().ItemSpacing.x);
	}
	else
	{
		ImGui::PushItemWidth(available_width);
	}

	// 1. Check for ESC key to clear search
	if (ImGui::InputTextWithHint("##SearchBar", "Search assets...", &search_bar))
	{
		// Text changed logic if needed
	}

	// Check if ESC is pressed while the window is focused to clear search
	if (!search_bar.empty() && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape))
	{
		search_bar.clear();
	}

	ImGui::PopItemWidth();

	// 2. Render 'X' Button if text exists
	if (!search_bar.empty())
	{
		ImGui::SameLine();
		if (ImGui::Button("X", ImVec2(clear_button_width, 0)))
		{
			search_bar.clear();
		}

		// Optional: Tooltip to tell user they can clear
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Clear search (Esc)");
		}
	}

	ImGui::Separator();

	ImGui::Text("");
	RenderContent();
	ImGui::EndChild();

	ImGui::End();

	// --- ADD THIS BLOCK ---
	if (m_ShowTextureSettingsPopup)
	{
		// We call SetNextWindowPos to center the modal
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		// This will open the popup
		ImGui::OpenPopup("Texture Import Settings");
		RenderTextureSettingsPopup();
	}
	// --- END ADDED BLOCK ---
	if (m_ShowMeshSettingsPopup)
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::OpenPopup("Mesh Import Settings");
		RenderMeshSettingsPopup();
	}

	if (show_loaded_assets)
	{
		ViewLoadedAssets();
	}
}

void AG::AGImGui::ContentBrowser::RenderDirectories(const std::filesystem::path asset_directory)
{
	for (const auto& entry : std::filesystem::directory_iterator(asset_directory))
	{
		if (entry.is_directory())
		{
			/* --- Render files as tree nodes --- */
			std::string folder = entry.path().filename().string();

			if (ImGui::TreeNodeEx(folder.c_str(), ImGuiTreeNodeFlags_OpenOnArrow, folder.c_str()))
			{
				if (ImGui::IsItemClicked() && ImGui::IsItemHovered())
				{
					output_path = entry.path().string();
					SelectDirectory(entry.path());
				}
				RenderDirectories(entry.path());

				ImGui::TreePop();
			}
			else
			{
				if (ImGui::IsItemClicked() && ImGui::IsItemHovered())
				{
					output_path = entry.path().string();
					SelectDirectory(entry.path());
				}
			}
		}
		else
		{
			/* --- Render content as leaf nodes --- */
			std::string content = entry.path().filename().string();

			ImGuiTreeNodeFlags fileFlags =
				ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			ImGui::TreeNodeEx(content.c_str(), fileFlags, "%s", content.c_str());

			if (ImGui::IsItemClicked() && ImGui::IsItemHovered())
			{
				selected_asset = entry;
			}
		}
	}
}

void AG::AGImGui::ContentBrowser::RenderContent()
{
	float panelWidth = ImGui::GetContentRegionAvail().x;
	float cellWidth = AGWINDOW.getWindowSize().first / 15.f;
	ImVec2 cellSize = { cellWidth, cellWidth };
	columns = (unsigned int)(panelWidth / cellWidth - 2.f);
	columns = std::max(1, static_cast<int>(columns));
	ImGui::Columns(columns, 0, false);
    
	/*AG_CORE_ERROR("[ContentBrowser] RenderContent Start. SearchBar Empty: {}", search_bar.empty());
    AG_CORE_ERROR("[ContentBrowser] Selected Directory: '{}' (Exists: {}, IsDir: {})", 
        selected_directory.string(), 
        std::filesystem::exists(selected_directory), 
        std::filesystem::is_directory(selected_directory));*/

	if (search_bar.empty()) /* --- Search Bar Not In Use --- */
	{
		int loopCount = 0;
		for (const auto& entry : std::filesystem::directory_iterator(selected_directory))
		{
			loopCount++;
			//AG_CORE_ERROR("[ContentBrowser] Loop Item #{}: {}", loopCount, entry.path().filename().string());

			std::string entry_str = entry.path().generic_string();
			std::string contentName = entry.path().filename().string();
			ImTextureID texid = GetIcon(entry);

			if (entry.is_directory()) /* --- Render File Icon --- */
			{
				if (ImGui::ImageButton(contentName.c_str(), texid, cellSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.f, 0.f, 0.f, 0.f)))
				{
					SelectDirectory(entry);
				}
			}
			else /* --- Render Content Icon --- */
			{
				ImVec2 uv0(0, 0), uv1(1, 1);
                
                //AG_CORE_ERROR("[ContentBrowser] Checking AssetType for: {}", contentName);
				auto type = AssetManager::GetInstance().GetFileType(entry);
                //AG_CORE_ERROR("[ContentBrowser] AssetType: {}", (int)type);

				if (type == ASSET_TYPE::DDS || (type == ASSET_TYPE::TEXTURE && entry.path().extension() == ".dds")) {
					size_t h = AssetManager::GetInstance().hasher(contentName);
					auto& map = AssetManager::GetInstance().GetAssets(type);
					if (map.find(h) != map.end()) { uv0 = { 0, 1 }; uv1 = { 1, 0 }; }
				}

				ImGui::ImageButton(contentName.c_str(), texid, cellSize, uv0, uv1, ImVec4(0.f, 0.f, 0.f, 0.f));

                // --- DRAG DROP LOGIC (MAIN LOOP) ---
                std::string ext = entry.path().extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
					return static_cast<char>(std::tolower(c));
					});

                if (ext == ".mpg")
                {
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        std::string vidPath = entry.path().string();
                        ImGui::SetDragDropPayload("VIDEO_FILE", vidPath.c_str(), vidPath.size() + 1, ImGuiCond_Once);
                        ImGui::TextUnformatted(contentName.c_str());
                        ImGui::EndDragDropSource();
                    }
                }
                else if (ImGui::BeginDragDropSource())
                {
                    const char* content = contentName.c_str();
                    ImGui::SetDragDropPayload("IMGUI_CONTENT", content, contentName.size() * sizeof(char), ImGuiCond_Once);
                    ImGui::EndDragDropSource();
                }
                // --- END DRAG DROP LOGIC ---

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					// --- START MODIFIED LOGIC ---
					auto asset_type = AssetManager::GetInstance().GetFileType(entry);
					std::string extension = entry.path().extension().string();

					// ONLY open the popup for source texture files (.png, .jpg, etc.)
					if (asset_type == ASSET_TYPE::TEXTURE)
					{
						// We clicked a source file. Use it as the source input.
						m_Popup_SelectedAssetPath = entry.path();
						m_Popup_SourceImagePath = entry.path().string();

						// Set default settings for a new import
						m_TexSettings_ColorSpace = 0; // Linear
						m_TexSettings_WrapMode = 0;   // Repeat
						m_TexSettings_FilterMode = 0; // Nearest
						m_TexSettings_GenerateMipMap = true;
						m_TexSettings_CompressionMode = 0; // None (Uncompressed)

						m_ShowTextureSettingsPopup = true; // Open the popup
						m_TexSettings_TextureType = 0;  // 
						m_TexSettings_MaxSize = 0; //
						// --- ADD THESE LINES ---
						m_TexSettings_GenNormalMap = false; // Reset to false
						m_TexSettings_NormalMapStrength = 2.0f; // Reset to default

					}
					// --- END MODIFIED LOGIC ---

					else if (extension == ".fbx" || extension == ".obj" || extension == ".gltf")
					{
						m_Popup_SelectedMeshPath = entry.path();
						m_ShowMeshSettingsPopup = true; // Open the new popup
						m_MeshImportType = 0; // Reset to Static Mesh by default
					}

					else if (extension == ".ttf" || extension == ".otf")
					{
						AG::AssetCompiler::GetInstance().OpenFontImport(entry.path());
					}

					else if (extension == ".agmat")
					{
						auto mat_asset = AssetManager::GetInstance().GetAsset(HASH(entry.path().filename().generic_string()));
						if (mat_asset.lock())
						{
							if (auto mat_sp = std::dynamic_pointer_cast<MaterialTemplate>(mat_asset.lock()))
							{
								ObjInspector::InspectMaterial(mat_sp, entry.path().parent_path().generic_string());
							}
						}
					}
					else if (extension == ".agstaticmesh")
					{
						auto mesh_asset = AssetManager::GetInstance().GetAsset(HASH(entry.path().filename().generic_string()));
						if (mesh_asset.lock())
						{
							if (auto mat_sp = std::dynamic_pointer_cast<StaticMeshAsset>(mesh_asset.lock()))
							{
								ObjInspector::InspectMesh(mat_sp);
							}
						}
					}
					else if (extension == ".agskinnedmesh")
					{
						auto mesh_asset = AssetManager::GetInstance().GetAsset(HASH(entry.path().filename().generic_string()));
						if (mesh_asset.lock())
						{
							if (auto mat_sp = std::dynamic_pointer_cast<SkinnedMeshAsset>(mesh_asset.lock()))
							{
								ObjInspector::InspectSkinnedMesh(mat_sp);
							}
						}
					}

				}

				RightClickMenu(entry);
				//size_t content_hash = HASH(contentName);

				if (entry.path().extension() == ".bht")
				{
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
					{
						std::string path = entry.path().generic_string();
						ImGui::SetDragDropPayload("BHT_CONTENT", path.c_str(), path.size() + 1, ImGuiCond_Once);
						ImGui::TextUnformatted(contentName.c_str());

						ImGui::EndDragDropSource();
					}
				}
				else if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					const char* content = contentName.c_str();
					ImGui::SetDragDropPayload("IMGUI_CONTENT", content, contentName.size() * sizeof(char), ImGuiCond_Once);
					ImGui::TextUnformatted(contentName.c_str());
					//AG_CORE_INFO("[DragDrop] Started dragging item: '{}' with hash: {}", contentName, content_hash);

					ImGui::EndDragDropSource();
				}

			}
			ImGui::TextWrapped(contentName.c_str());
			ImGui::NextColumn();
		}
	}
	else /* --- Search Bar In Use --- */
	{
		std::string query = search_bar;
		std::transform(query.begin(), query.end(), query.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		for (const auto& entry : std::filesystem::recursive_directory_iterator(asset_file_path))
		{
			std::string contentName = entry.path().filename().string();


			std::string lowerName = contentName;
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			if (lowerName.find(query) == std::string::npos)
				continue;

			ImTextureID texid = GetIcon(entry);

			if (entry.is_directory()) /* --- Render File Icon --- */
			{
				if (ImGui::ImageButton(contentName.c_str(), texid, cellSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.f, 0.f, 0.f, 0.f)))
				{
					SelectDirectory(entry);
				}
			}
			else /* --- Render Content Icon --- */
			{
				
				ImVec2 uv0(0, 0), uv1(1, 1);
				auto type = AssetManager::GetInstance().GetFileType(entry);
				if (type == ASSET_TYPE::DDS || (type == ASSET_TYPE::TEXTURE && entry.path().extension() == ".dds")) {
					size_t h = AssetManager::GetInstance().hasher(contentName);
					auto& map = AssetManager::GetInstance().GetAssets(type);
					if (map.find(h) != map.end()) { uv0 = { 0, 1 }; uv1 = { 1, 0 }; }
				}

				ImGui::ImageButton(contentName.c_str(), texid, cellSize, uv0, uv1, ImVec4(0.f, 0.f, 0.f, 0.f));

                // --- DRAG DROP LOGIC MOVED HERE ---
                // Must act on the last item (ImageButton)
                std::string ext = entry.path().extension().string();
                std::string filename = entry.path().filename().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
					return static_cast<char>(std::tolower(c));
					});

                //AG_CORE_ERROR("[ContentBrowser] Checking: '{}' Ext: '{}'", filename, ext);

                if (ext == ".mpg")
                {
                    //AG_CORE_ERROR("[ContentBrowser] MPG Match! Starting Drag Source... (Step 1)");
                    // Use standard DragDropSource with no extra flags first to see if it works
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                       // AG_CORE_ERROR("[ContentBrowser] BeginDragDropSource SUCCESS (Step 2)");
                        std::string vidPath = entry.path().string();
                        ImGui::SetDragDropPayload("VIDEO_FILE", vidPath.c_str(), vidPath.size() + 1, ImGuiCond_Once);
                        ImGui::TextUnformatted(contentName.c_str());
                        ImGui::EndDragDropSource();
                    }
                    else {
                        // AG_CORE_WARN("[ContentBrowser] BeginDragDropSource Returned FALSE (Not Dragging)");
                    }
                }
                else if (ImGui::BeginDragDropSource())
                {
                    const char* content = contentName.c_str();
                    ImGui::SetDragDropPayload("IMGUI_CONTENT", content, contentName.size() * sizeof(char), ImGuiCond_Once);
					//AG_CORE_INFO("[DragDrop] Started dragging item: '{}' with hash: {}", contentName, HASH(contentName));
                    ImGui::EndDragDropSource();
                }
                // --- END DRAG DROP LOGIC ---

				// 2. NOW, check for a double-click on the item we just drew
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
				// --- START MODIFIED LOGIC ---
					auto asset_type = AssetManager::GetInstance().GetFileType(entry);
					std::string extension = entry.path().extension().string();

					// ONLY open the popup for source texture files (.png, .jpg, etc.)
					if (asset_type == ASSET_TYPE::TEXTURE)
					{
						// We clicked a source file. Use it as the source input.
						m_Popup_SelectedAssetPath = entry.path();
						m_Popup_SourceImagePath = entry.path().string();

						// Set default settings for a new import
						m_TexSettings_ColorSpace = 0; // Linear
						m_TexSettings_WrapMode = 0;   // Repeat
						m_TexSettings_FilterMode = 0; // Nearest
						m_TexSettings_GenerateMipMap = true;
						m_TexSettings_CompressionMode = 0; // None (Uncompressed)

						m_ShowTextureSettingsPopup = true; // Open the popup
						m_TexSettings_TextureType = 0;  // 
						m_TexSettings_MaxSize = 0; //
						
						m_TexSettings_GenNormalMap = false; // Reset to false
						m_TexSettings_NormalMapStrength = 2.0f; // Reset to default

					}
					else if (extension == ".fbx" || extension == ".obj" || extension == ".gltf")
					{
						m_Popup_SelectedMeshPath = entry.path();
						m_ShowMeshSettingsPopup = true; // Open the popup
						m_MeshImportType = 0; // Reset to Static Mesh
					}
					// --- END MODIFIED LOGIC ---
				}

				RightClickMenu(entry);
			}

			ImGui::TextWrapped(contentName.c_str());
			ImGui::NextColumn();
		}

	}

	if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
	{
		if (ImGui::MenuItem("New Cube Map"))
		{
			// This triggers the function we wrote in Step 1 & 2
			AG::AssetCompiler::GetInstance().OpenCubeMapImport();
		}
		if (ImGui::MenuItem("New Material"))
		{
			std::shared_ptr<MaterialTemplate> new_mat_temp = std::make_shared<MaterialTemplate>();
			new_mat_temp->material_name = "Empty_Material";
			ObjInspector::InspectMaterial(new_mat_temp, selected_directory.generic_string());
		}
		ImGui::EndPopup();
	}

}

bool AG::AGImGui::ContentBrowser::HasDirectory(std::filesystem::path path)
{
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_directory())
			return true;
	}
	return false;
}

void AG::AGImGui::ContentBrowser::SelectDirectory(std::filesystem::path path)
{
	previous_path.push(selected_directory);
	selected_directory = path;
}

ImTextureID AG::AGImGui::ContentBrowser::GetIcon(std::filesystem::directory_entry path)
{
	ImTextureID texid = 0;
	std::string ext = path.path().extension().string();
	auto guiIcon = [&](std::string ext) -> ImTextureID
		{
			if (ext.empty())
				ext = ".folder";
			auto it = imgui_icons.find(ext.substr(1));
			if (it != imgui_icons.end())
				return reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(it->second->textureID));
			else
				return reinterpret_cast <ImTextureID>(0);
		};
	

	texid = guiIcon(ext);
	if (texid != 0) return texid;

	auto asset_type = AssetManager::GetInstance().GetFileType(path);
	auto asset_map = AssetManager::GetInstance().GetAssets(asset_type);
	size_t asset_hashed = AssetManager::GetInstance().hasher(path.path().filename().string());

	if (asset_map.find(asset_hashed) == asset_map.end())
		return texid;


	auto tex = asset_map[asset_hashed];
	if (tex.lock()) // if asset exist
	{


		switch (asset_type)
		{
		case ASSET_TYPE::TEXTURE:
		{
			auto texAsset = std::dynamic_pointer_cast<TextureAsset>(tex.lock());
			if (texAsset)
			{
				texid = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texAsset->textureID));
			}
			break;
		}
		case ASSET_TYPE::DDS:
		{
			auto texAsset = std::dynamic_pointer_cast<TextureAsset>(tex.lock());
			if (texAsset)
			{
				texid = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texAsset->textureID));
			}
			break;
		}
		case ASSET_TYPE::AUDIO:
		{

			break;
		}
		case ASSET_TYPE::FONT:
		{
			auto fontAsset = std::dynamic_pointer_cast<FontAsset>(tex.lock());

			if (fontAsset)
			{
				texid = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(fontAsset->m_textureID));
			}
			else
			{
				AG_CORE_ERROR("Font asset cant lock");
			}
			break;
		}

		default: break;
		}
	}

	return texid;
}

ImTextureID AG::AGImGui::ContentBrowser::GetIcon(std::string path)
{
	if (imgui_icons.find(path) != imgui_icons.end())
		return reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(imgui_icons[path]->textureID));
	else
		return reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(0));
}

void AG::AGImGui::ContentBrowser::ViewLoadedAssets()
{

	/* --- ASSET LOADED LIST --- */
	ImGui::Begin("Assets Bank", &show_loaded_assets, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);


	/* --- --- */
	ImGui::TextWrapped("All assets in the final product will be loaded using our custom binary file format. For the prototyping phase, we will continue to support common asset file types (e.g., PNG, OBJ, TTF). The long-term goal is to provide in-editor tools that will bake assets into the AG asset format, ensuring faster load times, consistency, and tighter integration with the engine.");
	if (ImGui::Button("Unload All Assets"))
	{
		ASSET_MANAGER.UnloadAssets();
	}
	ImGui::Separator();

	for (auto enumPair : enumName)
	{
		if (ImGui::TreeNode(enumPair.second.c_str()))
		{
			size_t unload_id{};
			for (auto& obj : ASSET_MANAGER.GetAssets(enumPair.first))
			{
				if (!obj.second.lock()) continue;

				std::string content_name = obj.second.lock()->asset_name;
				content_name += " = " + std::to_string(obj.first);
				ImGui::TreeNodeEx(content_name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);

				if (ImGui::BeginPopupContextItem(content_name.c_str())) // use unique ID per item
				{
					if (ImGui::MenuItem("Unload"))
					{
						unload_id = obj.first;
					}
					ImGui::EndPopup();
				}
			}
			ImGui::TreePop();
			if (unload_id)
				ASSET_MANAGER.UnloadAsset(unload_id);
		}
	}
	ImGui::End();
}

void AG::AGImGui::ContentBrowser::RightClickMenu(std::filesystem::directory_entry item_directory)
{

	if (ImGui::BeginPopupContextItem())
	{
		std::string item_name = item_directory.path().filename().string();

		if (item_directory.path().extension() == ".agmat")
		{
			if (ImGui::MenuItem("Create Material Instance"))
			{
				std::string ref_mat = item_directory.path().filename().string();
				std::shared_ptr<MaterialTemplate> mat_inst = std::make_shared<MaterialTemplate>();
				mat_inst->reference_mat = HASH(ref_mat);
				mat_inst->material_name = item_directory.path().stem().string() + "_Inst";
				ObjInspector::InspectMaterial(mat_inst, item_directory.path().generic_string());
			}
		}

		if (ImGui::MenuItem("Cut"))
		{
			// TODO: Cut Feature
		}

		if (ImGui::MenuItem("Copy"))
		{
			// TODO: Copy Feature
		}

		if (ImGui::MenuItem("Paste"))
		{
			// TODO: Paste Feature
		}
		if (ImGui::MenuItem("Delete"))
		{
			std::filesystem::remove(item_directory.path());
		}
		if (!search_bar.empty())
		{
			if (ImGui::MenuItem("Go To File Location"))
			{
				SelectDirectory(item_directory.path().parent_path());
				search_bar.clear();
			}
		}
		if (ImGui::MenuItem("Convert to Binary"))
		{
			convert_to_binary = true;
			selected_asset = item_directory;

			/* --- output path set up --- */
			output_path_fs = item_directory.path();
			output_path = output_path_fs.string();
			output_path = output_path.substr(0, output_path.find_last_of("/\\"));

			/* --- output name set up --- */
			output_name = item_directory.path().filename().string().c_str();
			output_name = output_name.substr(0, output_name.find_first_of('.'));
		}
		if (ImGui::MenuItem("Generate Cube Map"))
		{
			cube_map = true;
		}

		ImGui::Separator();

		if (item_directory.path().extension() == ".prefab") {
			if (ImGui::MenuItem("Create Object from Prefab (To be fixed)")) {
				//PREFABMANAGER.InstantiatePrefabInstance(item_directory.path());
			}
		}
		ImGui::EndPopup();
	}
}



void AG::AGImGui::ContentBrowser::RenderTextureSettingsPopup()
{
	// ImGuiWindowFlags_AlwaysAutoResize will make the popup fit its content
	if (ImGui::BeginPopupModal("Texture Import Settings", &m_ShowTextureSettingsPopup, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string asset_name = m_Popup_SelectedAssetPath.filename().string();
		ImGui::Text("Import Settings for: %s", asset_name.c_str());
		ImGui::Separator();


		// --- START: ADDED TEXTURE TYPE DROPDOWN ---

		const char* textureTypes[] = { "Default", "Normal Map", "UI / Sprite" };
		if (ImGui::Combo("Texture Type", &m_TexSettings_TextureType, textureTypes, IM_ARRAYSIZE(textureTypes)))
		{
			// This 'if' block runs when the user changes the dropdown.
			// We now apply our presets.
			switch (m_TexSettings_TextureType)
			{
			case 0: // Default
				m_TexSettings_ColorSpace = 1;		// sRGB
				m_TexSettings_CompressionMode = 2;	// BC3
				m_TexSettings_GenerateMipMap = true;
				break;
			case 1: // Normal Map
				m_TexSettings_ColorSpace = 0;		// Linear
				m_TexSettings_CompressionMode = 3;	// BC5
				m_TexSettings_GenerateMipMap = true;
				break;
			case 2: // UI / Sprite
				m_TexSettings_ColorSpace = 1;		// sRGB
				m_TexSettings_CompressionMode = 2;	// BC3 (or 0 for uncompressed)
				m_TexSettings_GenerateMipMap = false; // No mips for UI
				break;
			}
		}

		ImGui::Separator();

		// --- START: ADDED NORMAL MAP GEN UI ---
		ImGui::Checkbox("Generate Normal Map from Grayscale", &m_TexSettings_GenNormalMap);

		if (m_TexSettings_GenNormalMap)
		{
			ImGui::Indent();
			ImGui::DragFloat("Bumpiness", &m_TexSettings_NormalMapStrength, 0.1f, 0.1f, 20.0f);

			// If we are generating a normal map, we PROBABLY want to use the "Normal Map" preset
			if (ImGui::Button("Apply Normal Map Preset"))
			{
				m_TexSettings_TextureType = 1; // Normal Map
				m_TexSettings_ColorSpace = 0;  // Linear
				m_TexSettings_CompressionMode = 3; // BC5
			}
			ImGui::Unindent();
		}
		ImGui::Separator();
		// --- END: ADDED NORMAL MAP GEN UI ---

		// --- END: ADDED TEXTURE TYPE DROPDOWN ---


		// --- START: ADDED MAX TEXTURE SIZE DROPDOWN ---

		const char* maxSizes[] = { "Original", "1024", "2048", "4096" };
		ImGui::Combo("Max Texture Size", &m_TexSettings_MaxSize, maxSizes, IM_ARRAYSIZE(maxSizes));

		ImGui::Separator();

		// --- END: ADDED MAX TEXTURE SIZE DROPDOWN ---


		// --- Source Image ---
		// We use InputText with ReadOnly flags.
		// A better implementation would be to add a "..." button to open a file dialog.
		ImGui::Text("Source Image");
		ImGui::SameLine();
		ImGui::InputText("##SourcePath", &m_Popup_SourceImagePath, ImGuiInputTextFlags_ReadOnly);

		if (m_Popup_SourceImagePath.empty())
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Click the source .png/.jpg to import,");
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "or select one to re-import this .dds.");
		}


		ImGui::SeparatorText("Color Space");
		ImGui::RadioButton("Linear##Color", &m_TexSettings_ColorSpace, 0); ImGui::SameLine();
		ImGui::RadioButton("sRGB", &m_TexSettings_ColorSpace, 1);

		ImGui::SeparatorText("Wrap Mode");
		ImGui::RadioButton("Repeat", &m_TexSettings_WrapMode, 0); ImGui::SameLine();
		ImGui::RadioButton("Clamp", &m_TexSettings_WrapMode, 1); ImGui::SameLine();
		ImGui::RadioButton("Mirror", &m_TexSettings_WrapMode, 2);

		ImGui::SeparatorText("Filter Mode");
		ImGui::RadioButton("Nearest", &m_TexSettings_FilterMode, 0); ImGui::SameLine();
		ImGui::RadioButton("Linear", &m_TexSettings_FilterMode, 1);

		ImGui::SeparatorText("MipMap");
		ImGui::Checkbox("Generate MipMap##gmm", &m_TexSettings_GenerateMipMap);

		ImGui::SeparatorText("Compression Format");
		ImGui::RadioButton("None (Uncompressed)", &m_TexSettings_CompressionMode, 0);
		ImGui::RadioButton("BC1 (DXT1)", &m_TexSettings_CompressionMode, 1);
		ImGui::RadioButton("BC3 (DXT5)", &m_TexSettings_CompressionMode, 2);
		ImGui::RadioButton("BC5 (Normal Map)", &m_TexSettings_CompressionMode, 3);
		ImGui::RadioButton("BC7 (High Quality)", &m_TexSettings_CompressionMode, 4);

		ImGui::Separator();

		// --- Apply / Cancel Buttons ---
		bool can_compile = !m_Popup_SourceImagePath.empty();
		if (!can_compile)
		{
			// Disable the button if no source image is selected
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Apply & Import"))
		{
			std::filesystem::path inputPath = m_Popup_SourceImagePath;
			std::filesystem::path outputPath = m_Popup_SelectedAssetPath;

			// If we clicked a .png, our output path should be .dds
			if (outputPath.extension() != ".dds")
			{
				outputPath.replace_extension(".dds");
			}

			TextureCompressionFormat format = static_cast<TextureCompressionFormat>(m_TexSettings_CompressionMode);

			// Use a temporary TextureAsset to call the conversion function
			// This matches the pattern from your AssetCompiler
			TextureAsset tempTex;
			bool success = tempTex.ConvertToDDS(
				inputPath,
				outputPath.string(),
				m_TexSettings_ColorSpace,
				m_TexSettings_WrapMode,
				m_TexSettings_FilterMode,
				m_TexSettings_GenerateMipMap,
				format,
				m_TexSettings_MaxSize,
				m_TexSettings_GenNormalMap,      // <--- ADD THIS
				m_TexSettings_NormalMapStrength
			);

			if (success)
			{
				// Reload the asset in the manager
				AG_CORE_WARN("Re-import successful: {}", outputPath.filename().string());

				std::shared_ptr<TextureAsset> reloaded_asset = std::make_shared<TextureAsset>();
				reloaded_asset->asset_name = outputPath.filename().string();
				reloaded_asset->LoadFromDDS(outputPath); // Load the new data

				size_t asset_hash = AssetManager::GetInstance().hasher(reloaded_asset->asset_name);

				// Unload the old one if it exists
				AssetManager::GetInstance().UnloadAsset(asset_hash);

				// Register the new one
				AssetManager::GetInstance().RegisterAsset(DDS, asset_hash, reloaded_asset);
			}
			else
			{
				AG_CORE_WARN("Re-import FAILED for: {}", outputPath.filename().string());
			}

			m_ShowTextureSettingsPopup = false;
			ImGui::CloseCurrentPopup();
		}

		if (!can_compile)
		{
			ImGui::EndDisabled();
			ImGui::PopStyleVar();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			m_ShowTextureSettingsPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}