#pragma once

#include "../AGEngine/Core.h"
#include <utility>
#pragma warning(push)
#pragma warning(disable: 4251)
namespace AG {

	class AG_API Input
	{
	public:
		// Public API
		static bool IsKeyPressed(int keycode);
		static bool IsKeyTriggered(int keycode);
		static bool IsKeyReleased(int keycode);

		static bool IsMouseButtonPressed(int button);
		static bool IsMouseButtonTriggered(int button);
		static bool IsMouseButtonReleased(int button);

		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMouseDelta();

		static void LockInput();
		static void UnlockInput();
		static bool IsInputLocked();
		static float GetMouseScroll();
		static void ResetMouseScroll() { GetInstance().ResetMouseScrollImpl(); }
		// Called once per frame by the application loop
		static void Update();

	private:
		// Implementation-specific functions
		bool IsKeyPressedImpl(int keycode);
		bool IsKeyTriggeredImpl(int keycode);
		bool IsKeyReleasedImpl(int keycode);

		bool IsMouseButtonPressedImpl(int button);
		bool IsMouseButtonTriggeredImpl(int button);
		bool IsMouseButtonReleasedImpl(int button);


		std::pair<float, float> GetMousePositionImpl();
		float GetMouseXImpl();
		float GetMouseYImpl();
		std::pair<float, float> GetMouseDeltaImpl();

		void ClearScroll() { m_MouseScrollDelta = 0.0f; }
		void LockInputImpl();
		void UnlockInputImpl();
		bool IsInputLockedImpl();
		void ResetMouseScrollImpl() { m_MouseScrollDelta = 0.0f; }
		void UpdateImpl();

		// Singleton instance getter
		static Input& GetInstance()
		{
			static Input instance;
			return instance;
		}

		Input(); // Private constructor

		// Member variables
		bool m_InputLocked = false;

		std::vector<bool> m_CurrentKeyStates;
		std::vector<bool> m_PreviousKeyStates;

		std::vector<bool> m_CurrentMouseStates;
		std::vector<bool> m_PreviousMouseStates;

		std::pair<float, float> m_CurrentMousePos;
		std::pair<float, float> m_PreviousMousePos;

		float GetMouseScrollImpl();
		static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

		// Add to member variables
		float m_MouseScrollDelta = 0.0f;
	};
}
#pragma warning(pop)