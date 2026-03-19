/*!*****************************************************************************
\file pch.h
\author Zulfami Ashrafi Bin Wakif
\date 19/2/2025 (MM/DD/YYYY)

\brief Pre-compile header for the project
*******************************************************************************/
#pragma once

#include <Windows.h>
#include <commdlg.h>
#include <shobjidl.h>
//=================== Standard Library ===================//
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <istream>
#include <sstream>
#include <map>
#include <numeric>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <future>
#include <stack>
#include <set>
#include <queue>
#include <filesystem>
#include <iostream> // For standard input/output operations
#include <cstdio>
#include <random>
//=================== Standard Library ===================//

//====================== Data Types ======================//
#include "../Data/ObjectTypes.h"
#include "../Data/ComponentTypes.h"
//====================== Data Types ======================//

//========================= spdlog =======================//
#define SPDLOG_HEADER_ONLY
#define FMT_HEADER_ONLY 
#include <spdlog/spdlog.h>
//========================= spdlog =======================//



//======================= Patterns =======================//
#include "../Pattern/ISingleton.h"
//======================= Patterns =======================//


//=================== Graphics Library ===================//
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_precision.hpp>
//#include <gtx/component_wise.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h> // For OpenGL and windowing
#include <GLFW/glfw3native.h> 
#include <stb_image.h>

#include <DirectXTex.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "AGWindows.h"
//=================== Graphics Library ===================//

//======================= Math ===========================//
#include "../Math/Matrix4x4.h"
#include "../Math/Matrix3x3.h"
#include "../Math/Vector3D.h"
#include "../Math/Vector2D.h"
//======================= Math ===========================//

//=================== Audio Library ===================//
#include <fmod.hpp>
#include <fmod_studio_common.h>
#include <fmod_studio.hpp>
#pragma warning(push)
#pragma warning(disable: 4505) // unreferenced function removed
#include <fmod_errors.h>
#pragma warning(pop)
//=================== Audio Library ===================//


//=================== Imgui Library ======================//
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "ImGuizmo.h"
#include "imnodes.h"
#include "imnodes_internal.h"
//=================== Imgui Library ======================//

#include "../Pattern/ImGuiCustom.h"


//======================= Events ===========================//
// Event.h must be included before any file that uses it.
// Application_Event.h includes Event.h, so this is a good spot.
#include "../System/Application_Event.h" 
//======================= Events ===========================//

//====================== Data Types ======================//
#include <ostreamwrapper.h>
#include <istreamwrapper.h>
#include <prettywriter.h>
#include <document.h>

#include "../Data/ObjectTypes.h"
#include "../Data/GUID.h"
#include "../Data/ComponentTypes.h"
#include "../Data/BinaryIO.h"
#include "../Data/Reflection.h"
#include "../Data/SerializationSystem.h"
#include "../Data/SerializerRegistry.h"
//====================== Data Types ======================//


//====================== Interfaces ======================//
#include "../Scenes/IScene.h"
#include "../ImguiWindow/IImguiWindow.h"
#include "../Pattern/ICommand.h"
#include "../Pattern/CommandManager.h"
#include "../Object Base/IObject.h"
#include "../Component/IComponent.h"
//====================== Interfaces ======================//

#include "../AManager/IAsset.h"
#include "../AManager/TextureAsset.h"
#include "../AManager/StaticMeshAsset.h"
#include "../AManager/SkinnedMeshAsset.h"
#include "../AManager/AudioBankAsset.h"
#include "../AManager/CubeMapAsset.h"
#include "../AManager/FontAsset.h"
#include "../AManager/PrefabAsset.h"
#include "../AManager/TreeAsset.h"
#include "../AManager/MaterialTemplate.h"
#include "../AManager/AssetManager.h"
#include "../AManager/AssetCompiler.h"
#include "../Object Base/Prefab/PrefabEditor.h"
//=================== System Header ======================//
#include "../System/GameStateManager.h"
#include "../Object Base/ObjectManager.h"
#include "../Object Base/ComponentManager.h"
#include "../Object Base/Prefab/PrefabManager.h"
#include "../ImguiWindow/ImGuiSystem.h"
#include "../System/Logger.h"
#include "../System/Benchmarker.h"
#include "../Scenes/SceneTools.h"
#include "../System/PhysXManager.h"
#include "../System/SceneManager.h"

// == With imgui widow == //
#include "../System/ToolBar.h"
#include "../System/FileWatcher.h"
#include "../ImguiWindow/ObjHierarchy.h"
#include "../ImguiWindow/ObjInspector.h"
#include "../ImguiWindow/ImScene.h"
#include "../ImguiWindow/ImGame.h"
#include "../ImguiWindow/ContentBrowser.h"
#include "../ImguiWindow/ConsoleIMGUI.h"
#include "../ImguiWindow/Profiler.h"
#include "../ImguiWindow/Canvas.h"
#include "../ImguiWindow/ObjectPicking.h"
#include "../ImguiWindow/BHTEditor.h"
//=================== System Header ======================//

//===================Input================================//
#include "../Input/Keycodes.h"
#include "../Input/Input.h"
//===================Input================================//


//====================== Componnets ======================//
#include "../Component/TransformComponent.h"
#include "../Component/AudioComponent.h"
#include "../Component/CameraComponent.h"
#include "../Component/LightComponent.h"
#include "../Component/MeshRendererComponent.h"
#include "../Component/SkyBoxComponent.h"
#include "../Component/BoxColliderComponent.h"
#include "../Component/CapsuleColliderComponent.h"
#include "../Component/PostProcessVolumeComponent.h"
#include "../Component/RigidBodyComponent.h"
#include "../Component/RectTransformComponent.h"
#include "../Component/Image2DComponent.h"
#include "../Component/ScriptComponent.h"
#include "../Component/PrefabComponent.h"
#include "../Component/TextMeshUIComponent.h"
#include "../Component/ParticleComponent.h"
#include "../Component/SkinnedMeshRenderer.h"
#include "../Component/BehaviorComponent.h"
#include "../Component/ButtonComponent.h"
#include "../Component/VideoPlayerComponent.h"

#include "../Component/CubeComponent.h"
//====================== Componnets ======================//



//====================== Scenes ==========================//
#include "../Scenes/Scene1.h"
//====================== Scenes ==========================//


//====================== Graphics ==========================//
#include "../Graphics/Camera.h"
#include "../Graphics/Shaders.h"
#include "../Graphics/Light.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/DebugRenderer.h"
#include "../Graphics/ShadowMapping.h"
#include "../Graphics/BatchRenderer.h"
#include "../Graphics/Particles.h"
#include "../Graphics/Cube.h"
#include "../Graphics/MaterialAsset.h"
#include "../Graphics/Scan.h"
#include "../Graphics/VideoTexture.h"
//====================== Graphics ==========================//

//======================= Audio ===========================//
#include "../Audio/Audio.h"
//======================= Audio ===========================//

//======================= Script ===========================//
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include "../Script/SerializeField.h"
#include "../Script/ScriptConnector.h"
#include "../Script/ScriptEngine.h"
//======================= Script ===========================//


//======================= Math ===========================//
#include "../Math/Matrix4x4.h"
#include "../Math/Matrix3x3.h"
#include "../Math/Vector3D.h"
#include "../Math/Vector2D.h"
//======================= Math ===========================//
//======================= Application ===========================//
#include "../AGEngine/Application.h"
//======================= Application ===========================//
