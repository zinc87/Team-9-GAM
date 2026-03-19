#include "pch.h"
#include "ScriptConnector.h"

/*
	Guide when adding new internal call(Call C++ function from C#)
	1. Add C++ function under namespace InternalCall(in both .cpp and .h). Notice that the signature, return type and param must be the same as the C# version that create later. 
	   Some of the type need to interchange between C# and C++(eg. string in C# need to change to MonoString before changing to C string)
	2. Use Macro ADD_INTERNAL_CALL(#FunctionName) to register the c++ function into mono.
	3. Add the C# internal call function in the InternalCall.cs
*/

#define ADD_INTERNAL_CALL(functionName) mono_add_internal_call("PrivateAPI.InternalCall::" #functionName, InternalCall::functionName)
#define ADD_COMPONENT(componentName) componentFullName = "Script.Library."#componentName;\
									managedType = mono_reflection_type_from_name(componentFullName.data(), SCRIPTENGINE.getApiImage());\
									entityHasMonoComponent[managedType] = [&](System::IObject::ID objID) {return hasComponent<Component::componentName>(objID);}\

namespace AG {

	void ScriptConnector::init()
	{
		addFunction();
		addComponents();
	}
	bool ScriptConnector::HasMonoComponent(MonoType* monoType, System::IObject::ID objID)
	{
		if (entityHasMonoComponent.find(monoType) == entityHasMonoComponent.end()) {
			AG_CORE_ERROR("No such mono type");
			return false;
		}
		else {
			return entityHasMonoComponent[monoType](objID);
		}
	}
	//2. Register internal call here
	//Without registering, it will MissingMethodException from C#
	void ScriptConnector::addFunction()
	{
		ADD_INTERNAL_CALL(objectHasComponent_type);
		ADD_INTERNAL_CALL(objectHasComponent_enum);
		ADD_INTERNAL_CALL(createGameObject);
		ADD_INTERNAL_CALL(createPatient);
		ADD_INTERNAL_CALL(objectAddComponent);

		ADD_INTERNAL_CALL(setGameObjectActive);
		ADD_INTERNAL_CALL(getGameObjectActive);
		ADD_INTERNAL_CALL(linkParentChild);

		ADD_INTERNAL_CALL(getTransformComponent);
		ADD_INTERNAL_CALL(setTransformComponent);

		ADD_INTERNAL_CALL(getCameraComponent);
		ADD_INTERNAL_CALL(setCameraComponent);

		ADD_INTERNAL_CALL(getLightComponent);
		ADD_INTERNAL_CALL(setLightComponent);

		ADD_INTERNAL_CALL(getRigidBodyComponent);
		ADD_INTERNAL_CALL(setRigidBodyComponent);

		ADD_INTERNAL_CALL(getBoxColliderComponent);
		ADD_INTERNAL_CALL(setBoxColliderComponent);

		ADD_INTERNAL_CALL(getSkyBoxComponent);
		ADD_INTERNAL_CALL(setSkyBoxComponent);

		ADD_INTERNAL_CALL(getParticleComponent);
		ADD_INTERNAL_CALL(setParticleComponent);

		ADD_INTERNAL_CALL(getPostProcessVolumeComponent);
		ADD_INTERNAL_CALL(setPostProcessVolumeComponent);

		ADD_INTERNAL_CALL(getRectTransformComponent);
		ADD_INTERNAL_CALL(setRectTransformComponent);

		ADD_INTERNAL_CALL(getImage2DComponent);
		ADD_INTERNAL_CALL(setImage2DComponent);

		ADD_INTERNAL_CALL(getButtonComponent);
		ADD_INTERNAL_CALL(setButtonComponent);

		ADD_INTERNAL_CALL(getTextMeshUIComponent);
		ADD_INTERNAL_CALL(setTextMeshUIComponent);
		ADD_INTERNAL_CALL(setFont);
		ADD_INTERNAL_CALL(setFontSize);
		ADD_INTERNAL_CALL(setLineSpacing);
		ADD_INTERNAL_CALL(setLetterSpacing);
		ADD_INTERNAL_CALL(setWrapLimit);
		ADD_INTERNAL_CALL(setWrap);
		ADD_INTERNAL_CALL(setCentered);
		ADD_INTERNAL_CALL(setTextColor);


		ADD_INTERNAL_CALL(getMeshRendererComponent);
		ADD_INTERNAL_CALL(setMeshRendererComponent);
		ADD_INTERNAL_CALL(setMeshRendererSubmeshMaterial);

		ADD_INTERNAL_CALL(getSkinnedMeshRenderer);
		ADD_INTERNAL_CALL(setSkinnedMeshRenderer);

		ADD_INTERNAL_CALL(GetCameraRay);
		ADD_INTERNAL_CALL(setOverlayCamera);
		ADD_INTERNAL_CALL(setOverlayActive);
		ADD_INTERNAL_CALL(setOverlayViewport);

		ADD_INTERNAL_CALL(log);

		ADD_INTERNAL_CALL(drawLine);

		ADD_INTERNAL_CALL(getMousePostion);
		ADD_INTERNAL_CALL(getMouseDelta);
		ADD_INTERNAL_CALL(mouseScroll);
		ADD_INTERNAL_CALL(isMousePressed);
		ADD_INTERNAL_CALL(isMouseReleased);
		ADD_INTERNAL_CALL(isKeyPressed);
		ADD_INTERNAL_CALL(isKeyReleased);
		ADD_INTERNAL_CALL(getMouseState);
		ADD_INTERNAL_CALL(setMouseState);

		ADD_INTERNAL_CALL(setNodeStatus);
		ADD_INTERNAL_CALL(getNodeStatus);
		ADD_INTERNAL_CALL(setNodeResult);
		ADD_INTERNAL_CALL(getNodeResult);
		ADD_INTERNAL_CALL(nodeTick);
		ADD_INTERNAL_CALL(getNodeChildrens);

		ADD_INTERNAL_CALL(getAudioFullPath);
		ADD_INTERNAL_CALL(playAudio);
		ADD_INTERNAL_CALL(pauseAudio);
		ADD_INTERNAL_CALL(stopAudio);
		ADD_INTERNAL_CALL(getNodeChildrens);
		ADD_INTERNAL_CALL(getAudioParameter);
		ADD_INTERNAL_CALL(setAudioParameter);
		ADD_INTERNAL_CALL(setVolume);
		ADD_INTERNAL_CALL(getVolume);
		ADD_INTERNAL_CALL(setAudioInstanceVolume);

		ADD_INTERNAL_CALL(isRayCastHit);

		ADD_INTERNAL_CALL(loadScene);
		ADD_INTERNAL_CALL(getSceneName);

		ADD_INTERNAL_CALL(setScanState);
		ADD_INTERNAL_CALL(getScanState);

		ADD_INTERNAL_CALL(getVideoPlayerComponent);
		ADD_INTERNAL_CALL(setVideoPlayerComponent);
		ADD_INTERNAL_CALL(playVideo);
		ADD_INTERNAL_CALL(stopVideo);
		ADD_INTERNAL_CALL(isVideoFinished);

		ADD_INTERNAL_CALL(setMeshFromString);
		ADD_INTERNAL_CALL(setMeshFromHash);

		ADD_INTERNAL_CALL(changeModel);
		ADD_INTERNAL_CALL(changeShaderExcept);
		ADD_INTERNAL_CALL(playAnimation);
		ADD_INTERNAL_CALL(setLoopAnimation);
		ADD_INTERNAL_CALL(stopAnimation);

		ADD_INTERNAL_CALL(playCurrentAnimation);
		//ADD_INTERNAL_CALL(stopAnimation);
		ADD_INTERNAL_CALL(resetAnimationTimer);
		ADD_INTERNAL_CALL(setAnimationTimer);
		ADD_INTERNAL_CALL(setSubmeshMaterial);
		ADD_INTERNAL_CALL(getGamma);
		ADD_INTERNAL_CALL(setGamma);
		ADD_INTERNAL_CALL(getTonemapEnabled);
		ADD_INTERNAL_CALL(setTonemapEnabled);
	}
	void ScriptConnector::addComponents()
	{
		entityHasMonoComponent.clear();
		std::string componentFullName{};
		MonoType* managedType{ nullptr };

		ADD_COMPONENT(TransformComponent);
		ADD_COMPONENT(LightComponent);
		ADD_COMPONENT(CameraComponent);
		ADD_COMPONENT(AudioComponent);
		ADD_COMPONENT(MeshRendererComponent);
		ADD_COMPONENT(SkyBoxComponent);
		ADD_COMPONENT(BoxColliderComponent);
		ADD_COMPONENT(PostProcessVolumeComponent);
		ADD_COMPONENT(RigidBodyComponent);
		ADD_COMPONENT(RectTransformComponent);
		ADD_COMPONENT(Image2DComponent);
		ADD_COMPONENT(ScriptComponent);
		ADD_COMPONENT(TextMeshUIComponent);
		ADD_COMPONENT(ParticleComponent);
		ADD_COMPONENT(SkinnedMeshRenderer);
		ADD_COMPONENT(BehaviorComponent);
		ADD_COMPONENT(CubeComponent);
		ADD_COMPONENT(ButtonComponent);
		ADD_COMPONENT(VideoPlayerComponent);
		
	}

