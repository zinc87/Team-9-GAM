/************************************************************************/
/*!
	\file   Logger.cpp
	\author Bin Wakif Zulfami Ashrafi, b.zulfamiashrafi, 2301298
	\email  b.zulfamiashrafi@digipen.edu
	\date   27 February 2025
	\brief  This file contains definitions for Logger.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/************************************************************************/
#include "pch.h"


namespace AG {
	namespace System {
#pragma region Logger Defination
		Logger::Logger() : m_isActive{ true } {

			m_ImGuiLogger = std::make_shared<ImGuiLogger>();
			auto imgui_sinks = std::make_shared<ImGuiSink>(*m_ImGuiLogger);

			m_ImGuiInputLogger = std::make_shared<ImGuiLogger>();
			auto imgui_input_sinks = std::make_shared<ImGuiSink>(*m_ImGuiInputLogger);

			m_ImGuiGraphicsLogger = std::make_shared<ImGuiLogger>();
			auto imgui_graphics_sinks = std::make_shared<ImGuiSink>(*m_ImGuiGraphicsLogger);

			// Create sinks for logging
			std::vector<spdlog::sink_ptr> core_sinks;
			core_sinks.push_back(imgui_sinks);

			std::vector<spdlog::sink_ptr> client_sinks;
			client_sinks.push_back(imgui_sinks);

			std::vector<spdlog::sink_ptr> input_sinks;
			input_sinks.push_back(imgui_input_sinks);

			std::vector<spdlog::sink_ptr> graphics_sinks;
			graphics_sinks.push_back(imgui_graphics_sinks);

			m_CoreLogger = std::make_shared<spdlog::logger>("Engine", core_sinks.begin(), core_sinks.end());
			m_ClientLogger = std::make_shared<spdlog::logger>("APP", client_sinks.begin(), client_sinks.end());
			m_InputLogger = std::make_shared<spdlog::logger>("INPUT", input_sinks.begin(), input_sinks.end());
			m_GraphicsLogger = std::make_shared<spdlog::logger>("Graphics", graphics_sinks.begin(), graphics_sinks.end());

			m_CoreLogger->set_level(spdlog::level::trace);
			m_ClientLogger->set_level(spdlog::level::trace);
			m_InputLogger->set_level(spdlog::level::trace);
			m_GraphicsLogger->set_level(spdlog::level::trace);

			m_CoreLogger->set_pattern("%^[%T] %n: %v%$");
			m_ClientLogger->set_pattern("%^[%T] %n: %v%$");
			m_InputLogger->set_pattern("%^[%T] %n: %v%$");
			m_GraphicsLogger->set_pattern("%^[%T] %n: %v%$");
		}

		Logger::~Logger()
		{
		}

		std::shared_ptr<ImGuiLogger>& Logger::GetImGuiLogger()
		{
			return m_ImGuiLogger;
		}

		std::shared_ptr<ImGuiLogger>& Logger::GetImGuiInputLogger()
		{
			return m_ImGuiInputLogger;
		}

		std::shared_ptr<ImGuiLogger>& Logger::GetImGuiGraphicsLogger()
		{
			return m_ImGuiGraphicsLogger;
		}

		std::shared_ptr<spdlog::logger>& Logger::GetCoreLogger()
		{
			return m_CoreLogger;
		}

		std::shared_ptr<spdlog::logger>& Logger::GetClientLogger()
		{
			return m_ClientLogger;
		}

		std::shared_ptr<spdlog::logger>& Logger::GetInputLogger()
		{
			return m_InputLogger;
		}

		std::shared_ptr<spdlog::logger>& Logger::GetGraphicsLogger()
		{
			return m_GraphicsLogger;
		}
#pragma endregion

#pragma region ImGuiLogger Defination
		void ImGuiLogger::AddLog(const std::string& log, const ImVec4& color) {
			m_logs.push_back({ log, color });

			if (m_autoScroll) {
				m_scrollToBottom = true;
			}
		}

		void ImGuiLogger::ClearLog() {
			m_logs.clear();
		}

		bool* ImGuiLogger::GetScrollPtr() {
			return &m_autoScroll;
		}

		void ImGuiLogger::Draw() {
			for (const LogEntry& log : m_logs) {
				ImGui::TextColored(log.m_color, "%s", log.m_message.c_str());
			}

			// Auto-scroll logic
			if (m_scrollToBottom && m_autoScroll) {
				ImGui::SetScrollHereY(1.0f);  // Scroll to bottom
			}

			m_scrollToBottom = false;
		}
#pragma endregion

#pragma region ImGuiSink Defination
		void ImGuiSink::sink_it_(const LogMsg& msg) {
			spdlog::memory_buf_t formatted;
			base_sink<std::mutex>::formatter_->format(msg, formatted);

			ImVec4 color;
			switch (msg.level) {
			case spdlog::level::info:
				color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green for info
				break;
			case spdlog::level::warn:
				color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow for warn
				break;
			case spdlog::level::err:
				color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for error
				break;
			case spdlog::level::critical:
				color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for critical
				break;
			default:
				color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White for others
				break;
			}

			imgui_logger.AddLog(fmt::to_string(formatted), color);
		}
#pragma endregion
	}
}