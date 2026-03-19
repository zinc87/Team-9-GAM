#pragma once
#include "pch.h"
#include "../Graphics/MaterialAsset.h"
#include "../Graphics/Scan.h"


namespace AG {
	namespace Component {
		class SkinnedMeshRenderer : public IComponent {
		public:
			using Self = SkinnedMeshRenderer;

			// --- ADD THIS STATIC FUNCTION ---
			static Type GetStaticType() { return Data::ComponentTypes::SkinnedMeshRenderer; }

			SkinnedMeshRenderer() :
				IComponent(Data::ComponentTypes::SkinnedMeshRenderer) {
			}

			SkinnedMeshRenderer(ID id, ID objId) :
				IComponent(Data::ComponentTypes::SkinnedMeshRenderer, id, objId) {
			}

			// Inherited via IComponent
			void Awake() override;

			void Start() override;

			void Update() override;

			void LateUpdate() override;

			void Free() override;

			void Inspector() override;

			//virtual std::shared_ptr<IComponent> Clone() override;
			void AssignFrom([[maybe_unused]] const std::shared_ptr<IComponent>& fromCmp) override;

			void DrawMesh(SkinnedMeshAsset& mesh, TransformComponent& trf);
			void DrawForObjPick(const SkinnedMeshAsset& mesh, TransformComponent& trf);
			void DrawShadows(const SkinnedMeshAsset& mesh, TransformComponent& trf);

			/* === FOR C# CHANGING === */
			void ChangeModel(std::string model_name);
			void ChangeShaderExcept(std::string shader_name, std::string submesh_name);
			void PlayAnimation(std::string animation_name) { animation_clip_name = animation_name; play = true; }
			void SetLoopAnimation(bool should_loop) { loop = should_loop; }
			void playCurrentAnimation() { play = true; }		// 31/1
			void stopAnimation() { play = false; }		// 31/1
			void ResetTimer() { current_time = 0.f; }	// 31/1
			void SetTimer(float time) { current_time = time; } // 31/1
			void SetSubmeshMaterial(std::string submesh_name, std::string material_name);

			/* === FOR C# CHANGING === */

			std::string GetTypeName() const override { return "AG::Component::SkinnedMeshRenderer"; }

			REFLECT() {
				return {
					REFLECT_FIELD(skinned_mesh_hash, size_t, "Mesh Hash"),
					REFLECT_FIELD(animation_clip_name, std::string, "Animation Clip Name"),
					REFLECT_FIELD(loop, bool, "Loop"),
					REFLECT_FIELD(play, bool, "Play"),
					REFLECT_FIELD(use_correction, bool, "Use Correction"),
					REFLECT_FIELD(shininess, float, "Shininess"),
					REFLECT_FIELD(material_hash_list, std::vector<size_t>, "Material Hash List"),
					REFLECT_FIELD(material_value_list, std::vector<glm::vec3>, "Material Values")
				};
			}

			REFLECT_SERIALIZABLE(SkinnedMeshRenderer)
			
			size_t skinned_mesh_hash = 0;
			std::string animation_clip_name = "";
			bool loop = false;
			bool play = false;
			std::vector<size_t>		material_hash_list;
			std::vector<glm::vec3>	material_value_list; // used cased of no texture

		private:
            float current_time = 0.f;                         
            std::vector<glm::mat4> boneTransforms;
			
			

			//int MAX_BONES_SHADER = 256;

			
			bool use_correction = false;
			float shininess = 32.0f;

			//temp
			int submesh_selected_index = -1;

			static glm::vec3 InterpolateTranslation(const BoneAnimation& boneAnim, float time);
			static glm::quat InterpolateRotation(const BoneAnimation& boneAnim, float time);
            static glm::vec3 InterpolateScale(const BoneAnimation& boneAnim, float time);
			

			//SCAN DATA
			ScanParams SP;
			//bool scanning = false;
			glm::mat4 viewProj = glm::mat4();
			float aspect = 0.f;

			// animation intepolation
			struct TransitionState {
				std::string old_clip_name = "";
				float old_time = 0.0f;
				float fade_time = 0.0f;
				float fade_duration = 0.2f; // Adjust this for faster/slower fades
				bool active = false;
			};
			TransitionState transition;
			std::string last_clip_name = ""; // To detect when PlayAnimation is called

			glm::mat4 SampleBone(const AnimationClip* clip, const std::string& boneName, float time);
			glm::mat4 MixMatrices(const glm::mat4& a, const glm::mat4& b, float t);
			const AnimationClip* FindClip(const std::shared_ptr<SkinnedMeshAsset>& mesh, const std::string& name);
		};
	}


}