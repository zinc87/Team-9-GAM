/************************************************************************/
/*!
	\file   ConsoleIMGUI.h
	\author Bin Wakif Zulfami Ashrafi, b.zulfamiashrafi, 2301298
	\email  b.zulfamiashrafi@digipen.edu
	\date   8 November 2024
	\brief  This file contains declarations for ConsoleIMGUI to display
			the debug console.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/************************************************************************/

#pragma once
#include "../Header/pch.h"

/************************************************************************/
/*!
	\brief
	The BS namespace contains all engine-related systems and utilities.
*/
/************************************************************************/
namespace AG {
	/************************************************************************/
	/*!
		\brief
		Namespace containing all ImGui-based UI tools and windows.
	*/
	/************************************************************************/
	namespace AGImGui {
		/************************************************************************/
		/*!
			\brief
			Displays the debug console using ImGui.
		*/
		/************************************************************************/
		class Console : public IImguiWindow, public Pattern::ISingleton<Console>
		{
		public:
			/************************************************************************/
			/*!
				\brief
				Constructor for Console..
			*/
			/************************************************************************/
			Console() : AGImGui::IImguiWindow({ 200.f, 50.f }, "Console") {}

			/************************************************************************/
			/*!
				\brief
				Renders the Console window using ImGui.
			*/
			/************************************************************************/
			virtual void		Render() override;

			/************************************************************************/
			/*!
				\brief
				Override function for per-frame logic (unused).
			*/
			/************************************************************************/
			virtual void Update() {}

			/************************************************************************/
			/*!
				\brief
				Override function to free any console resources (unused).
			*/
			/************************************************************************/
			virtual void Free() {}
		};
	}
#define CONSOLE_IMGUI ImguiUI::Console::GetInstance()
}