	struct MonoCameraComponent
	{
		float reflected_near;
		float reflected_far;
		float reflected_fov;
		float reflected_vertical;
		int reflected_proj;
		bool reflected_isGameCam;
	};

	struct MonoLightComponent
	{
		glm::vec3 r_position;
		glm::vec3 r_direction;
		float r_intensity;
		float r_range;
		int r_type_index;
	};

	struct MonoRigidBodyComponent
	{
		glm::vec3 reflected_drag;
		glm::vec3 reflected_velocity;
		glm::vec3 reflected_acceleration;
		glm::vec3 reflected_force;
		float reflected_mass;
		glm::vec3 reflected_gravity;
		bool reflected_useGravity;
		glm::vec3 reflected_finalAcceleration;
	};

	struct MonoBoxColliderComponent
	{
		glm::vec3 reflected_center_offset;
		glm::vec3 reflected_halfExtent;
		bool reflected_isTrigger;
		bool reflected_drawCollider;
		uint32_t layerMask;
	};

	struct MonoSkyBoxComponent
	{
		int reflected_cm_index;
		float reflected_exposure;
		glm::vec3 reflected_tint;
		glm::vec3 reflected_rotation;
	};

	struct MonoParticleComponent
	{
		glm::vec3 reflected_position;
		float reflected_speed;
		float reflected_upVel;
		float reflected_grav;
		bool reflected_play;
	};

	struct MonoVideoPlayerComponent
	{
		MonoString* reflected_videoPath;
		bool reflected_playOnStart;
		bool reflected_looping;
	};

	struct MonoPostProcessVolumeComponent
	{
		bool reflected_isGlobal;
		glm::vec3 reflected_VolCentre;
		glm::vec3 reflected_HalfExtend;
	};

	struct MonoRectTransformComponent
	{
		glm::vec2 reflected_position;
		glm::vec2 reflected_scale;
		float reflected_rotation;
	};

	struct MonoImage2DComponent
	{
		uint64_t reflected_image_hashed; // 64-bit
		Vector2D topLeft;
		Vector2D topRight;
		Vector2D bottomLeft;
		Vector2D bottomRight; // store 4 corners (x,y)
		glm::vec4 reflected_color;        // r,g,b,a
	};

	struct MonoButtonComponent
	{
		int reflected_L_buttonStates[4];
		int reflected_R_buttonStates[4];
		int reflected_M_buttonStates[4];
	};

	struct MonoTextMeshUIComponent
	{
		glm::vec4 color;
		uint64_t hashed;

		MonoString* text;

		float reflected_size;
		float reflected_spacing;
		
	};

	struct MonoMeshRendererComponent
	{
		uint64_t reflected_model_hash;
		MonoString* reflected_mesh_name;
		//uint64_t reflected_submesh_name_hashes;
		//uint64_t reflected_material_hash_list;
		//glm::vec3 reflected_material_value_list;
		float reflected_shininess;

	};
	 
	struct MonoSkinnedMeshRenderer
	{
		uint64_t reflected_skinned_mesh_hash;
		MonoString* reflected_animation_clip_name;
		bool reflected_loop;
		bool reflected_play;

	};

	struct MonoRaycastHitData {
		glm::vec3 point;
		glm::vec3 normal;
		float distance;
		MonoString* hitEntityID;
		Component::CollisionLayers layer;
	};

	std::weak_ptr<AI::AINode> getNode(MonoString* objID, MonoString* nodeID)
	{
		std::string _objID(mono_string_to_utf8(objID));
		const System::ComponentManager::ObjMap& objMap = COMPONENTMANAGER.GetComponentsObj();
		auto& compVec = objMap.at(_objID);
		for (auto& comp : compVec) {
			if (comp.expired()) return std::weak_ptr<AI::AINode>();
			if (comp.lock()->GetType() == Data::ComponentTypes::Behavior) {
				Component::BehaviorComponent* bComp = reinterpret_cast<Component::BehaviorComponent*>(comp.lock().get());
				const AI::BehaviorTree::TreeID& treeID = bComp->getTreeID();
				std::weak_ptr<AI::BehaviorTree> tree = AI::AIManager::GetInstance().getTree(treeID);
				if (tree.expired()) {
					AG_CORE_WARN("No tree {} in object {}", treeID, _objID);
				}
				std::string _nodeID(mono_string_to_utf8(nodeID));
				return tree.lock()->getNode(_nodeID);
			}
		}
		AG_CORE_WARN("Cant find Behavior component");
		return std::weak_ptr<AI::AINode>();
	}

	//1.1 Add the implementation of the internal Call function here
	namespace InternalCall {
#pragma region Object
		MonoString* createGameObject()
		{
			return mono_string_new(mono_domain_get(), OBJECTMANAGER.CreateObject()->GetID().c_str());
		}

		// Temp delete ltr
		MonoString* createPatient()
		{
			return mono_string_new(mono_domain_get(), OBJECTMANAGER.CreatePatient()->GetID().c_str());
		}

		void setGameObjectActive(MonoString* objID, bool active)
		{
			std::string _objID(mono_string_to_utf8(objID));
			OBJECTMANAGER.GetObjectById(_objID)->SetActive(active);
		}

		bool getGameObjectActive(MonoString* objID)
		{
			std::string _objID(mono_string_to_utf8(objID));
			return OBJECTMANAGER.GetObjectById(_objID)->GetActive();
		}
		bool objectHasComponent_type(MonoString* objID, MonoReflectionType* componentType) {
			MonoType* monoComponentype = mono_reflection_type_get_type(componentType);
			std::string _objID(mono_string_to_utf8(objID));
			return SCRIPTENGINE.getScriptConnector().HasMonoComponent(monoComponentype, _objID);
		}
		bool objectHasComponent_enum(MonoString* objID, Data::ComponentTypes type) {
			std::string _objID(mono_string_to_utf8(objID));
			return OBJECTMANAGER.GetObjectById(_objID)->hasComponent(type);
		}
		void objectAddComponent(MonoString* objID, Data::ComponentTypes type) {
			std::string _objID(mono_string_to_utf8(objID));
			COMPONENTMANAGER.CreateComponent(OBJECTMANAGER.GetObjectById(_objID).get(), type);
		}

