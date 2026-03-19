#include "pch.h"
#include "Application.h"
#include "../Graphics/VideoTexture.h"
#include <filesystem>

#include "../System/Key_Event.h"

#include "../System/DebugMonitor.h"

#include "../System/DebugMonitorView.h"
#include "../System/EventManager.h"

#include "../System/Application_Event.h"

#include "../Input/Input.h"
#include "../Input/Keycodes.h"

#include <deque>          // <-- Move these from the .h
#include <string>         // <-- Move these from the .h

#define WIDTH 1600
#define HEIGHT 900

namespace AG
{
	namespace
	{
		void RenderOverlayCamera()
		{
			if (!CAMERAMANAGER.isOverlayActive())
				return;

			auto overlayCam = CAMERAMANAGER.getOverlayCamera().lock();
			if (!overlayCam)
				return;

			auto prevGameCam = CAMERAMANAGER.getGameCamera().lock();

			auto [winW, winH] = AGWINDOW.getWindowSize();
			auto rect = CAMERAMANAGER.getOverlayViewport();

			int vpW = static_cast<int>(rect.w * winW);
			int vpH = static_cast<int>(rect.h * winH);
			if (vpW < 1) vpW = 1;
			if (vpH < 1) vpH = 1;

			int vpX = static_cast<int>(rect.x * winW);
			int vpY = static_cast<int>((1.0f - rect.y - rect.h) * winH); // top-left origin -> bottom-left

			// Swap in overlay camera for this render
			CAMERAMANAGER.setGameCamera(overlayCam);
			CAMERAMANAGER.setCurrentCamera(CameraManager::CAMERA_TYPE::GAME_CAMERA);

			glEnable(GL_DEPTH_TEST);
			AG::PostProcesser::GetInstance().BeginScene();
			RenderPipeline::GetInstance().SetPipeline(RenderPipeline::LIGHT);
			ShaderManager::GetInstance().Use("Algin_3D");
			ShadowMapping::GetInstance().BindForLighting(ShaderManager::GetInstance().GetShaderProgram());
			COMPONENTMANAGER.RenderGraphicsOnly();
			AG::PostProcesser::GetInstance().EndScene();
			AG::PostProcesser::GetInstance().ApplyOverlay(vpX, vpY, vpW, vpH);

			// Restore camera + GL state
			CAMERAMANAGER.setGameCamera(prevGameCam);
			CAMERAMANAGER.setCurrentCamera(CameraManager::CAMERA_TYPE::GAME_CAMERA);
		}
	}

	struct Application::Impl
	{
		// 2. Move your private members here
		std::unique_ptr<DebugMonitor> m_debugMonitor;
		std::deque<std::string> m_EventLog;
		const size_t m_MaxLogSize = 100;
	};

	Application* Application::s_Instance = nullptr;

	Application::Application() : m_pimpl(std::make_unique<Impl>()) // Create the impl
	{
		s_Instance = this;
		EVENT_MANAGER.Subscribe(EventType::WindowClose, std::bind(&Application::OnWindowClose, this, std::placeholders::_1));
	}
	Application::~Application() { s_Instance = nullptr; }


