/*!*****************************************************************************
\file Scene1.cpp
\author Zulfami Ashrafi Bin Wakif
\date 18/2/2025 (MM/DD/YYYY)

\brief Test Scene
*******************************************************************************/
#pragma once
#include "pch.h"

namespace AG {
	namespace System {
		IObject* _temp;

		Scene1::Scene1() {
			sceneName = "Scene1";
		}

		void Scene1::Start() {
			std::cout << sceneName << " is Running" << std::endl;
		}

		void Scene1::Update() {

		}

		void Scene1::End() {

		}
	}
}