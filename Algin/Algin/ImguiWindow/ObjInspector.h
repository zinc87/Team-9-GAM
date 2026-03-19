/*!*****************************************************************************
\file ObjInspector.h
\author Zulfami Ashrafi Bin Wakif
\date 20/1/2025 (MM/DD/YYYY)

\brief Imgui Inspector for the engine
*******************************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace AGImGui {
		static std::shared_ptr<System::IObject> G_selectedObj;

		class ObjInspector : public AGImGui::IImguiWindow {
		public:

		public:
			ObjInspector() : AGImGui::IImguiWindow({ 200.f, 50.f }, "Inspector") {}
			~ObjInspector() override;
			void Render() override;

			void MaterialInspector();
			static void InspectMaterial(std::shared_ptr<MaterialTemplate> ptr, std::string path)
			{
				m_editingMaterial = ptr;
				temp_mat_name = ptr->material_name;
				current_path = path;
			}

			static void CloseInspector()
			{
				m_editingMaterial.reset();
				m_editingMesh.reset();
				m_editingSkinned.reset();
			}

			void MeshInspector();
			static void InspectMesh(std::shared_ptr<StaticMeshAsset> ptr)
			{
				m_editingMesh = ptr;
			}

			void MeshInspectorSkinned();
			static void InspectSkinnedMesh(std::shared_ptr<SkinnedMeshAsset> ptr)
			{
				m_editingSkinned = ptr;
			}

		private:
			std::shared_ptr<System::IObject> m_renamingObject = nullptr;
			char m_renameBuffer[128] = {};
			bool m_renameJustStarted = false;
			inline static std::shared_ptr<MaterialTemplate> m_editingMaterial;
			inline static std::string temp_mat_name;
			inline static std::string current_path;

			inline static std::shared_ptr<StaticMeshAsset> m_editingMesh;
			inline static std::shared_ptr<SkinnedMeshAsset> m_editingSkinned;
		};
	}
}