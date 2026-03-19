/*!*****************************************************************************
\file GameStateManager.h
\author Zulfami Ashrafi Bin Wakif
\date 20/1/2025 (MM/DD/YYYY)

\brief Function to control the game scene
*******************************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace System {
		class SceneManager : public Pattern::ISingleton<SceneManager> {
		public:
			enum SceneState {
				NONE,
				PLAYING,
				PAUSED,
				STOPPED
			};

		public:
			void Init();
			void Start();
			void Update();
			void End();

			void AddScene(std::unique_ptr<IScene> scene);
			void ChangeState(int state);
			void ChangeState(std::string state);
			void ReloadState();
			void EndState();

			SceneState GetSceneState() const;
			void SetSceneState(const SceneState state);
			bool IsSceneRunning() const;

			// Debug function
			std::string GetSceneStateStr() const;

			void AddFuncOnStart(std::function<void()>);
			void AddFuncOnPause(std::function<void()>);
			void AddFuncOnEnd(std::function<void()>);

			void RunOnStart();
			void RunOnPause();
			void RunOnEnd();

		private:
			// Can change to map if want string
			std::map<std::string, std::unique_ptr<IScene>> scenes;
			std::string currSceneStr;
			int currSceneInt = 0;
			bool hasStarted = false;
			bool hasEnded = false;

			SceneState currState = SceneState::STOPPED;

			// Function Ptr to add at certain point of frame
			std::stack<std::function<void()>> OnStart;
			std::stack<std::function<void()>> OnPause;
			std::stack<std::function<void()>> OnEnd;
		};
	}
#define SCENESTATEMANAGER AG::System::SceneManager::GetInstance()
}