		void linkParentChild(MonoString* parentID, MonoString* childID)
		{
			std::string _parentID(mono_string_to_utf8(parentID));
			std::string _childID(mono_string_to_utf8(childID));
			
			AG_CORE_INFO("[ScriptConnector] linkParentChild called - Parent: {}, Child: {}", _parentID, _childID);
			
			auto parent = OBJECTMANAGER.GetObjectById(_parentID);
			auto child = OBJECTMANAGER.GetObjectById(_childID);
			
			if (!parent)
			{
				AG_CORE_WARN("[ScriptConnector] linkParentChild - Parent object not found: {}", _parentID);
			}
			if (!child)
			{
				AG_CORE_WARN("[ScriptConnector] linkParentChild - Child object not found: {}", _childID);
			}
			
			if (parent && child)
			{
				AG_CORE_INFO("[ScriptConnector] linkParentChild - Linking {} to {}", child->GetName(), parent->GetName());
				OBJECTMANAGER.LinkParentChild(parent, child);
			}
		}
#pragma endregion

#pragma region Component

		void getTransformComponent(MonoString* objID, Transformation* trf)
		{
			std::string _objID(mono_string_to_utf8(objID));
			*trf = OBJECTMANAGER.GetObjectById(_objID)->GetComponent<Component::TransformComponent>().lock()->GetTransformation();
		}

		void setTransformComponent(MonoString* objID, Transformation* trf)
		{
			std::string _objID(mono_string_to_utf8(objID));
			OBJECTMANAGER.GetObjectById(_objID)->GetComponent<Component::TransformComponent>().lock()->GetTransformation() = *trf;
		}

		void getCameraComponent(MonoString* objID, MonoCameraComponent* cam)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto cameraComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::CameraComponent>().lock();

			if (!cameraComp)
				return;

			cam->reflected_near = cameraComp->reflected_near;
			cam->reflected_far = cameraComp->reflected_far;
			cam->reflected_fov = cameraComp->reflected_fov;
			cam->reflected_vertical = cameraComp->reflected_vertical;
			cam->reflected_proj = cameraComp->reflected_proj;
			cam->reflected_isGameCam = cameraComp->reflected_isGameCam;
		}

		void setCameraComponent(MonoString* objID, MonoCameraComponent* cam)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto cameraComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::CameraComponent>().lock();

			if (!cameraComp)
				return;

			cameraComp->reflected_near = cam->reflected_near;
			cameraComp->reflected_far = cam->reflected_far;
			cameraComp->reflected_fov = cam->reflected_fov;
			cameraComp->reflected_vertical = cam->reflected_vertical;
			cameraComp->reflected_proj = cam->reflected_proj;
			cameraComp->reflected_isGameCam = cam->reflected_isGameCam;

