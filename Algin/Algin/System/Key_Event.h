#pragma once

#include "Event.h"

namespace AG {

	class AG_API KeyEvent : public Event
	{
	public:
		int GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(int keycode)
			: m_KeyCode(keycode) {
		}

		int m_KeyCode;
	};

	class AG_API KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int keycode, int repeatCount)
			: KeyEvent(keycode), m_RepeatCount(repeatCount) {
		}

		int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			return "KeyPressedEvent: " + std::to_string(m_KeyCode) + " (" + std::to_string(m_RepeatCount) + " repeats)";
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

	class AG_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keycode)
			: KeyEvent(keycode) {
		}

		std::string ToString() const override
		{
			return "KeyReleasedEvent: " + std::to_string(m_KeyCode);
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class AG_API KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(int keycode)
			: KeyEvent(keycode) {
		}

		std::string ToString() const override
		{
			return "KeyTypedEvent: " + std::to_string(m_KeyCode);
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}
