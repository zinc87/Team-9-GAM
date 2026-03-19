#pragma once
#include "Core.h"
#include "Application_Event.h" // <-- ADD THIS (It includes Event.h too)
#include "../System/Key_Event.h"


#include "Player.h"
#define WIDTH 1600
#define HEIGHT 900

namespace AG
{
	class DebugMonitor;
	class AG_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Init();
		void Update();
		void Free();

		// This function will be the main entry point for all events.
		void OnEvent(Event& e);

		// Static getter to access the single instance of the application
		static Application& Get() { return *s_Instance; }

		// Public accessors for the event log
		const std::deque<std::string>& GetEventLog() const; /*{ return m_EventLog; }*/
		void ClearEventLog();/* { m_EventLog.clear(); }*/

	private:

		struct Impl;

		// Specific event handler functions
		bool OnWindowClose(Event& e);

#pragma warning(push)
#pragma warning(disable : 4251) // Disables C4251 for this line
		std::unique_ptr<Impl> m_pimpl;
#pragma warning(pop)

		//bool OnWindowResize(WindowResizeEvent& e);

		//bool OnKeyPressed(KeyPressedEvent& e); // <-- ADD THIS

		//std::deque<std::string> m_EventLog;
		//const size_t m_MaxLogSize = 100; // Store the last 100 events
	private:
		// A static pointer to hold the single instance of the Application
		static Application* s_Instance;

		/*std::unique_ptr<DebugMonitor> m_debugMonitor;*/

	};

	// Declaration for the global accessor function
	Application* CreateApplication();
}
