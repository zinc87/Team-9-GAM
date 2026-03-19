#pragma once

#ifndef EVENT_H
#define EVENT_H

#include "../AGEngine/Core.h"
#include <string>
#include <functional>

namespace AG {

	// Enum for all possible event types in the engine.
	// This is used for dispatching events to the correct handlers.
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	// Enum for event categories. An event can belong to multiple categories.
	// This is useful for filtering events.
	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = 1 << 0,
		EventCategoryInput = 1 << 1,
		EventCategoryKeyboard = 1 << 2,
		EventCategoryMouse = 1 << 3,
		EventCategoryMouseButton = 1 << 4
	};

	// Macro to reduce boilerplate code in event classes.
	// This sets up the static type, event type, and name for an event.
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

// Macro to set the category flags for an event class.
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class AG_API Event
	{
	public:
		virtual ~Event() = default;

		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		// Check if an event belongs to a specific category.
		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	};

	// The EventDispatcher is responsible for sending an event to the correct callback function.
	// It checks the event type and, if it matches the template type, calls the function.
	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		// Dispatches the event to a function if the event type matches.
		// T is the specific Event class (e.g., KeyPressedEvent)
		// F is the function type (usually a lambda or std::function)
		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				// If the types match, call the function and update the event's handled state.
				// In the version I provided
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	// Overload the << operator for easy event logging.
	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}

#endif