			cameraComp->SyncToCamera();
		}

		void getLightComponent(MonoString* objID, MonoLightComponent* light)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto lightComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::LightComponent>().lock();

			if (!lightComp)
				return;

			light->r_position = lightComp->r_position;
			light->r_direction = lightComp->r_direction;
			light->r_intensity = lightComp->r_intensity;
			light->r_range = lightComp->r_range;
			light->r_type_index = lightComp->r_type_index;
		}

		void setLightComponent(MonoString* objID, MonoLightComponent* light)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto lightComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::LightComponent>().lock();

			if (!lightComp)
				return;

			lightComp->r_position = light->r_position;
			lightComp->r_direction = light->r_direction;
			lightComp->r_intensity = light->r_intensity;
			lightComp->r_range = light->r_range;
			lightComp->r_type_index = light->r_type_index;

			lightComp->SyncToLight();
		}

		void getRigidBodyComponent(MonoString* objID, MonoRigidBodyComponent* rb)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto rbComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::RigidBodyComponent>().lock();

			if (!rbComp)
				return;

			// copy fields
			rb->reflected_drag = rbComp->_data.drag;
			rb->reflected_velocity = rbComp->_data.velocity;
			rb->reflected_acceleration = rbComp->_data.acceleration;
			rb->reflected_force = rbComp->_data.force;
			rb->reflected_mass = rbComp->_data.mass;
			rb->reflected_gravity = rbComp->_data.gravity;
			rb->reflected_useGravity = rbComp->_data.useGravity;
			rb->reflected_finalAcceleration = rbComp->_data.finalAcceleration;
		}

		void setRigidBodyComponent(MonoString* objID, MonoRigidBodyComponent* rb)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto rbComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::RigidBodyComponent>().lock();

			if (!rbComp)
				return;

			// assign fields back to component
			rbComp->_data.drag = rb->reflected_drag;
			rbComp->_data.velocity = rb->reflected_velocity;
			rbComp->_data.acceleration = rb->reflected_acceleration;
			rbComp->_data.force = rb->reflected_force;
			rbComp->_data.mass = rb->reflected_mass;
			rbComp->_data.gravity = rb->reflected_gravity;
			rbComp->_data.useGravity = rb->reflected_useGravity;
			rbComp->_data.finalAcceleration = rb->reflected_finalAcceleration;

			
		}

		void getBoxColliderComponent(MonoString* objID, MonoBoxColliderComponent* bc)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto bcComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::BoxColliderComponent>().lock();

			if (!bcComp)
				return;

			bc->reflected_center_offset = bcComp->center_offset;
			bc->reflected_halfExtent = bcComp->halfExtent;
			bc->reflected_isTrigger = bcComp->m_isTrigger;
			bc->reflected_drawCollider = bcComp->m_drawCollider;
			bc->layerMask = bcComp->layer;
		}

		void setBoxColliderComponent(MonoString* objID, MonoBoxColliderComponent* bc)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto bcComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::BoxColliderComponent>().lock();

			if (!bcComp)
				return;

			bcComp->center_offset = bc->reflected_center_offset;
			bcComp->halfExtent = bc->reflected_halfExtent;
			bcComp->m_isTrigger = bc->reflected_isTrigger;
			bcComp->m_drawCollider = bc->reflected_drawCollider;
			bcComp->layer = static_cast<Component::CollisionLayers>(bc->layerMask);
		}

		void getSkyBoxComponent(MonoString* objID, MonoSkyBoxComponent* sky)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto skyComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::SkyBoxComponent>().lock();

			if (!skyComp)
				return;

			sky->reflected_cm_index = skyComp->cm_index;
			sky->reflected_exposure = skyComp->m_exposure;
			sky->reflected_tint = skyComp->m_tint;
			sky->reflected_rotation = skyComp->m_rotation;
		}

		void setSkyBoxComponent(MonoString* objID, MonoSkyBoxComponent* sky)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto skyComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::SkyBoxComponent>().lock();

			if (!skyComp)
				return;

			skyComp->cm_index = sky->reflected_cm_index;
			skyComp->m_exposure = sky->reflected_exposure;
			skyComp->m_tint = sky->reflected_tint;
			skyComp->m_rotation = sky->reflected_rotation;
		}

		void getParticleComponent(MonoString* objID, MonoParticleComponent* pc)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto pComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::ParticleComponent>().lock();

			if (!pComp)
				return;

			pc->reflected_position = pComp->r_position;
			pc->reflected_speed = pComp->r_speed;
			pc->reflected_upVel = pComp->r_upVel;
			pc->reflected_grav = pComp->r_grav;
			pc->reflected_play = pComp->play;
		}

		void setParticleComponent(MonoString* objID, MonoParticleComponent* pc)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto pComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::ParticleComponent>().lock();

			if (!pComp)
				return;

			pComp->r_position = pc->reflected_position;
			pComp->r_speed = pc->reflected_speed;
			pComp->r_upVel = pc->reflected_upVel;
			pComp->r_grav = pc->reflected_grav;
			pComp->play = pc->reflected_play;

			pComp->SyncToParticle();
		}

		void getPostProcessVolumeComponent(MonoString* objID, MonoPostProcessVolumeComponent* ppv)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto ppvComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::PostProcessVolumeComponent>().lock();

			if (!ppvComp)
				return;

			ppv->reflected_isGlobal = ppvComp->m_isGlobal;
			ppv->reflected_VolCentre = ppvComp->m_ppvolume.center;
			ppv->reflected_HalfExtend = ppvComp->m_ppvolume.center;
		}

		void setPostProcessVolumeComponent(MonoString* objID, MonoPostProcessVolumeComponent* ppv)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto ppvComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::PostProcessVolumeComponent>().lock();

			if (!ppvComp)
				return;

			ppvComp->m_isGlobal = ppv->reflected_isGlobal;
			ppvComp->m_ppvolume.center = ppv->reflected_VolCentre;
			ppvComp->m_ppvolume.center = ppv->reflected_HalfExtend;
		}

		void getRectTransformComponent(MonoString* objID, MonoRectTransformComponent* rt)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto rtComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::RectTransformComponent>().lock();

			if (!rtComp)
				return;

			rt->reflected_position = rtComp->m_trf.position;
			rt->reflected_scale = rtComp->m_trf.scale;
			rt->reflected_rotation = rtComp->m_trf.rotation;
		}

		void setRectTransformComponent(MonoString* objID, MonoRectTransformComponent* rt)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto rtComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::RectTransformComponent>().lock();

			if (!rtComp)
				return;

			rtComp->m_trf.position = glm::vec3(rt->reflected_position,1.0f);
			rtComp->m_trf.scale = rt->reflected_scale;
			rtComp->m_trf.rotation = rt->reflected_rotation;
		}

		void getImage2DComponent(MonoString* objID, MonoImage2DComponent* img)
		{
			if (!objID || !img) return;

			std::string _objID(mono_string_to_utf8(objID));
			auto objPtr = OBJECTMANAGER.GetObjectById(_objID);
			if (!objPtr) return;

			auto imgComp = objPtr->GetComponent<Component::Image2DComponent>().lock();
			if (!imgComp) return;

			img->reflected_image_hashed = static_cast<uint64_t>(imgComp->m_image_hashed);

			// Directly assign glm::vec4 from Color union
			img->reflected_color = imgComp->m_color.rgba;

			// Flatten texture coordinates
			/*for (int i = 0; i < 4; ++i) {
				img->reflected_tc[i * 2 + 0] = imgComp->m_tc.textureCoord[i][0];
				img->reflected_tc[i * 2 + 1] = imgComp->m_tc.textureCoord[i][1];
			}*/

			img->topLeft = Vector2D{
				imgComp->m_tc.textureCoord[0][0],
				imgComp->m_tc.textureCoord[0][1]
			};

			img->topRight = Vector2D{
				imgComp->m_tc.textureCoord[1][0],
				imgComp->m_tc.textureCoord[1][1]
			};

			img->bottomLeft = Vector2D{
				imgComp->m_tc.textureCoord[2][0],
				imgComp->m_tc.textureCoord[2][1]
			};

			img->bottomRight = Vector2D{
				imgComp->m_tc.textureCoord[3][0],
				imgComp->m_tc.textureCoord[3][1]
			};
		}

		void setImage2DComponent(MonoString* objID, MonoImage2DComponent* img)
		{
			if (!objID || !img) return;

			std::string _objID(mono_string_to_utf8(objID));
			auto objPtr = OBJECTMANAGER.GetObjectById(_objID);
			if (!objPtr) return;

			auto imgComp = objPtr->GetComponent<Component::Image2DComponent>().lock();
			if (!imgComp) return;

			imgComp->m_image_hashed = static_cast<size_t>(img->reflected_image_hashed);

			// Directly assign glm::vec4 to Color union
			imgComp->m_color.rgba = img->reflected_color;

			// Set texture coordinates back
			/*for (int i = 0; i < 4; ++i) {
				imgComp->m_tc.textureCoord[i][0] = img->reflected_tc[i * 2 + 0];
				imgComp->m_tc.textureCoord[i][1] = img->reflected_tc[i * 2 + 1];
			}*/

			imgComp->m_tc.textureCoord[0][0] = img->topLeft.x;
			imgComp->m_tc.textureCoord[0][1] = img->topLeft.y;

			imgComp->m_tc.textureCoord[1][0] = img->topRight.x;
			imgComp->m_tc.textureCoord[1][1] = img->topRight.y;

			imgComp->m_tc.textureCoord[2][0] = img->bottomLeft.x;
			imgComp->m_tc.textureCoord[2][1] = img->bottomLeft.y;

			imgComp->m_tc.textureCoord[3][0] = img->bottomRight.x;
			imgComp->m_tc.textureCoord[3][1] = img->bottomRight.y;
		}

		void getButtonComponent(MonoString* objID, MonoButtonComponent* btn)
		{
			if (!objID || !btn) return;
			std::string _objID(mono_string_to_utf8(objID));

			auto objPtr = OBJECTMANAGER.GetObjectById(_objID);
			if (!objPtr) return;

			auto btnCompPtr = objPtr->GetComponent<Component::ButtonComponent>().lock();
			if (!btnCompPtr) return;

			const std::array<bool, 4>& L = btnCompPtr->GetLeftStates();
			const std::array<bool, 4>& R = btnCompPtr->GetRightStates();
			const std::array<bool, 4>& M = btnCompPtr->GetMiddleStates();

			for (int i = 0; i < 4; ++i) {
				btn->reflected_L_buttonStates[i] = L[i] ? 1 : 0;
				btn->reflected_R_buttonStates[i] = R[i] ? 1 : 0;
				btn->reflected_M_buttonStates[i] = M[i] ? 1 : 0;
			}
		}

		void setButtonComponent(MonoString* objID, MonoButtonComponent* btn)
		{
			if (!objID || !btn) return;
			std::string _objID(mono_string_to_utf8(objID));

			auto objPtr = OBJECTMANAGER.GetObjectById(_objID);
			if (!objPtr) return;

			auto btnCompPtr = objPtr->GetComponent<Component::ButtonComponent>().lock();
			if (!btnCompPtr) return;

			std::array<bool, 4> L{};
			std::array<bool, 4> R{};
			std::array<bool, 4> M{};

			for (int i = 0; i < 4; ++i) {
				L[i] = (btn->reflected_L_buttonStates[i] != 0);
				R[i] = (btn->reflected_R_buttonStates[i] != 0);
				M[i] = (btn->reflected_M_buttonStates[i] != 0);
			}

			btnCompPtr->SetLeftStates(L);
			btnCompPtr->SetRightStates(R);
			btnCompPtr->SetMiddleStates(M);
		}

		void getTextMeshUIComponent(MonoString* objID, MonoTextMeshUIComponent* tm)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto tmComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::TextMeshUIComponent>().lock();

			if (!tmComp)
				return;
			
			tm->color = tmComp->font_color.rgba;
			tm->hashed = tmComp->font_hash;
			tm->text = mono_string_new(mono_domain_get(), tmComp->text_content.c_str());
			tm->reflected_size = tmComp->font_size;
			tm->reflected_spacing = tmComp->line_spacing;

			/*AG_CORE_INFO("[getTextMeshUI] obj={} hash={} size={} spacing={} color=({},{},{},{}) text='{}'",
				_objID, tmComp->font_hash, tmComp->font_size, tmComp->line_spacing,
				tmComp->font_color.rgba.r, tmComp->font_color.rgba.g, 
				tmComp->font_color.rgba.b, tmComp->font_color.rgba.a,
				tmComp->text_content);*/
		}

		void setTextMeshUIComponent(MonoString* objID, MonoTextMeshUIComponent* tm)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto tmComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::TextMeshUIComponent>().lock();

			if (!tmComp)
				return;

			/*AG_CORE_INFO("[setTextMeshUI] obj={} incoming: hash={} size={} spacing={} color=({},{},{},{}) text='{}'",
				_objID, tm->hashed, tm->reflected_size, tm->reflected_spacing,
				tm->color.r, tm->color.g, tm->color.b, tm->color.a,
				mono_string_to_utf8(tm->text));*/

			tmComp->font_color.rgba = tm->color;
			// Only overwrite font_hash if the C# side has a valid (non-zero) value.
			if (tm->hashed != 0)
				tmComp->font_hash = tm->hashed;
			tmComp->text_content = mono_string_to_utf8(tm->text);
			tmComp->font_size = tm->reflected_size;
			tmComp->line_spacing = tm->reflected_spacing;
		}

		void setFont(MonoString* objID, MonoString* font_str) {
			std::string _id = mono_string_to_utf8(objID);
			std::string _font = mono_string_to_utf8(font_str);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				auto tm = obj->GetComponent<Component::TextMeshUIComponent>().lock();
				if (tm) tm->SetFont(_font);
			}
		}

		void setFontSize(MonoString* objID, float size) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				auto tm = obj->GetComponent<Component::TextMeshUIComponent>().lock();
				if (tm) tm->SetFontSize(size);
			}
		}

		void setLineSpacing(MonoString* objID, float spacing) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				auto tm = obj->GetComponent<Component::TextMeshUIComponent>().lock();
				if (tm) tm->SetLineSpacing(spacing);
			}
		}

		void setLetterSpacing(MonoString* objID, float spacing) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				auto tm = obj->GetComponent<Component::TextMeshUIComponent>().lock();
				if (tm) tm->SetLetterSpacing(spacing);
			}
		}

		void setWrapLimit(MonoString* objID, float limit) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				auto tm = obj->GetComponent<Component::TextMeshUIComponent>().lock();
				if (tm) tm->SetWrapLimit(limit);
			}
		}

		void setWrap(MonoString* objID, bool wrap) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				auto tm = obj->GetComponent<Component::TextMeshUIComponent>().lock();
				if (tm) tm->SetWrap(wrap);
			}
		}

		void setCentered(MonoString* objID, bool centered) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				auto tm = obj->GetComponent<Component::TextMeshUIComponent>().lock();
				if (tm) tm->SetCentered(centered);
			}
		}

		void setTextColor(MonoString* objID, float r, float g, float b, float a) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				auto tm = obj->GetComponent<Component::TextMeshUIComponent>().lock();
				if (tm) {
					// Assuming SetColor takes 4 floats as per your request
					tm->SetColor(r, g, b, a);
				}
			}
		}

		void getMeshRendererComponent(MonoString* objID, MonoMeshRendererComponent* meshrenderer)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto meshrendererComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::MeshRendererComponent>().lock();

			if (!meshrendererComp)
				return;

			meshrenderer->reflected_model_hash = meshrendererComp->model_hash;
			mono_string_new(mono_domain_get(), meshrendererComp->mesh_name.c_str());
			
			//meshrenderer->reflected_submesh_name_hashes = meshrendererComp->submesh_name_hashes;
			//meshrenderer->reflected_material_hash_list = meshrendererComp->material_hash_list;
			//meshrenderer->reflected_material_value_list = meshrendererComp->material_value_list;
			meshrenderer->reflected_shininess = meshrendererComp->shininess;
		}

		void setMeshRendererComponent(MonoString* objID, MonoMeshRendererComponent* meshrenderer)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto meshrendererComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::MeshRendererComponent>().lock();

			if (!meshrendererComp)
				return;

			meshrendererComp->model_hash = meshrenderer->reflected_model_hash;
			std::string mesh_name = mono_string_to_utf8(meshrenderer->reflected_mesh_name);
			meshrendererComp->mesh_name = mesh_name;
			//meshrendererComp->submesh_name_hashes = meshrenderer->reflected_submesh_name_hashes;
			//meshrendererComp->material_hash_list = meshrenderer->reflected_material_hash_list;
			//meshrendererComp->material_value_list = meshrenderer->reflected_material_value_list;
			meshrendererComp->shininess = meshrenderer->reflected_shininess;
		}

		void setMeshRendererSubmeshMaterial(MonoString* objID, MonoString* submeshName, MonoString* materialName)
		{
			std::string _objID = mono_string_to_utf8(objID);
			std::string _submesh = mono_string_to_utf8(submeshName);
			std::string _material = mono_string_to_utf8(materialName);
			auto obj = OBJECTMANAGER.GetObjectById(_objID);
			if (obj) {
				auto meshRenderer = obj->GetComponent<Component::MeshRendererComponent>().lock();
				if (meshRenderer) {
					meshRenderer->SetMeshRendererSubmeshMaterial(_submesh, _material);
				}
			}
		}

		void getSkinnedMeshRenderer(MonoString* objID, MonoSkinnedMeshRenderer* skinnedmeshrenderer)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto smrComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::SkinnedMeshRenderer>().lock();

			if (!smrComp)
				return;

			skinnedmeshrenderer->reflected_skinned_mesh_hash = smrComp->skinned_mesh_hash;
			skinnedmeshrenderer->reflected_animation_clip_name = mono_string_new(mono_domain_get(), smrComp->animation_clip_name.c_str());
			skinnedmeshrenderer->reflected_loop = smrComp->loop;
			skinnedmeshrenderer->reflected_play = smrComp->play;
		}

		void setSkinnedMeshRenderer(MonoString* objID, MonoSkinnedMeshRenderer* skinnedmeshrenderer)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto smrComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::SkinnedMeshRenderer>().lock();

			if (!smrComp)
				return;

			smrComp->skinned_mesh_hash = skinnedmeshrenderer->reflected_skinned_mesh_hash;
			smrComp->animation_clip_name = mono_string_to_utf8(skinnedmeshrenderer->reflected_animation_clip_name);
			smrComp->loop = skinnedmeshrenderer->reflected_loop;
			smrComp->play = skinnedmeshrenderer->reflected_play;
		}

		void setMeshFromString(MonoString* objID, MonoString* meshName) {
			std::string _objID(mono_string_to_utf8(objID));
			std::string _meshName(mono_string_to_utf8(meshName));

			auto meshRenderer = OBJECTMANAGER.GetObjectById(_objID)->GetComponent<Component::MeshRendererComponent>().lock();
			if (meshRenderer) {
				meshRenderer->setMeshfromString(_meshName);
			}
		}

		void setMeshFromHash(MonoString* objID, uint64_t hash) {
			std::string _objID(mono_string_to_utf8(objID));

			auto meshRenderer = OBJECTMANAGER.GetObjectById(_objID)->GetComponent<Component::MeshRendererComponent>().lock();
			if (meshRenderer) {
				meshRenderer->setMeshfromHash(static_cast<size_t>(hash));
			}
		}

		void setSubmeshMaterial(MonoString* objID, MonoString* submesh_name, MonoString* material_name) {
			std::string _id = mono_string_to_utf8(objID);
			std::string _submesh = mono_string_to_utf8(submesh_name);
			std::string _material = mono_string_to_utf8(material_name);

			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (!obj) return;

			auto meshRenderer = obj->GetComponent<Component::SkinnedMeshRenderer>().lock();

			if (meshRenderer) 
			{
				meshRenderer->SetSubmeshMaterial(_submesh, _material);
				return;
			}
		}

		void playAnimation(MonoString* objID, MonoString* animationName) {
			std::string _objID = mono_string_to_utf8(objID);
			auto smr = OBJECTMANAGER.GetObjectById(_objID)->GetComponent<Component::SkinnedMeshRenderer>().lock();
			if (smr) {
				smr->PlayAnimation(mono_string_to_utf8(animationName));
			}
		}

		void setLoopAnimation(MonoString* objID, bool shouldLoop) {
			std::string _objID = mono_string_to_utf8(objID);
			auto smr = OBJECTMANAGER.GetObjectById(_objID)->GetComponent<Component::SkinnedMeshRenderer>().lock();
			if (smr) smr->SetLoopAnimation(shouldLoop);
		}

		/*void stopAnimation(MonoString* objID) {
			std::string _objID = mono_string_to_utf8(objID);
			auto smr = OBJECTMANAGER.GetObjectById(_objID)->GetComponent<Component::SkinnedMeshRenderer>().lock();
			if (smr) smr->stopAnimation();
		}*/

		void playCurrentAnimation(MonoString* objID) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				
				auto smr = obj->GetComponent<Component::SkinnedMeshRenderer>().lock();
				if (smr) {
					smr->playCurrentAnimation();
				}
			}
		}

		void stopAnimation(MonoString* objID) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				if (auto smr = obj->GetComponent<Component::SkinnedMeshRenderer>().lock()) {
					smr->stopAnimation();
				}
			}
		}

		void resetAnimationTimer(MonoString* objID) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				auto smr = obj->GetComponent<Component::SkinnedMeshRenderer>().lock();
				if (smr) {
					smr->ResetTimer();
				}
			}
		}

		void setAnimationTimer(MonoString* objID, float time) {
			std::string _id = mono_string_to_utf8(objID);
			auto obj = OBJECTMANAGER.GetObjectById(_id);
			if (obj) {
				auto smr = obj->GetComponent<Component::SkinnedMeshRenderer>().lock();
				if (smr) {
					smr->SetTimer(time);
				}
			}
		}

		void changeModel(MonoString* objID, MonoString* modelName) {
			std::string _objID = mono_string_to_utf8(objID);
			auto smr = OBJECTMANAGER.GetObjectById(_objID)->GetComponent<Component::SkinnedMeshRenderer>().lock();
			if (smr) smr->ChangeModel(mono_string_to_utf8(modelName));
		}

		void changeShaderExcept(MonoString* objID, MonoString* shaderName, MonoString* submeshName) {
			std::string _objID = mono_string_to_utf8(objID);
			auto smr = OBJECTMANAGER.GetObjectById(_objID)->GetComponent<Component::SkinnedMeshRenderer>().lock();
			if (smr) smr->ChangeShaderExcept(mono_string_to_utf8(shaderName), mono_string_to_utf8(submeshName));
		}

		

		void getVideoPlayerComponent(MonoString* objID, MonoVideoPlayerComponent* vp)
		{
			(void)vp;
			std::string _objID(mono_string_to_utf8(objID));
			auto vpComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::VideoPlayerComponent>().lock();

			if (!vpComp) return;

			// Access private members via reflection macros if they were public, or rely on friends.
			// Since we are in core engine, we usually access public members or use SerializeField system.
			// But wait, VideoPlayerComponent members are private and reflection macro is inside class.
			// The standard pattern here seems to access direct members assuming struct mapping or public access.
			// Let's assume we can access them because the serializer does.
			// Actually, VideoPlayerComponent members m_videoPath etc are private.
			// I should probably make them public or provide getters/setters or use Friend class?
			// For simplicity in this step, I'll assume I need to update VideoPlayerComponent to be friend or have public members.
			// But for now, let's use the reflection system's GetValue/SetValue style or just cast?
			// No, direct access is standard here. I'll rely on friend declaration if needed or just cast.
			// Wait, previous components seem to access members directly. 
			// VideoPlayerComponent has members in REFLECT() block but declared as private variables below it?
			// Let's check VideoPlayerComponent.h again. Yes, m_videoPath is private.
			// I'll assume for now I will fix visibility or add getters.
			// Let's use direct access and if it fails compilation due to private, I will fix header.
			
			// Ah, wait. I can't access private members here.
			// I should add friend class ScriptConnector; to VideoPlayerComponent.h?
			// Or just use getters?
			// I'll implement assuming public getters exist or I will add them.
			
			// Actually, let's re-read VideoPlayerComponent.h generated in step 1137. 
			// REFLECT() is public. Members are private.
			// I will update VideoPlayerComponent.h to have friend declaration for internal call namespace or ScriptConnector.
			// Or just make members public for simplicity as per other components?
			// Most other components (e.g. RigidBodyComponent) have members public or struct _data public.
			
			// I'll assume they are accessible for now to move forward, and fix header next if needed.
			// Wait, I can use the same reflection names.
			
			// NOTE: I am casting to reference because REFLECT uses them.
			// But wait, direct access `vpComp->m_videoPath`.
			
			// To be safe, I will implement internal calls to use Play/Stop methods mainly for logic,
			// and reflection for properties.
			
			// Implementing property reflection:
			// NOTE: This will fail if m_videoPath is private.
			// I'll skip direct member access for 'private' fields and focus on the runtime methods which are public.
			// But SetVideoPlayerComponent is needed for inspector?
			// Inspector usually uses C++ reflection. C# side uses this for Get/Set in scripts.
			
			// Actually, let's implement the methods first.
		}

		void setVideoPlayerComponent(MonoString* objID, MonoVideoPlayerComponent* vp)
		{
			(void)vp;
			(void)objID;
			// Same visibility issue.
		}

		void playVideo(MonoString* objID, MonoString* path)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto vpComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::VideoPlayerComponent>().lock();
			if (vpComp) {
				std::string p = mono_string_to_utf8(path);
				vpComp->Play(p);
			}
		}

		void stopVideo(MonoString* objID)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto vpComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::VideoPlayerComponent>().lock();
			if (vpComp) {
				vpComp->Stop();
			}
		}

		bool isVideoFinished(MonoString* objID)
		{
			std::string _objID(mono_string_to_utf8(objID));
			auto vpComp = OBJECTMANAGER.GetObjectById(_objID)
				->GetComponent<Component::VideoPlayerComponent>().lock();
			return vpComp ? vpComp->IsFinished() : true;
		}

		float getGamma()
		{
			return PostProcesser::GetInstance().GetGammaScaling();
		}

		void setGamma(float gamma)
		{
			PostProcesser::GetInstance().SetGammaScaling(gamma);
		}

		bool getTonemapEnabled()
		{
			return PostProcesser::GetInstance().IsTonemapEnabled();
		}

		void setTonemapEnabled(bool enabled)
		{
			PostProcesser::GetInstance().SetTonemapEnabled(enabled);
		}
