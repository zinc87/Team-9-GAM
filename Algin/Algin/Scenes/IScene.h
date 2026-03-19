/*!*****************************************************************************
\file IScene.h
\author Zulfami Ashrafi Bin Wakif
\date 20/1/2025 (MM/DD/YYYY)

\brief Interface for scenes
*******************************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace System {
		class IScene {
		public:
			virtual ~IScene() = default;
			virtual void Start() = 0;
			virtual void Update() = 0;
			virtual void End() = 0;
			std::string sceneName;
		};
	}
}