	void Application::Init() 
	{	
        // Force a MessageBox to confirm new DLL logic is loaded
		// MessageBoxA(NULL, "Algin Engine: DLL Updated Successfully!", "Debug Message", MB_OK | MB_ICONINFORMATION);

		AG::Systems::PhysXManager::GetInstance()->Init();
		SCENESTATEMANAGER.Init();

		std::string m_appName = "Algin";
		std::string m_openingScene = "no scene";
		int m_windowWidth = 1600;
		int m_windowHeight = 900;
#ifdef EXPORT
		AG_CORE_INFO("[EXPORT] Loading GameConfig.json for export...");
		SERIALIZATIONSYSTEM.LoadGameConfig(m_appName, m_windowWidth, m_windowHeight, m_openingScene);
		AG_CORE_INFO("[EXPORT] Loaded GameConfig.json");
#endif 

		AGWINDOW.createWindow(m_windowWidth, m_windowHeight, m_appName.c_str());
		AGWINDOW.SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

		AGWINDOW.createFrameBuffer("Scene");
		AGWINDOW.createFrameBuffer("Game");
		AGWINDOW.createFrameBuffer("Canvas");
		AssetManager::GetInstance().LoadAssets("Assets/");


		// (VideoTexture test moved to Update loop for visibility)


		SHADERMANAGER.LoadAllShaders(); //all shader loaded in this function

		ShadowMapping::GetInstance().init();
		objectPicking::GetInstance().init();
		BatchRenderer::GetInstance().Init();
		Particle::GetInstance().init();
		MaterialSetup::GetInstance().init();

		//AUDIOSYSTEM.printBankEvents();

		/* = Debug Monitor Disabled = */
		//IMGUISYSTEM.AddWindow(std::make_unique<AGImGui::DebugMonitorView>(m_debugMonitor.get()));*/
		//m_pimpl->m_debugMonitor = std::make_unique<DebugMonitor>();
		//IMGUISYSTEM.AddWindow(std::make_unique<AGImGui::DebugMonitorView>(m_pimpl->m_debugMonitor.get()));
		//m_pimpl->m_debugMonitor = std::make_unique<DebugMonitor>();


		SCRIPTENGINE.init();
		AI::TreeBuilder::GetInstance().init();
		AUDIOSYSTEM.Start(); //This must be after the assets init
		AG::PostProcesser::GetInstance().Init();
		//PHYSICS.Init();

#ifdef EXPORT
		AG::System::SerializationSystem::GetInstance().loadSceneJson(m_openingScene);
		SCENESTATEMANAGER.SetSceneState(System::SceneManager::PLAYING);
		SCENESTATEMANAGER.RunOnStart();
#endif 
	}


	void Application::OnEvent(Event& e)
	{
		// Publish the event to all subscribers.
		EVENT_MANAGER.Publish(e);

		if (m_pimpl->m_EventLog.size() >= m_pimpl->m_MaxLogSize) {
			m_pimpl->m_EventLog.pop_back();
		}
		m_pimpl->m_EventLog.push_front(e.ToString());
	}


	// CORRECTED: The implementation also needs to match the new signature.
	bool Application::OnWindowClose(Event& e)
	{
		(void)e; // To mark the parameter as used
		AGWINDOW.closeWindow();
		return true; // The event is handled.
	}


	const std::deque<std::string>& Application::GetEventLog() const
	{
		return m_pimpl->m_EventLog;
	}

	void Application::ClearEventLog()
	{
		m_pimpl->m_EventLog.clear();
	}


