/*****************************************************************//**
 * \file   SceneManager.h
 * \brief
 *
 * \author Zulfami Ashrafi
 * \date   November 2025
 *********************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace SystemTest {
		class SceneManager {
		public:
			enum class CommandType {
				LoadScene,
				UnloadScene,
				RestartScene
			};

			struct Command
			{
				CommandType type;
				std::string sceneFile;
			};

			static void LoadScene(const std::string& file);
			static void Process();
			static std::string GetSceneName();
			static void setSceneName(std::string name);

		private:
			static std::string sceneName;

			static std::queue<Command>& getQueue() {
				static std::queue<Command> queue;
				return queue;
			}

			static void ProcessLoadScene(const std::string& file);
		};
	}
}