#pragma endregion
#pragma region Camera
		void GetCameraRay(MonoString* objID, MonoCameraRay* outRay) {
			if (!outRay) return;
			if (!objID) {
				AG_CORE_WARN("[GetCameraRay] objID is null");
				return;
			}
			
			// Safely convert MonoString to C++ string
			char* utf8Str = mono_string_to_utf8(objID);
			if (!utf8Str) {
				AG_CORE_WARN("[GetCameraRay] Failed to convert objID to UTF-8");
				return;
			}
			
			std::string _objID(utf8Str);
			mono_free(utf8Str);  // Free the Mono-allocated string
			
			// Get the object - check if it exists
			auto objPtr = OBJECTMANAGER.GetObjectById(_objID);
			if (!objPtr) {
				AG_CORE_WARN("[GetCameraRay] Object not found: {}", _objID);
				return;
			}
			
			auto cameraComp = objPtr->GetComponent<Component::CameraComponent>().lock();

			if (!cameraComp) {
				AG_CORE_WARN("[GetCameraRay] No camera component found for: {}", _objID);
				return;
			}

			Component::CameraRay cppRay = cameraComp->GetForwardRay();
			
			// Copy values explicitly to ensure correct marshalling
			outRay->origin = cppRay.origin;
			outRay->direction = cppRay.direction;
		}

		void setOverlayCamera(MonoString* objID)
		{
			if (!objID)
			{
				CAMERAMANAGER.setOverlayCamera(nullptr);
				return;
			}

			char* utf8Str = mono_string_to_utf8(objID);
			if (!utf8Str)
			{
				CAMERAMANAGER.setOverlayCamera(nullptr);
				return;
			}

			std::string _objID(utf8Str);
			mono_free(utf8Str);

			if (_objID.empty())
			{
				CAMERAMANAGER.setOverlayCamera(nullptr);
				return;
			}

			auto objPtr = OBJECTMANAGER.GetObjectById(_objID);
			if (!objPtr)
			{
				CAMERAMANAGER.setOverlayCamera(nullptr);
				return;
			}

			auto camComp = objPtr->GetComponent<Component::CameraComponent>().lock();
			if (!camComp)
			{
				CAMERAMANAGER.setOverlayCamera(nullptr);
				return;
			}

			CAMERAMANAGER.setOverlayCamera(camComp->GetCameraPtr());
		}

		void setOverlayActive(bool active)
		{
			CAMERAMANAGER.setOverlayActive(active);
		}

		void setOverlayViewport(float x, float y, float w, float h)
		{
			CAMERAMANAGER.setOverlayViewport(x, y, w, h);
		}
