#pragma once
#include "../ImguiWindow/IImguiWindow.h"

namespace AG {
	namespace AGImGui {
		// EventHistory now correctly inherits from your IImguiWindow interface
		class EventHistory : public IImguiWindow
		{
		public:
			// Call the base class constructor with a default size and name
			EventHistory() : IImguiWindow({ 0.f, 0.f }, "Event History") {}
			~EventHistory() override = default;

			// Implement the pure virtual Render function
			void Render() override;
		};
	}
}