	void Application::Update() 
	{
		// System Update //
		while (AGWINDOW.runningWindow())
		{
			// --- VideoTexture Load Test (Delayed) ---
			/*
			static bool s_videoTestDone = false;
			if (!s_videoTestDone) {
				s_videoTestDone = true;
				AG_CORE_INFO("--- [VideoTexture] Delayed Load Test Start ---");
				// Log CWD to help debug path issues
				AG_CORE_INFO("CWD: {}", std::filesystem::current_path().string()); 

				VideoTexture videoTest;
				// Try default path
				std::string path = "Assets/Videos/test.mpg";
				if (videoTest.Load(path)) {
					AG_CORE_INFO("[VideoTexture] SUCCESS: Loaded '{}'", path);
					AG_CORE_INFO("[VideoTexture] Info: {}x{}", videoTest.GetWidth(), videoTest.GetHeight());
				}
				else {
					AG_CORE_WARN("[VideoTexture] FAILED to load '{}'. Trying absolute path fallback...", path);
					
					// Fallback: try constructing absolute path from CWD if possible, or try ../ path
					// Let's just try to be helpful and check if file exists
					if (std::filesystem::exists(path)) {
						AG_CORE_ERROR("[VideoTexture] File '{}' EXISTS but pl_mpeg failed to open it. Check file validtity/permissions.", path);
					} else {
						AG_CORE_ERROR("[VideoTexture] File '{}' does NOT exist relative to CWD.", path);
					}
				}
				AG_CORE_INFO("----------------------------------------------");
			}
			*/
			// -----------------------------------------

			try {
				//auto& objMgr = AG::System::ObjectManager::GetInstance();
				//auto& compMgr = AG::System::ComponentManager::GetInstance();
				//AG::System::SerializationSystem::SaveIfDirty("scene.bin", objMgr, compMgr);
				Benchmarker_StartMarker("Update");

				Benchmarker_StartMarker("GLFW things");
				AGWINDOW.resizeUpdate();

				glViewport(0, 0, AGWINDOW.getWindowSize().first, AGWINDOW.getWindowSize().second);

				glClearColor(0.2f, 0.2f, 0.2f, 1.f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				CAMERAMANAGER.setCurrentCamera(CameraManager::CAMERA_TYPE::SCENE_CAMERA);
				Input::Update();
				Benchmarker_EndMarker("GLFW things");



#ifndef EXPORT
				Benchmarker_StartMarker("IMGUI PRE RENDER");
				IMGUISYSTEM.PreRender();
				Benchmarker_EndMarker("IMGUI PRE RENDER");


				Benchmarker_StartMarker("Scene Manager");
				SCENESTATEMANAGER.Start();
				SCENESTATEMANAGER.Update();
				SCENESTATEMANAGER.End();
				Benchmarker_EndMarker("Scene Manager");
#endif
				Benchmarker_StartMarker("Scene Update");
				AG::Systems::PhysXManager::GetInstance()->UpdatePhysics(static_cast<float>(Benchmarker::GetInstance().GetDeltaTime()));
				COMPONENTMANAGER.Update();
				//COLLISIONSYSTEM.Update();
				SCRIPTENGINE.update();
				Benchmarker_EndMarker("Scene Update");
#ifndef EXPORT
				/* --- EDITOR SCENE --- */
				//Shadow Pass
				Benchmarker_StartMarker("Shadow Pass");
				auto lights = LightManager::GetInstance().getLightList();
				ShadowMapping::GetInstance().BuildShadowSet(lights);
				RenderPipeline::GetInstance().SetPipeline(RenderPipeline::SHADOW);
				ShadowMapping::GetInstance().BeginShadowPass();
				ShadowMapping::GetInstance().EndShadowPass();
				//Light Pass
				Benchmarker_EndMarker("Shadow Pass");
				Benchmarker_StartMarker("Light Pass");
				RenderPipeline::GetInstance().SetPipeline(RenderPipeline::LIGHT);
				ShaderManager::GetInstance().Use("Algin_3D");
				GLuint shader = ShaderManager::GetInstance().GetShaderProgram();
				ShadowMapping::GetInstance().LightPass(shader);
				COMPONENTMANAGER.LateUpdate();
				//double dt = BENCHMARKER.GetDeltaTime();
				//Particle::GetInstance().updateTest(dt);
				//Particle::GetInstance().drawParticles();


				DebugRenderer::GetInstance().DrawPlane();
				AGWINDOW.copyBackBuffer("Scene");
				Benchmarker_EndMarker("Light Pass");
				/* --- EDITOR SCENE --- */


				/* --- CANVAS --- */
				Benchmarker_StartMarker("Canvas");
				glDisable(GL_DEPTH_TEST);
#endif 
#ifdef EXPORT
				auto lights = LightManager::GetInstance().getLightList();
				ShadowMapping::GetInstance().BuildShadowSet(lights);
				RenderPipeline::GetInstance().SetPipeline(RenderPipeline::SHADOW);
				ShadowMapping::GetInstance().BeginShadowPass();
				ShadowMapping::GetInstance().EndShadowPass();
#endif
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				BatchRenderer::GetInstance().BeginBatch();
				PIPELINE.SetPipeline(RenderPipeline::STATE::BATCH);
				COMPONENTMANAGER.LateUpdate();
				BatchRenderer::GetInstance().EndBatch();
#ifndef EXPORT
				BatchRenderer::GetInstance().Render(); // render but dont flush buffer
				AGWINDOW.copyBackBuffer("Canvas");
				glEnable(GL_DEPTH_TEST);
				Benchmarker_EndMarker("Canvas");
				/* --- CANVAS--- */



				if (PrefabEditor::GetInstance().IsEnabled())
					PrefabEditor::GetInstance().RenderGUI();

				if (AssetCompiler::GetInstance().IsEnabled())
				{
					AssetCompiler::GetInstance().RenderGUI();
				}

				/* --- GAME SCENE --- */
				Benchmarker_StartMarker("Game Late Update");
#endif
				glViewport(0, 0, AGWINDOW.getWindowSize().first, AGWINDOW.getWindowSize().second);
				glClearColor(0.2f, 0.2f, 0.2f, 1.f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				CAMERAMANAGER.setCurrentCamera(CameraManager::CAMERA_TYPE::GAME_CAMERA);
				// render game scene
				RenderPipeline::GetInstance().SetPipeline(RenderPipeline::LIGHT);
				ShaderManager::GetInstance().Use("Algin_3D");
				ShadowMapping::GetInstance().LightPass(ShaderManager::GetInstance().GetShaderProgram());
				AG::PostProcesser::GetInstance().BeginScene();
				COMPONENTMANAGER.LateUpdate();
				// apply post processing
				AG::PostProcesser::GetInstance().EndScene();
				AG::PostProcesser::GetInstance().Apply();
				// render game scene UI
				glDisable(GL_DEPTH_TEST);
				BatchRenderer::GetInstance().Render();
				BatchRenderer::GetInstance().Flush();
				glEnable(GL_DEPTH_TEST);
				RenderOverlayCamera();
#ifndef EXPORT
				AGWINDOW.copyBackBuffer("Game");
				Benchmarker_EndMarker("Game Late Update");
				/* --- GAME SCENE --- */
				//glDisable(GL_FRAMEBUFFER_SRGB);
				Benchmarker_StartMarker("Scene Tool");
				SceneTools::GetInstance().RenderUI();
				Benchmarker_EndMarker("Scene Tool");
				Benchmarker_StartMarker("IMGUI");
				IMGUISYSTEM.Render();
				IMGUISYSTEM.PostRender();
				Benchmarker_EndMarker("IMGUI");
#endif
				AUDIOSYSTEM.Update();
				//glEnable(GL_FRAMEBUFFER_SRGB);


				Benchmarker_StartMarker("Comp and Obj");
				AG::System::ComponentManager::GetInstance().FlushDeletedComponent();
				AG::System::ObjectManager::GetInstance().FlushDeletedObjects();


				//AG::System::SerializationSystem::SaveIfDirty("scene.bin", objMgr, compMgr);
				Benchmarker_EndMarker("Comp and Obj");

				Input::ResetMouseScroll();
				Benchmarker_StartMarker("Swap Buffer");
				AGWINDOW.swapBuffer();
				Benchmarker_EndMarker("Swap Buffer");

				Benchmarker_StartMarker("GLFW Poll");
				glfwPollEvents();
				Benchmarker_EndMarker("GLFW Poll");

				Benchmarker_EndMarker("Update");
				Benchmarker_Update;
				BENCHMARKER.ResetFrustumChecker();



				SystemTest::SceneManager::Process();
			}
			catch (...) {
				std::cerr << "An Exception is thrown!" << std::endl;
			}
		}
	}


	void Application::Free() 
	{
		//PHYSICS.Terminate();
		AI::TreeBuilder::GetInstance().terminate();
		SCRIPTENGINE.terminate();
		IMGUISYSTEM.Free();
		AUDIOSYSTEM.End();
		BatchRenderer::GetInstance().Terminate();
		glfwTerminate();
		FILEWATCHER.~FileWatcher();
		AG::Systems::PhysXManager::GetInstance()->Shutdown();
	}
}
