/*!*****************************************************************************
\file GameStateManager.cpp
\author Zulfami Ashrafi Bin Wakif
\date 20/1/2025 (MM/DD/YYYY)

\brief Function to control the game scene
*******************************************************************************/
#pragma once
#include "pch.h"

namespace AG {
	namespace System {
		void SceneManager::Init() {
			AddScene(std::make_unique<Scene1>());
		}

		void SceneManager::Start() {
			if (scenes.size() == 0) return;
			if (hasStarted) return;

			scenes[currSceneStr]->Start();
			hasStarted = true;
		}

		void SceneManager::Update() {
			if (scenes.size() == 0) return;

			scenes[currSceneStr]->Update();
		}

		void SceneManager::End() {
			if (scenes.size() == 0) return;
			if (!hasEnded) return;

			scenes[currSceneStr]->End();
			hasEnded = false;
		}

		void SceneManager::AddScene(std::unique_ptr<IScene> scene) {
			scenes[scene->sceneName] = std::move(scene);

			if (scenes.size() <= 1) {
				currSceneStr = scenes.begin()->first;
			}
		}

		void SceneManager::ChangeState(int state) {
			if (state <= 0 || state >= scenes.size()) return;

			currSceneInt = state;
			auto it = scenes.begin();
			std::advance(it, state);
			currSceneStr = it->first;

			hasEnded = true;
		}

		void SceneManager::ChangeState(std::string state) {
			if (scenes[state] == nullptr) return;
			currSceneStr = state;
			currSceneInt = static_cast<int>(std::distance(scenes.begin(), scenes.find(state)));

			hasEnded = true;
		}

		void SceneManager::ReloadState() {
			hasStarted = false;
		}

		void SceneManager::EndState() {
			hasEnded = true;
		}


		SceneManager::SceneState SceneManager::GetSceneState() const {
			return currState;
		}

		void SceneManager::SetSceneState(const SceneState state) {
			currState = state;
		}

		bool SceneManager::IsSceneRunning() const {
			return 
			SCENESTATEMANAGER.GetSceneState() != AG::System::SceneManager::STOPPED &&
			SCENESTATEMANAGER.GetSceneState() != AG::System::SceneManager::NONE ? 
			true : false;
		}

		std::string SceneManager::GetSceneStateStr() const {
			switch (currState)
			{
			case SceneState::NONE:
				return "None";
			case SceneState::PLAYING:
				return "Playing";
			case SceneState::PAUSED:
				return "Paused";
			case SceneState::STOPPED:
				return "Stopped";
			default:
				return "None";
			}
		}
		
		void SceneManager::AddFuncOnStart(std::function<void()> func)
		{
			OnStart.push(func);
		}

		void SceneManager::AddFuncOnPause(std::function<void()> func)
		{
			OnPause.push(func);
		}

		void SceneManager::AddFuncOnEnd(std::function<void()> func)
		{
			OnEnd.push(func);
		}

		void SceneManager::RunOnStart()
		{
			while (!OnStart.empty()) {
				OnStart.top()();   // call
				OnStart.pop();     // remove
			}
		}

		void SceneManager::RunOnPause()
		{
			while (!OnPause.empty()) {
				OnPause.top()();   // call
				OnPause.pop();     // remove
			}
		}

		void SceneManager::RunOnEnd()
		{
			while (!OnEnd.empty()) {
				OnEnd.top()();   // call
				OnEnd.pop();     // remove
			}
		}
	}
}

