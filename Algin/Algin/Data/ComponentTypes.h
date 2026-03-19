/*!*****************************************************************************
\file ComponentTypes.h
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief List of all the component types that is available
*******************************************************************************/
#pragma once

namespace AG {
	namespace Data {
		enum class ComponentTypes {
			None = 0,
			Transform,
			RectTransform,
			Camera,
			Texture,
			Light,
			Audio,
			MeshRenderer,
			SkinnedMeshRenderer,
			SkyBox,
			BoxCollider,
			PostProcessVolume,
			RigidBody,
			Image2DComponent,
			Script,
			Prefab,
			TextMeshUI,
			TextMesh,
			Particles,
			Behavior,
			Cube,
			Button,
			CapsuleCollider,
			VideoPlayer,
		};

		inline std::string ComponentTypeToString(ComponentTypes t) {
			switch (t) {
			case ComponentTypes::None:      return "None";
			case ComponentTypes::Transform: return "Transform";
			case ComponentTypes::RectTransform: return "RectTransform";
			case ComponentTypes::Texture:   return "Texture";
			case ComponentTypes::Camera:   return "Camera";
			case ComponentTypes::Light:   return "Light";
			case ComponentTypes::Audio:   return "Audio";
			case ComponentTypes::MeshRenderer:   return "Mesh Renderer";
			case ComponentTypes::SkyBox:   return "Sky Box";
			case ComponentTypes::BoxCollider:   return "Box Collider";
			case ComponentTypes::CapsuleCollider:   return "Capsule Collider";
			case ComponentTypes::PostProcessVolume:   return "Post Process Volume";
			case ComponentTypes::RigidBody:   return "Rigid Body";
			case ComponentTypes::Image2DComponent:   return "Image2D";
			case ComponentTypes::Script:   return "Script";
			case ComponentTypes::Prefab:   return "Prefab";
			case ComponentTypes::TextMeshUI:   return "TextMeshUI";
			case ComponentTypes::TextMesh:   return "TextMesh";
			case ComponentTypes::Particles:   return "Particles";
			case ComponentTypes::SkinnedMeshRenderer:   return "Skinned Mesh Renderer";
			case ComponentTypes::Behavior:   return "Behavior";
			case ComponentTypes::Cube:   return "Cube";
			case ComponentTypes::Button:   return "Button";
			case ComponentTypes::VideoPlayer: return "VideoPlayer";

			}
			return "Unknown";
		}
	}
}