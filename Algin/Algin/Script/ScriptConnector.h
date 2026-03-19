#pragma once
#include "pch.h"

namespace AG {
	class ScriptConnector : Pattern::ISingleton<ScriptConnector> {
	public:
		void init();

		bool HasMonoComponent(MonoType* nonoType, System::IObject::ID objID);
		

	private:
		void addFunction();
		void addComponents();

		template <typename T>
		bool hasComponent(System::IObject::ID objID) {
			std::shared_ptr<System::IObject> temp = OBJECTMANAGER.GetObjectByID(objID);
			return temp ? temp->hasComponent<T>() : false;
		}

		
	private:
		std::unordered_map<MonoType*, std::function<bool(System::IObject::ID)>> entityHasMonoComponent;
	};

#pragma region HelperStructAndFunc
	struct MonoCameraComponent;
	struct MonoLightComponent;
	struct MonoRigidBodyComponent;
	struct MonoBoxColliderComponent;
	struct MonoSkyBoxComponent;
	struct MonoParticleComponent;
	struct MonoPostProcessVolumeComponent;
	struct MonoRectTransformComponent;
	struct MonoImage2DComponent;
	struct MonoButtonComponent;
	struct MonoTextMeshUIComponent;
	struct MonoMeshRendererComponent;
	struct MonoSkinnedMeshRenderer;
	struct MonoRaycastHitData;
	
	// CameraRay struct matching C# layout
	struct MonoCameraRay {
		glm::vec3 origin;
		glm::vec3 direction;
	};
	struct MonoVideoPlayerComponent;

	//Get node helper function
	std::weak_ptr<AI::AINode> getNode(MonoString* objID, MonoString* nodeID);
#pragma endregion

	//1.2 Add the declaration of the internal Call function heres
	namespace InternalCall {
#pragma region Object
		MonoString* createGameObject();
		MonoString* createPatient();
		void setGameObjectActive(MonoString* objID, bool active);
		bool getGameObjectActive(MonoString* objID);
		bool objectHasComponent_type(MonoString* objID, MonoReflectionType* componentType);
		bool objectHasComponent_enum(MonoString* objID, Data::ComponentTypes type);
		void objectAddComponent(MonoString* objID, Data::ComponentTypes type);
		void linkParentChild(MonoString* parentID, MonoString* childID);
#pragma endregion

#pragma region Component

		void getTransformComponent(MonoString* objID, Transformation* trf);
		void setTransformComponent(MonoString* objID, Transformation* trf);
		
		void getCameraComponent(MonoString* objID, MonoCameraComponent* cam);
		void setCameraComponent(MonoString* objID, MonoCameraComponent* cam);

		void getLightComponent(MonoString* objID, MonoLightComponent* light);
		void setLightComponent(MonoString* objID, MonoLightComponent* light);

		void getRigidBodyComponent(MonoString* objID, MonoRigidBodyComponent* rb);
		void setRigidBodyComponent(MonoString* objID, MonoRigidBodyComponent* rb);

		void getBoxColliderComponent(MonoString* objID, MonoBoxColliderComponent* bc);
		void setBoxColliderComponent(MonoString* objID, MonoBoxColliderComponent* bc);

		void getSkyBoxComponent(MonoString* objID, MonoSkyBoxComponent* sky);
		void setSkyBoxComponent(MonoString* objID, MonoSkyBoxComponent* sky);

		void getParticleComponent(MonoString* objID, MonoParticleComponent* pc);
		void setParticleComponent(MonoString* objID, MonoParticleComponent* pc);

		void getPostProcessVolumeComponent(MonoString* objID, MonoPostProcessVolumeComponent* ppv);
		void setPostProcessVolumeComponent(MonoString* objID, MonoPostProcessVolumeComponent* ppv);

		void getRectTransformComponent(MonoString* objID, MonoRectTransformComponent* rt);
		void setRectTransformComponent(MonoString* objID, MonoRectTransformComponent* rt);

		void getImage2DComponent(MonoString* objID, MonoImage2DComponent* img);
		void setImage2DComponent(MonoString* objID, MonoImage2DComponent* img);

		void getButtonComponent(MonoString* objID, MonoButtonComponent* btn);
		void setButtonComponent(MonoString* objID, MonoButtonComponent* btn);

		void getTextMeshUIComponent(MonoString* objID, MonoTextMeshUIComponent* tm);
		void setTextMeshUIComponent(MonoString* objID, MonoTextMeshUIComponent* tm);
		void setFont(MonoString* objID, MonoString* font_str);
		void setFontSize(MonoString* objID, float size);
		void setLineSpacing(MonoString* objID, float spacing);
		void setLetterSpacing(MonoString* objID, float spacing);
		void setWrapLimit(MonoString* objID, float limit);
		void setWrap(MonoString* objID, bool wrap);
		void setCentered(MonoString* objID, bool centered);
		void setTextColor(MonoString* objID, float r, float g, float b, float a);
		void setTextMeshUIText(MonoString* objID, MonoString* newText);