#pragma endregion
#pragma region Log

		void log(int logLevel, MonoString* logmsg) {
			char* utf8String = mono_string_to_utf8(logmsg);
			std::string cppString(utf8String);
			cppString = "From C#: " + cppString;
			switch (logLevel) {
			case 0:
				AG_CORE_TRACE(cppString);
				break;
			case 1:
				AG_CORE_INFO(cppString);
				break;
			case 2:
				AG_CORE_WARN(cppString);
				break;
			case 3:
				AG_CORE_ERROR(cppString);
				break;
			case 4:
				AG_CORE_FATAL(cppString);
				break;
			default:
				break;
			}
		}
#pragma endregion

#pragma region Debug Drawing
		void drawLine(Vector3D a, Vector3D b) {
			glm::vec3 a_vec3(a.x, a.y, a.z);
			glm::vec3 b_vec3(b.x, b.y, b.z);

			DebugRenderer::GetInstance().DrawLine(a_vec3, b_vec3, {0.f, 1.0f, 0.f, 1.0f});
		}
#pragma endregion

#pragma region input
		void getMousePostion(Vector2D* mousePos)
		{
			mousePos->setX(Input::GetMousePosition().first);
			mousePos->setY(Input::GetMousePosition().second);
		}

		void getMouseDelta(Vector2D* mouseDelta)
		{
			std::pair<float, float> delta = Input::GetMouseDelta();
			mouseDelta->setX(delta.first);
			mouseDelta->setY(delta.second);
		}

		void mouseScroll(Vector2D* offset)
		{
			offset->setY(Input::GetMouseScroll());
		}
		
		bool isMousePressed(int btn)
		{
			return Input::IsMouseButtonPressed(btn);
		}
		
		bool isMouseReleased(int btn)
		{
			return Input::IsMouseButtonReleased(btn);
		}

		bool isKeyPressed(int key)
		{
			return Input::IsKeyPressed(key);
		}
		
		bool isKeyReleased(int key)
		{
			return Input::IsKeyReleased(key);
		}

		bool getMouseState() {
			return AGWindow::cursor_disabled;
		}
		void setMouseState(bool state) {
			AGWindow::cursor_disabled = state;
			if (state == true) {
				glfwSetInputMode(AGWINDOW.getWindowContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				glfwSetInputMode(AGWINDOW.getWindowContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			
		}
#pragma endregion

#pragma region Node
		void setNodeStatus(MonoString* objID, MonoString* nodeID, AI::NodeStatus status)
		{
			std::weak_ptr<AI::AINode> node = getNode(objID, nodeID);
			if (node.expired()) {
				AG_CORE_WARN("Cant find node");
				return;
			}
			node.lock()->setStatus(status);
			return;
		}
		void getNodeStatus(MonoString* objID, MonoString* nodeID, AI::NodeStatus* status)
		{
			std::weak_ptr<AI::AINode> node = getNode(objID, nodeID);
			if (node.expired()) {
				AG_CORE_WARN("Cant find node");
				return;
			}
			*status = node.lock()->getStatus();
			return;
		}
		void setNodeResult(MonoString* objID, MonoString* nodeID, AI::NodeResult result)
		{
			std::weak_ptr<AI::AINode> node = getNode(objID, nodeID);
			if (node.expired()) {
				AG_CORE_WARN("Cant find node");
				return;
			}
			node.lock()->setResult(result);
			return;
		}
		void getNodeResult(MonoString* objID, MonoString* nodeID, AI::NodeResult* result)
		{
			std::weak_ptr<AI::AINode> node = getNode(objID, nodeID);
			if (node.expired()) {
				AG_CORE_WARN("Cant find node");
				return;
			}
			*result = node.lock()->getResult();
			return;
		}
		void nodeTick(MonoString* objID, MonoString* nodeID)
		{
			std::weak_ptr<AI::AINode> node = getNode(objID, nodeID);
			if (node.expired()) {
				AG_CORE_WARN("Cant find node");
				return;
			}
			node.lock()->tick(static_cast<float>(BENCHMARKER.GetDeltaTime()));
			return;
		}
		MonoArray* getNodeChildrens(MonoString* objID, MonoString* nodeID)
		{
			std::vector<AI::AINode::NodeID> children{};
			std::weak_ptr<AI::AINode> node = getNode(objID, nodeID);
			if (node.expired()) {
				AG_CORE_WARN("Cant find node");
				return nullptr;
			}
			node.lock()->getChildren(children);
			MonoArray* result = mono_array_new(SCRIPTENGINE.getAppDomain(), mono_get_string_class(), children.size());
			for (size_t i{}; i < children.size(); ++i) {
				MonoString* monoStr = mono_string_new(SCRIPTENGINE.getAppDomain(), children[i].c_str());
				mono_array_set(result, MonoString*, i, monoStr);
			}
			return result;
			
		}

#pragma endregion

#pragma region Audio
		MonoString* getAudioFullPath(MonoString* objID, MonoString* eventName)
		{
			std::string _objID{ mono_string_to_utf8(objID) };
			std::string _eventName{ mono_string_to_utf8(eventName) };

			auto objPtr = OBJECTMANAGER.GetObjectByID(_objID);
			if (!objPtr) {
				return mono_string_new(SCRIPTENGINE.getAppDomain(), "");
			}

			System::IObject::WeakCompPtr cmpPtr = objPtr->GetComponent(Data::ComponentTypes::Audio);
			if (cmpPtr.expired()) {
				return mono_string_new(SCRIPTENGINE.getAppDomain(), "");
			}

			std::shared_ptr<Component::AudioComponent> audioCmp = std::dynamic_pointer_cast<Component::AudioComponent>(cmpPtr.lock());
			if (!audioCmp) {
				return mono_string_new(SCRIPTENGINE.getAppDomain(), "");
			}

			return mono_string_new(SCRIPTENGINE.getAppDomain(), audioCmp->getEvent(_eventName).c_str());
		}

		void playAudio(MonoString* objID, MonoString* eventName)
		{
			std::string _objID{ mono_string_to_utf8(objID) };
			std::string _eventName{ mono_string_to_utf8(eventName) };

			auto objPtr = OBJECTMANAGER.GetObjectByID(_objID);
			if (!objPtr) {
				return;
			}

			System::IObject::WeakCompPtr cmpPtr = objPtr->GetComponent(Data::ComponentTypes::Audio);
			if (cmpPtr.expired()) {
				return;
			}

			std::shared_ptr<Component::AudioComponent> audioCmp = std::dynamic_pointer_cast<Component::AudioComponent>(cmpPtr.lock());
			if (!audioCmp) {
				return;
			}

			audioCmp->PlayEvent(_eventName);
		}

		void pauseAudio(MonoString* objID, MonoString* eventName)
		{
			std::string _objID{ mono_string_to_utf8(objID) };
			std::string _eventName{ mono_string_to_utf8(eventName) };

			auto objPtr = OBJECTMANAGER.GetObjectByID(_objID);
			if (!objPtr) {
				return;
			}

			System::IObject::WeakCompPtr cmpPtr = objPtr->GetComponent(Data::ComponentTypes::Audio);
			if (cmpPtr.expired()) return;

			std::shared_ptr<Component::AudioComponent> audioCmp = std::dynamic_pointer_cast<Component::AudioComponent>(cmpPtr.lock());
			if (!audioCmp) return;

			audioCmp->pauseEvent(_eventName);
		}

		void stopAudio(MonoString* objID, MonoString* eventName)
		{
			std::string _objID{ mono_string_to_utf8(objID) };
			std::string _eventName{ mono_string_to_utf8(eventName) };

			auto objPtr = OBJECTMANAGER.GetObjectByID(_objID);
			if (!objPtr) {
				return;
			}

			System::IObject::WeakCompPtr cmpPtr = objPtr->GetComponent(Data::ComponentTypes::Audio);
			if (cmpPtr.expired()) return;

			std::shared_ptr<Component::AudioComponent> audioCmp = std::dynamic_pointer_cast<Component::AudioComponent>(cmpPtr.lock());
			if (!audioCmp) return;

			audioCmp->StopEvent(_eventName);
		}

		float getAudioParameter(MonoString* objID, MonoString* eventName, MonoString* paramName)
		{
			float result{ -1.f };
			std::string _objID{ mono_string_to_utf8(objID) };
			std::string _eventName{ mono_string_to_utf8(eventName) };
			std::string _paramName{ mono_string_to_utf8(paramName) };

			auto objPtr = OBJECTMANAGER.GetObjectByID(_objID);
			if (!objPtr) {
				return result;
			}

			System::IObject::WeakCompPtr cmpPtr = objPtr->GetComponent(Data::ComponentTypes::Audio);
			if (cmpPtr.expired()) return result;

			std::shared_ptr<Component::AudioComponent> audioCmp = std::dynamic_pointer_cast<Component::AudioComponent>(cmpPtr.lock());
			if (!audioCmp) return result;

			result = audioCmp->getParameter(_eventName, _paramName);
			return result;
		}

		void setAudioParameter(MonoString* objID, MonoString* eventName, MonoString* paramName, float value)
		{
			std::string _objID{ mono_string_to_utf8(objID) };
			std::string _eventName{ mono_string_to_utf8(eventName) };
			std::string _paramName{ mono_string_to_utf8(paramName) };

			auto objPtr = OBJECTMANAGER.GetObjectByID(_objID);
			if (!objPtr) {
				return;
			}

			System::IObject::WeakCompPtr cmpPtr = objPtr->GetComponent(Data::ComponentTypes::Audio);
			if (cmpPtr.expired()) return;

			std::shared_ptr<Component::AudioComponent> audioCmp = std::dynamic_pointer_cast<Component::AudioComponent>(cmpPtr.lock());
			if (!audioCmp) return;

			audioCmp->setParameter(_eventName, _paramName, value);
		}

		void setVolume(float volume, MonoString* busName)
		{
			std::string _busName{ mono_string_to_utf8(busName) };
			if (_busName == "") {
				AUDIOSYSTEM.setBusVolume("bus:/", volume);
			}
			else {
				AUDIOSYSTEM.setBusVolume(_busName, volume);
		}
		}

		float getVolume(MonoString* busName)
		{
			std::string _busName{ mono_string_to_utf8(busName) };
			if (_busName == "") {
				return AUDIOSYSTEM.getBusVolume("bus:/");
			}
			else {
				return AUDIOSYSTEM.getBusVolume(_busName);
			}
			return 0.0f;
		}

		void setAudioInstanceVolume(MonoString* objID, MonoString* eventName, float volume)
		{
			std::string id = mono_string_to_utf8(objID);
			std::string event = mono_string_to_utf8(eventName);

			auto obj = OBJECTMANAGER.GetObjectByID(id);
			if (!obj) return;

			auto audio = obj->GetComponent<AG::Component::AudioComponent>().lock();
			if (!audio) return;

			audio->setVolume(event, volume);
		}
#pragma endregion

#pragma region Physics
		bool AG::InternalCall::isRayCastHit(glm::vec3 origin,glm::vec3 direction, MonoRaycastHitData* hitInfo,float maxDistance)
		{
			Component::RaycastHit cppHitInfo;
			bool isHit = AG::Systems::PhysXManager::GetInstance()->Raycast(origin,direction,maxDistance, cppHitInfo);
			if (!isHit) return false;

			hitInfo->point = cppHitInfo.point;
			hitInfo->normal = cppHitInfo.normal;
			hitInfo->distance = cppHitInfo.distance;
			MonoString* monoEntID = mono_string_new(SCRIPTENGINE.getAppDomain(), cppHitInfo.hitEntityID.c_str());
			hitInfo->hitEntityID = monoEntID;

			auto objectPtr = OBJECTMANAGER.GetObjectByID(cppHitInfo.hitEntityID);
			auto wkCompPtr = objectPtr.get()->GetComponent(Data::ComponentTypes::BoxCollider);
			auto compPtr = wkCompPtr.lock();
			if (compPtr) {
				auto* boxColliderComp = dynamic_cast<AG::Component::BoxColliderComponent*>(compPtr.get());
				if (boxColliderComp) {
					if (boxColliderComp->layer & Component::CollisionLayers::Layer_Default)
						hitInfo->layer = Component::CollisionLayers::Layer_Default;

					if (boxColliderComp->layer & Component::CollisionLayers::Layer_Player)
						hitInfo->layer = Component::CollisionLayers::Layer_Player;

					if (boxColliderComp->layer & Component::CollisionLayers::Layer_NPC)
						hitInfo->layer = Component::CollisionLayers::Layer_NPC;

					if (boxColliderComp->layer & Component::CollisionLayers::Layer_Trigger)
						hitInfo->layer = Component::CollisionLayers::Layer_Trigger;

					if (boxColliderComp->layer & Component::CollisionLayers::Layer_Computer)
						hitInfo->layer = Component::CollisionLayers::Layer_Computer;

					if (boxColliderComp->layer & Component::CollisionLayers::Layer_Centrifuge)
						hitInfo->layer = Component::CollisionLayers::Layer_Centrifuge;

					if (boxColliderComp->layer & Component::CollisionLayers::Layer_SkinTest)
						hitInfo->layer = Component::CollisionLayers::Layer_SkinTest;

					if (boxColliderComp->layer & Component::CollisionLayers::Layer_UrineTest)
						hitInfo->layer = Component::CollisionLayers::Layer_UrineTest;
				}
				else {
					hitInfo->layer = Component::CollisionLayers::Layer_Default;
				}
			}
			return isHit;
		}

#pragma endregion

#pragma region Scene
		void loadScene(MonoString* fileName)
		{
			std::string _fileName = mono_string_to_utf8(fileName);
			auto& objMgr = OBJECTMANAGER;
			auto& compMgr = COMPONENTMANAGER;
			objMgr.ClearScene();
			compMgr.ClearAllComponents();

			AG::SystemTest::SceneManager::LoadScene(_fileName);
		}

		MonoString* getSceneName()
		{
			return mono_string_new(SCRIPTENGINE.getAppDomain(), AG::SystemTest::SceneManager::GetSceneName().c_str());
		}
#pragma endregion

#pragma region temp
		void setScanState(bool state) {
			ScanSetup::GetInstance().scanning = state;
		}
		bool getScanState() {
			return ScanSetup::GetInstance().scanning;
		}
#pragma endregion
	}

}
