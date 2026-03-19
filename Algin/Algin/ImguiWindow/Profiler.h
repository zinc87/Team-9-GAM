#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace AGImGui {
		class Profiler : public AGImGui::IImguiWindow {
		public:

		public:
			Profiler() {}
			~Profiler() override {}
			void Render() override;

			ImColor GetColor(const std::string& name);
			float GetCPUUsage();
			void DrawPerformanceBlock(std::string name, ImVec2 pos, ImVec2 size, ImColor color);

		private:
			double last_update = 0.0;
			double update_gaps = 0.5;
			double delta_time = 0.0;
			double total_update_time = 0.0;
			int fps = 0;
			bool freeze = false;
			std::map<std::string, std::pair<double, double>> imgui_buffer;

			std::unordered_map<std::string, std::deque<float>> history_buffer;
			std::deque<float> cpu_usage;
			const size_t max_history = 10; 


		};
	}
}