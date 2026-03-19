/************************************************************************/
/*!
	\file   Logger.h
	\author Bin Wakif Zulfami Ashrafi, b.zulfamiashrafi, 2301298
	\email  b.zulfamiashrafi@digipen.edu
	\date   27 February 2025
	\brief  This file contains declarations for Logger.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/************************************************************************/

#pragma once
#include "../Header/pch.h"


#pragma warning(push)
#pragma warning(disable : 6285)
#pragma warning(disable : 26451)
#pragma warning(disable : 26498)
#pragma warning(disable : 26800)
#include "spdlog/spdlog.h"
#include "spdlog/sinks/base_sink.h"
#pragma warning(pop)

/************************************************************************/
/*!
	\brief
	The BS namespace contains all engine-related systems and utilities.
*/
/************************************************************************/
namespace AG {
	namespace System {
		/************************************************************************/
		/*!
			\brief
			A struct that represents a single log message and its corresponding color.
		*/
		/************************************************************************/
		struct LogEntry {
			std::string m_message;
			ImVec4 m_color;
		};

		/************************************************************************/
		/*!
			\brief
			Handles logging display within ImGui using colored text entries.
		*/
		/************************************************************************/
		class ImGuiLogger {
		public:
			using VLog = std::vector<LogEntry>;
		public:
			/************************************************************************/
			/*!
				\brief
				Adds a new log entry to the ImGui logger.

				\param log
				The message string to log.

				\param color
				The ImVec4 color used to render the log entry.
			*/
			/************************************************************************/
			void AddLog(const std::string& log, const ImVec4& color);

			/************************************************************************/
			/*!
				\brief
				Clears all log entries from the ImGui logger.
			*/
			/************************************************************************/
			void ClearLog();

			/************************************************************************/
			/*!
				\brief
				Gets a pointer to the auto-scroll flag.

				\return
				A pointer to the auto-scroll boolean flag.
			*/
			/************************************************************************/
			bool* GetScrollPtr();

			/************************************************************************/
			/*!
				\brief
				Renders the ImGui log window.
			*/
			/************************************************************************/
			void Draw();

		private:
			VLog m_logs;
			bool m_autoScroll{ true };
			bool m_scrollToBottom{ false };
		};

		/************************************************************************/
		/*!
			\brief
			Centralized logger manager handling both ImGui and spdlog loggers.
		*/
		/************************************************************************/
		class Logger : public Pattern::ISingleton<Logger>
		{
		public:
			using LoggerPtr = std::shared_ptr<spdlog::logger>;
			using GuiLoggerPtr = std::shared_ptr<ImGuiLogger>;

		public:
			/************************************************************************/
			/*!
				\brief
				Constructs for Logger.
			*/
			/************************************************************************/
			Logger();

			/************************************************************************/
			/*!
				\brief
				Destructor for Logger.
			*/
			/************************************************************************/
			~Logger();

			/************************************************************************/
			/*!
				\brief
				Returns the activation state of logging.

				\return
				True if logging is enabled, false otherwise.
			*/
			/************************************************************************/
			bool GetActive() { return m_isActive; }

			/************************************************************************/
			/*!
				\brief
				Sets the activation state of logging.

				\param state
				True to enable logging, false to disable.
			*/
			/************************************************************************/
			void SetActive(bool state) { m_isActive = state; }

			/************************************************************************/
			/*!
				\brief
				Gets the main ImGui logger instance.

				\return
				A reference to the GUI logger instance.
			*/
			/************************************************************************/
			GuiLoggerPtr& GetImGuiLogger();

			/************************************************************************/
			/*!
				\brief
				Gets the input-specific ImGui logger.

				\return
				A reference to the GUI logger for input.
			*/
			/************************************************************************/
			GuiLoggerPtr& GetImGuiInputLogger();

			/************************************************************************/
			/*!
				\brief
				Gets the graphics-specific ImGui logger.

				\return
				A reference to the GUI logger for graphics.
			*/
			/************************************************************************/
			GuiLoggerPtr& GetImGuiGraphicsLogger();

			/************************************************************************/
			/*!
				\brief
				Gets the core logger used by the engine.

				\return
				A reference to the core spdlog logger.
			*/
			/************************************************************************/
			LoggerPtr& GetCoreLogger();

			/************************************************************************/
			/*!
				\brief
				Gets the client logger for game/application code.

				\return
				A reference to the client spdlog logger.
			*/
			/************************************************************************/
			LoggerPtr& GetClientLogger();

			/************************************************************************/
			/*!
				\brief
				Gets the logger for input-related logs.

				\return
				A reference to the input spdlog logger.
			*/
			/************************************************************************/
			LoggerPtr& GetInputLogger();

			/************************************************************************/
			/*!
				\brief
				Gets the logger for graphics-related logs.

				\return
				A reference to the graphics spdlog logger.
			*/
			/************************************************************************/
			LoggerPtr& GetGraphicsLogger();

		private:
			bool			m_isActive;

			GuiLoggerPtr	m_ImGuiLogger;
			GuiLoggerPtr	m_ImGuiInputLogger;
			GuiLoggerPtr	m_ImGuiGraphicsLogger;

			LoggerPtr		m_CoreLogger;
			LoggerPtr		m_ClientLogger;
			LoggerPtr		m_InputLogger;
			LoggerPtr		m_GraphicsLogger;
		};

