/*!*****************************************************************************
\file Scene1.h
\author Zulfami Ashrafi Bin Wakif
\date 18/2/2025 (MM/DD/YYYY)

\brief Test Scene
*******************************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace System {
		class Scene1 : public IScene {
		public:
			Scene1();

			// Inherited via Scene
			void Start() override;
			void Update() override;
			void End() override;
		};
	}
}