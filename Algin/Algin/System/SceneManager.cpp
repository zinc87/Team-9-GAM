/*****************************************************************//**
 * \file   SceneManager.cpp
 * \brief
 *
 * \author Zulfami Ashrafi
 * \date   November 2025
 *********************************************************************/
#include "pch.h"
#include "SceneManager.h"

namespace AG {
	namespace SystemTest {
		std::string SceneManager::sceneName;

		void SceneManager::LoadScene(const std::string& file)
		{
			getQueue().push({CommandType::LoadScene, file});
		}

		void SceneManager::Process()
		{
			auto& q = getQueue();
			while (!q.empty()) {
				const Command& cmd = q.front();
				switch (cmd.type)
				{
				case CommandType::LoadScene:
					ProcessLoadScene(cmd.sceneFile);
					break;

				case CommandType::UnloadScene:
					// Todo
					break;

				case CommandType::RestartScene:
					// Todo
					break;


				default:
					break;
				}
				q.pop();
			}
		}

		std::string SceneManager::GetSceneName()
		{
			std::string result = sceneName.substr(sceneName.find_last_of("\\") + 1);
			return result;
		}

		void SceneManager::setSceneName(std::string name)
		{
			sceneName = name;
		}

		void SceneManager::ProcessLoadScene(const std::string& file)
		{
			AG_CORE_INFO("[SceneChangeQueue] Loading scene : ", file);
			if (!SERIALIZATIONSYSTEM.loadSceneJson(file))
			{
				AG_CORE_WARN("[SceneManager] JSON load failed, attempting Text format fallback for: {}", file);
				if (!SERIALIZATIONSYSTEM.LoadScene(file, OBJECTMANAGER, COMPONENTMANAGER))
				{
					AG_CORE_ERROR("[SceneManager] Failed to load scene in both JSON and Text formats: {}", file);
				}
			}
		}
	}
}