		/************************************************************************/
		/*!
			\brief
			Custom spdlog sink that forwards log output to an ImGuiLogger instance.
		*/
		/************************************************************************/
		class ImGuiSink : public spdlog::sinks::base_sink<std::mutex> {
		public:
			using LogMsg = spdlog::details::log_msg;
		public:
			/************************************************************************/
			/*!
				\brief
				Constructor for ImGuiSink.

				\param logger
				The ImGuiLogger instance to which logs will be forwarded.
			*/
			/************************************************************************/
			ImGuiSink(ImGuiLogger& logger) : imgui_logger(logger) {}

		protected:
			/************************************************************************/
			/*!
				\brief
				Overrides the spdlog sink_it_ to forward the log message to ImGui.

				\param msg
				The log message to be processed.
			*/
			/************************************************************************/
			void sink_it_(const LogMsg& msg) override;

			/************************************************************************/
			/*!
				\brief
				Flush override (no-op for ImGuiSink).
			*/
			/************************************************************************/
			void flush_() override {}


		private:
			ImGuiLogger& imgui_logger;
		};
	}
}


#define ISDEBUG 0
#ifdef DEBUG
	#define ISDEBUG 1
#endif // DEBUG

#define LOGGER AG::System::Logger::GetInstance()

#define AG_DEBUG_FATAL(...)	ISDEBUG ? LOGGER.GetCoreLogger()->critical(__VA_ARGS__) : (void)0
#define AG_DEBUG_ERROR(...)	ISDEBUG ? LOGGER.GetCoreLogger()->error(__VA_ARGS__) : (void)0
#define AG_DEBUG_WARN(...)	ISDEBUG ? LOGGER.GetCoreLogger()->warn(__VA_ARGS__) : (void)0
#define AG_DEBUG_INFO(...)	ISDEBUG ? LOGGER.GetCoreLogger()->info(__VA_ARGS__) : (void)0
#define AG_DEBUG_TRACE(...)	ISDEBUG ? LOGGER.GetCoreLogger()->trace(__VA_ARGS__) : (void)0

// Input Log Macros
#define AG_INPUT_FATAL(...)	LOGGER.GetActive() ? LOGGER.GetInputLogger()->critical(__VA_ARGS__) : (void)0
#define AG_INPUT_ERROR(...)	LOGGER.GetActive() ? LOGGER.GetInputLogger()->error(__VA_ARGS__) : (void)0
#define AG_INPUT_WARN(...)	LOGGER.GetActive() ? LOGGER.GetInputLogger()->warn(__VA_ARGS__) : (void)0
#define AG_INPUT_INFO(...)	LOGGER.GetActive() ? LOGGER.GetInputLogger()->info(__VA_ARGS__) : (void)0
#define AG_INPUT_TRACE(...)	LOGGER.GetActive() ? LOGGER.GetInputLogger()->trace(__VA_ARGS__) : (void)0

// Graphics Log Macros
#define AG_GRAPHICS_FATAL(...)	LOGGER.GetActive() ? LOGGER.GetGraphicsLogger()->critical(__VA_ARGS__) : (void)0
#define AG_GRAPHICS_ERROR(...)	LOGGER.GetActive() ? LOGGER.GetGraphicsLogger()->error(__VA_ARGS__) : (void)0
#define AG_GRAPHICS_WARN(...)	LOGGER.GetActive() ? LOGGER.GetGraphicsLogger()->warn(__VA_ARGS__) : (void)0
#define AG_GRAPHICS_INFO(...)	LOGGER.GetActive() ? LOGGER.GetGraphicsLogger()->info(__VA_ARGS__) : (void)0
#define AG_GRAPHICS_TRACE(...)	LOGGER.GetActive() ? LOGGER.GetGraphicsLogger()->trace(__VA_ARGS__) : (void)0


// Core Log Macros
#define AG_CORE_FATAL(...)	LOGGER.GetActive() ? LOGGER.GetCoreLogger()->critical(__VA_ARGS__) : (void)0
#define AG_CORE_ERROR(...)	LOGGER.GetActive() ? LOGGER.GetCoreLogger()->error(__VA_ARGS__) : (void)0
#define AG_CORE_WARN(...)	LOGGER.GetActive() ? LOGGER.GetCoreLogger()->warn(__VA_ARGS__) : (void)0
#define AG_CORE_INFO(...)	LOGGER.GetActive() ? LOGGER.GetCoreLogger()->info(__VA_ARGS__) : (void)0
#define AG_CORE_TRACE(...)	LOGGER.GetActive() ? LOGGER.GetCoreLogger()->trace(__VA_ARGS__) : (void)0

/* We dont deserve this yet
// Client Log Macros
#define AG_FATAL(...)	LOGGER.GetActive() ? LOGGER.GetClientLogger()->critical(__VA_ARGS__) : (void)0
#define AG_ERROR(...)	LOGGER.GetActive() ? LOGGER.GetClientLogger()->error(__VA_ARGS__) : (void)0
#define AG_WARN(...)	LOGGER.GetActive() ? LOGGER.GetClientLogger()->warn(__VA_ARGS__) : (void)0
#define AG_INFO(...)	LOGGER.GetActive() ? LOGGER.GetClientLogger()->info(__VA_ARGS__) : (void)0
#define AG_TRACE(...)	LOGGER.GetActive() ? LOGGER.GetClientLogger()->trace(__VA_ARGS__) : (void)0
*/
