#include "pch.h"
#include "Input.h"
#include "../AGEngine/Application.h"
#include <GLFW/glfw3.h>
#include "AGWindows.h"

namespace AG {

	// --- Public static functions ---

	bool Input::IsKeyPressed(int keycode) { return GetInstance().IsKeyPressedImpl(keycode); }
	bool Input::IsKeyTriggered(int keycode) { return GetInstance().IsKeyTriggeredImpl(keycode); }
	bool Input::IsKeyReleased(int keycode) { return GetInstance().IsKeyReleasedImpl(keycode); }

	bool Input::IsMouseButtonPressed(int button) { return GetInstance().IsMouseButtonPressedImpl(button); }
	bool Input::IsMouseButtonTriggered(int button) { return GetInstance().IsMouseButtonTriggeredImpl(button); }
	bool Input::IsMouseButtonReleased(int button) { return GetInstance().IsMouseButtonReleasedImpl(button); }

	std::pair<float, float> Input::GetMousePosition() { return GetInstance().GetMousePositionImpl(); }
	float Input::GetMouseX() { return GetInstance().GetMouseXImpl(); }
	float Input::GetMouseY() { return GetInstance().GetMouseYImpl(); }
	std::pair<float, float> Input::GetMouseDelta() { return GetInstance().GetMouseDeltaImpl(); }

	void Input::LockInput() { GetInstance().LockInputImpl(); }
	void Input::UnlockInput() { GetInstance().UnlockInputImpl(); }
	bool Input::IsInputLocked() { return GetInstance().IsInputLockedImpl(); }

	void Input::Update() { GetInstance().UpdateImpl(); }


	// --- Private member functions ---

	Input::Input()
	{

		// Initialize key and mouse states
		m_CurrentKeyStates.resize(512, false); // GLFW keys range up to 348, 512 is safe
		m_PreviousKeyStates.resize(512, false);
		m_CurrentMouseStates.resize(8, false); // GLFW mouse buttons are 0-7
		m_PreviousMouseStates.resize(8, false);

		auto window = AGWINDOW.getWindowContext();
		glfwSetScrollCallback(window, Input::ScrollCallback);
	}

	float Input::GetMouseScroll() { return GetInstance().GetMouseScrollImpl(); }

	float Input::GetMouseScrollImpl()
	{
		if (m_InputLocked) return 0.0f;
		return m_MouseScrollDelta;
	}

	void Input::UpdateImpl()
	{

		//m_MouseScrollDelta = 0.0f;

		// Update previous states
		m_PreviousKeyStates = m_CurrentKeyStates;
		m_PreviousMouseStates = m_CurrentMouseStates;
		m_PreviousMousePos = m_CurrentMousePos;

		auto window = AGWINDOW.getWindowContext();

		// Update current key states
		for (int key = 0; key < m_CurrentKeyStates.size(); ++key)
		{
			m_CurrentKeyStates[key] = (glfwGetKey(window, key) == GLFW_PRESS);
		}

		// Update current mouse button states
		for (int button = 0; button < m_CurrentMouseStates.size(); ++button)
		{
			m_CurrentMouseStates[button] = (glfwGetMouseButton(window, button) == GLFW_PRESS);
		}


		// Update current mouse position
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
#ifdef EXPORT
		int screenWidth, screenHeight;
		glfwGetWindowSize(window, &screenWidth, &screenHeight);

		float normX = (float)mouseX / (float)screenWidth;
		float normY = (float)mouseY / (float)screenHeight;

		float standardNDCX = (normX * 2.0f) - 1.0f;

		float standardNDCY = 1.0f - (normY * 2.0f);

		float screen_aspectR = (float)screenWidth / (float)screenHeight;

		float finalX = standardNDCX * screen_aspectR;
		float finalY = standardNDCY;

		m_CurrentMousePos = { finalX, finalY };
#else
		// editor side, for game scene coordinates
		m_CurrentMousePos = AGImGui::GameView::GetMouseNDC();
#endif
	}


	bool Input::IsKeyPressedImpl(int keycode)
	{
		if (m_InputLocked) return false;
		return m_CurrentKeyStates[keycode];
	}

	bool Input::IsKeyTriggeredImpl(int keycode)
	{
		if (m_InputLocked) return false;
		return m_CurrentKeyStates[keycode] && !m_PreviousKeyStates[keycode];
	}

	bool Input::IsKeyReleasedImpl(int keycode)
	{
		if (m_InputLocked) return false;
		return !m_CurrentKeyStates[keycode] && m_PreviousKeyStates[keycode];
	}

	bool Input::IsMouseButtonPressedImpl(int button)
	{
		if (m_InputLocked) return false;
		return m_CurrentMouseStates[button];
	}

	bool Input::IsMouseButtonTriggeredImpl(int button)
	{
		if (m_InputLocked) return false;
		return m_CurrentMouseStates[button] && !m_PreviousMouseStates[button];
	}

	bool Input::IsMouseButtonReleasedImpl(int button)
	{
		if (m_InputLocked) return false;
		return !m_CurrentMouseStates[button] && m_PreviousMouseStates[button];
	}

	std::pair<float, float> Input::GetMousePositionImpl()
	{
		return m_CurrentMousePos;
	}

	float Input::GetMouseXImpl()
	{
		return m_CurrentMousePos.first;
	}

	float Input::GetMouseYImpl()
	{
		return m_CurrentMousePos.second;
	}

	std::pair<float, float> Input::GetMouseDeltaImpl()
	{
		return { m_CurrentMousePos.first - m_PreviousMousePos.first,
				 m_CurrentMousePos.second - m_PreviousMousePos.second };
	}

	void Input::LockInputImpl()
	{
		m_InputLocked = true;
	}
	void Input::UnlockInputImpl()
	{
		m_InputLocked = false;
	}
	bool Input::IsInputLockedImpl()
	{
		return m_InputLocked;
	}

	void Input::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		// We usually care about yOffset (vertical scroll)
		GetInstance().m_MouseScrollDelta = (float)yOffset;
		(void)window;
		(void)xOffset;
	}

}