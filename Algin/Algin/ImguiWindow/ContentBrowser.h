/*****************************************************************//**
 * \file   ContentBrowser.h
 * \brief  
 * 
 * \author Brandon
 * \date   September 2025
 *********************************************************************/
#pragma once
#include "../Header/pch.h"

#define ASSET_PATH "Assets/"
#define IMGUI_ICON_PATH "EditorAssets/"
namespace AG {
	namespace AGImGui {

		static std::vector<std::pair<ASSET_TYPE, std::string>> enumName =
		{
			{TEXTURE, "Texture"},
			{AGTEXTURE, "AGTexture"},
			{AUDIO, "Audio"},
			{AGAUDIO, "AGAudio"},
			{MODEL, "Model"},
			{AGCUBEMAP, "AGCubeMap" },
			{AGSTATICMESH, "AGStaticMesh" },
			{FONT, "Font" }
		};

		static std::map<ASSET_TYPE, std::string> outputExt =
		{
			{TEXTURE, ".agtex"},
			{AUDIO, ".agaudio"},
		};

		class ContentBrowser : public AGImGui::IImguiWindow, public Pattern::ISingleton<ContentBrowser>
		{
		public:
			ContentBrowser() :cb_index{ 0,0,0,0,0,0 }, asset_file_path { ASSET_PATH }, selected_directory{ asset_file_path }, imgui_icon_path{ IMGUI_ICON_PATH }
			{
				for (const auto& entry : std::filesystem::directory_iterator(imgui_icon_path))
				{
					std::string key = entry.path().filename().replace_extension("").string();
					auto tex_shared = std::make_shared<TextureAsset>();
					tex_shared->asset_name = entry.path().filename().string();
					tex_shared->Load(entry.path());
					imgui_icons[key] = tex_shared;
				}
			}

			~ContentBrowser() override;
			void Render() override;

			/* --- Content Browser Render --- */
			void RenderDirectories(const std::filesystem::path asset_directory);
			void RenderContent();

			/* --- Content Browser Helper --- */
			bool HasDirectory(std::filesystem::path path);
			void SelectDirectory(std::filesystem::path path);
			ImTextureID GetIcon(std::filesystem::directory_entry path);
			ImTextureID GetIcon(std::string path);

			void ViewLoadedAssets();

			void RightClickMenu(std::filesystem::directory_entry item_directory);

		private:
			/* --- DIRECTORIES AND CONTENTS --- */
			bool show_loaded_assets = false;
			unsigned int columns = 10;
			std::filesystem::path asset_file_path;
			std::filesystem::path selected_directory;
			std::stack< std::filesystem::path> previous_path;
			std::filesystem::directory_entry selected_asset;

			/* --- SEARCH BAR --- */
			std::string search_bar;

			/* --- EXPORTING BINARY --- */
			bool convert_to_binary = false;
			std::filesystem::path output_path_fs;
			std::string output_name;
			std::string output_path;
			bool cube_map = false;
			std::array<int, 6> cb_index;

			/* --- GUI ICONS --- */
			std::filesystem::path imgui_icon_path;
			std::unordered_map<std::string, std::shared_ptr<TextureAsset>> imgui_icons;

			bool m_ShowTextureSettingsPopup = false;

			std::filesystem::path m_Popup_SelectedAssetPath;

			std::string m_Popup_SourceImagePath;

			int m_TexSettings_ColorSpace=0;
			int m_TexSettings_WrapMode;
			int m_TexSettings_FilterMode=0;
			bool m_TexSettings_GenerateMipMap;

			int m_TexSettings_CompressionMode=0;

			int m_TexSettings_TextureType = 0;

			int m_TexSettings_MaxSize = 0;

			bool m_TexSettings_GenNormalMap = false;
			float m_TexSettings_NormalMapStrength = 2.0f; // Default bumpiness

			void RenderTextureSettingsPopup();

			bool m_ShowMeshSettingsPopup = false;
			std::filesystem::path m_Popup_SelectedMeshPath;
			int m_MeshImportType = 0; // 0 = Static, 1 = Dynamic

			// Function declaration
			void RenderMeshSettingsPopup();


		};
	}
}