/*!*****************************************************************************
\file IImguiWindow.h
\author Zulfami Ashrafi Bin Wakif
\date 19/2/2025 (MM/DD/YYYY)

\brief interface for individual imgui window
*******************************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace AGImGui {
		class IImguiWindow {
		public:
			using FPair = std::pair<float, float>;

		public:
			//=== Constructor ===//
			IImguiWindow() : m_size{ 0.f,0.f }, m_name{} {}
			IImguiWindow(FPair size, std::string name = "") : m_size{size}, m_name{name} {}

			virtual ~IImguiWindow() {};
			virtual void Render() = 0;

			//=== Accessor ===//
			std::string GetName() { return m_name; }

		protected:
			FPair m_size{};
			std::string m_name{};
		};
	}
}