		void getMeshRendererComponent(MonoString* objID, MonoMeshRendererComponent* meshrenderer);
		void setMeshRendererComponent(MonoString* objID, MonoMeshRendererComponent* meshrenderer);
		void setMeshFromString(MonoString* objID, MonoString* meshStr);
		void setMeshFromHash(MonoString* objID, uint64_t hash);
		void setMeshRendererSubmeshMaterial(MonoString* objID, MonoString* submeshName, MonoString* materialName);

		void getSkinnedMeshRenderer(MonoString* objID, MonoSkinnedMeshRenderer* skinnedmeshrenderer);
		void setSkinnedMeshRenderer(MonoString* objID, MonoSkinnedMeshRenderer* skinnedmeshrenderer);
		void changeModel(MonoString* objID, MonoString* modelName);
		void changeShaderExcept(MonoString* objID, MonoString* shaderName, MonoString* submeshName);
		void playAnimation(MonoString* objID, MonoString* animationName);
		void setLoopAnimation(MonoString* objID, bool shouldLoop);
		//void stopAnimation(MonoString* objID);
		void playCurrentAnimation(MonoString* objID);
		void stopAnimation(MonoString* objID);
		void resetAnimationTimer(MonoString* objID);
		void setAnimationTimer(MonoString* objID, float time);
		void setSubmeshMaterial(MonoString* objID, MonoString* submesh_name, MonoString* material_name);

		// Video Player
		void getVideoPlayerComponent(MonoString* objID, MonoVideoPlayerComponent* vp);
		void setVideoPlayerComponent(MonoString* objID, MonoVideoPlayerComponent* vp);
		void playVideo(MonoString* objID, MonoString* path);
		void stopVideo(MonoString* objID);
		bool isVideoFinished(MonoString* objID);

		float getGamma();
		void setGamma(float gamma);
		bool getTonemapEnabled();
		void setTonemapEnabled(bool enabled);

#pragma endregion

#pragma region Camera
		void GetCameraRay(MonoString* objID, MonoCameraRay* outRay);
		void setOverlayCamera(MonoString* objID);
		void setOverlayActive(bool active);
		void setOverlayViewport(float x, float y, float w, float h);
#pragma endregion
		
#pragma region Logger
		void log(int logLevel, MonoString* logmsg);
#pragma endregion

#pragma region Debug Drawing
		void drawLine(Vector3D a, Vector3D b);
#pragma endregion

#pragma region Input
		void getMousePostion(Vector2D* mousePos);
		void getMouseDelta(Vector2D* mouseDelta);
		void mouseScroll(Vector2D* offset);
		bool isMousePressed(int btn);
		bool isMouseReleased(int btn);
		bool isKeyPressed(int key);
		bool isKeyReleased(int key);
		bool getMouseState();
		void setMouseState(bool state);
#pragma endregion

#pragma region Node
		void setNodeStatus(MonoString* objID, MonoString* nodeID, AI::NodeStatus status);
		void getNodeStatus(MonoString* objID, MonoString* nodeID, AI::NodeStatus* status);
		void setNodeResult(MonoString* objID, MonoString* nodeID, AI::NodeResult result);
		void getNodeResult(MonoString* objID, MonoString* nodeID, AI::NodeResult* result);
		void nodeTick(MonoString* objID, MonoString* nodeID);
		MonoArray* getNodeChildrens(MonoString* objID, MonoString* nodeID);
#pragma endregion

#pragma region Audio
		MonoString* getAudioFullPath(MonoString* objID, MonoString* eventName);
		void playAudio(MonoString* objID, MonoString* eventName);
		void pauseAudio(MonoString* objID, MonoString* eventName);
		void stopAudio(MonoString* objID, MonoString* eventName);
		float getAudioParameter(MonoString* objID, MonoString* eventName, MonoString* paramName);
		void setAudioParameter(MonoString* objID, MonoString* eventName, MonoString* paramName, float value);
		void setVolume(float volume, MonoString* busName);
		float getVolume(MonoString* busName);
		void setAudioInstanceVolume(MonoString* objID, MonoString* eventName, float volume);
#pragma endregion
#pragma region Physics
		bool isRayCastHit(glm::vec3 origin, glm::vec3 direction, MonoRaycastHitData* hitInfo, float maxDistance);
#pragma endregion

#pragma region Scene
		void loadScene(MonoString* fileName);
		MonoString* getSceneName();
#pragma endregion

#pragma region temp
		void setScanState(bool state);
		bool getScanState();
#pragma endregion

	}
}
