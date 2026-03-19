#pragma once
#include "pch.h"
#include <array>
#include <future>
#include <vector>
namespace AG
{
	class AssetCompiler : public Pattern::ISingleton<AssetCompiler>
	{
	public:

		// 1. Accessor to get the global instance
		//static AssetCompiler& Get() { static AssetCompiler instance; return instance; }

		// 2. Function to trigger the compilation setup
		void OpenStaticMeshImport(const std::filesystem::path& path);

		void OpenSkinnedMeshImport(const std::filesystem::path& path);

		enum COMPILE_TYPE : int
		{
			NONE,
			TEXTURE,
			MESH,
			SKINNED,
			CUBEMAP,
			FONT,
			MATERIAL
		};


		void RenderGUI();
		void CompileAsset();
		bool IsEnabled() { return enable; }
		void Enable() {
			if (!enable)
			{
				Reset();
				enable = true;
			}
		}
		void Reset() { compile_type = NONE; selection_enable = false; }

		/* --- Different Compiler Menus --- */
		void CompileTextureGUI();
		void CompileStaticMeshGUI();
		void CompileCubeMapGUI();
		void CompileFontGUI();
		void CompileSkinnedGUI();
		void CompileMaterialGUI();

		bool CheckBoxRight(const char* label, bool* v)
		{
			// Draw label on the left
			std::string label_str = label;
			std::string label_name = label_str.substr(0, label_str.find_first_of('#'));

			// Same line, but move to far right
			float avail = ImGui::GetContentRegionAvail().x;
			ImGui::Text(label_name.c_str());
			ImGui::SameLine(ImGui::GetCursorPosX() + avail - ImGui::GetFrameHeight());

			// Render radio button without label (## hides it from UI)
			return ImGui::Checkbox(label_str.c_str(), v);
		}

		bool RadioButtonRight(const char* label, int* v, int v_button)
		{
			// Draw label on the left
			std::string label_str = label;
			std::string label_name = label_str.substr(0, label_str.find_first_of('#'));

			// Same line, but move to far right
			float avail = ImGui::GetContentRegionAvail().x;
			ImGui::Text(label_name.c_str());
			ImGui::SameLine(ImGui::GetCursorPosX() + avail - ImGui::GetFrameHeight());

			// Render radio button without label (## hides it from UI)
			return ImGui::RadioButton(label_str.c_str(), v, v_button);
		}

		void OpenCubeMapImport();

		// [NEW] Add this declaration
		void OpenFontImport(const std::filesystem::path& path);

	private:

		// 3. Move these variables here (they were previously static inside the function)
		std::string m_TargetMeshStr;
		std::string m_OutFileStr;
		std::string m_OutLocStr;

		// Metadata cache
		int m_MetaVert = 0;
		int m_MetaSubmesh = 0;
		std::vector<std::string> m_MetaMaterials;

		// Helper to trigger the meta-data loading logic
		bool m_TriggerMeshLoad = false;

		std::vector<std::future<void>> m_compileFutures;
		bool enable = false;
		bool selection_mode = true;
		bool readytocompile = false;
		COMPILE_TYPE compile_type{};
		bool selection_changed = false;
		bool selection_enable = false;

		std::string selectedFile{};
		std::string targetedString{};


		/* --- Texture Settings --- */
		int colorSpace{}, wrapMode{}, filterMode{};
		bool mipmap = true;

		/* --- Cube Map Settings --- */
		std::array<int, 6> cb_index{};

		float m_BuildScale[3] = { 1.0f, 1.0f, 1.0f };
		float m_BuildRotation[3] = { 0.0f, 0.0f, 0.0f };


		std::string m_CM_Right;
		std::string m_CM_Left;
		std::string m_CM_Down;
		std::string m_CM_Top;
		std::string m_CM_Front;
		std::string m_CM_Back;
		std::string m_CM_OutFile;
		std::string m_CM_OutLoc;

		// [NEW] Add these member variables for Font state
		std::string m_FontInputPath;
		std::string m_FontOutputName;
		std::string m_FontOutputDir;

	};


}
