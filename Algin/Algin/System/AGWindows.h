#pragma once
// Remove #include "../Header/pch.h" if it's there
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <GLFW/glfw3.h>
#include "../Pattern/ISingleton.h"
#include "../System/Event.h" // <-- ADD THIS

#define AGWINDOW AGWindow::GetInstance()

struct WindowSpecs
{
	std::pair<int, int> size;
	std::pair<int, int> position;
};

class AGWindow : public AG::Pattern::ISingleton<AGWindow>
{
public:

	// Define a type for our event callback function
	using EventCallbackFn = std::function<void(AG::Event&)>;

	void createWindow(int width = 1600, int height = 900, const char* title = "Algin");
	void destroyWindow();
	void moveWindow(int x, int y);
	bool runningWindow();
	void swapBuffer();
	void createFrameBuffer(std::string id_string);
	GLuint getFramebuffer(std::string id_string);
	void resizeUpdate();
	int windowFocused();
	void copyBackBuffer(std::string id_string);
	void setAspectRatio(int x, int y) { aspectRatio = { x,y }; };
	std::pair<int, int> getAspectRatio() { return aspectRatio; }

	void toggleFullScreen();
	void closeWindow();
	void iconifyWindow();
	void enlargeWindow();
	void restoreWindow();

	GLFWwindow* getWindowContext() { return window; }
	void setWindowTitle(std::string new_title);
	void setWindowSize(int x, int y);
	std::pair<int, int> getViewportSize();
	std::pair<int, int>& getWindowSize();
	std::pair<int, int> getWindowPosition();

	// Function to set the event callback from another class (like Application)
	void SetEventCallback(const EventCallbackFn& callback) { m_EventCallback = callback; }

	inline static bool cursor_disabled = true;

private:
	GLFWwindow*									window{};
	GLFWmonitor*								monitor{};
	const GLFWvidmode*							mode{};
	WindowSpecs									smallWindow{};
	WindowSpecs									bigWindow{};
	WindowSpecs									fullscreenWindow{};
	const char*									window_title{};
	bool										fullscreen_mode{};
	std::unordered_map<std::string, GLuint>		framebuffers;
	std::pair<int, int>							aspectRatio{ 16,9 };

	EventCallbackFn m_EventCallback;

};