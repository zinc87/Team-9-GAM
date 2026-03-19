#include "pch.h"
#include <Mouse_Event.h>
#include <Key_Event.h>

void AGWindow::createWindow(int width, int height, const char* title)
{
	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
		});

	if (!glfwInit())
		exit(EXIT_FAILURE);

	// Configure OpenGL context properties
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // FOR TITLE BAR
#ifdef EXPORT
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#else
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#endif

	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	glfwWindowHint(GLFW_DEPTH_BITS, 24);   // make sure the window has a depth buffer

	// Create a window with an OpenGL context
	window = glfwCreateWindow(width, height, title, NULL, NULL);

	if (!window)
	{
		std::cerr << "Unable to create OpenGL context." << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	// --- NEW ---
	// Set the user pointer to this instance of AGWindow.
	// This allows us to retrieve the object within the static lambda callbacks.
	glfwSetWindowUserPointer(window, this);


	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		glfwTerminate();
		std::exit(EXIT_FAILURE);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_SCISSOR_TEST);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glEnable(GL_ALPHA_TEST);
	//glEnable(GL_FRAMEBUFFER_SRGB);
	//glEnable(GL_ALPHA_TEST);

	monitor = glfwGetPrimaryMonitor();
	mode = glfwGetVideoMode(monitor);
	glfwGetWindowPos(window, &smallWindow.position.first, &smallWindow.position.second);
	smallWindow.size = { width, height };
	fullscreenWindow.size = { mode->width, mode->height };
	fullscreenWindow.position = { 0,0 };

	glfwSetWindowSizeCallback(window, [](GLFWwindow* w, int newWidth, int newHeight) {
		AGWINDOW.getWindowSize().first = newWidth;
		AGWINDOW.getWindowSize().second = newHeight;
		AG::PostProcesser::GetInstance().OnResize(newWidth, newHeight);
		AGWindow* self = static_cast<AGWindow*>(glfwGetWindowUserPointer(w));
		if (self->m_EventCallback) {
			AG::WindowResizeEvent event(newWidth, newHeight);
			self->m_EventCallback(event);
		}
		});

	glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {
		AGWindow* self = static_cast<AGWindow*>(glfwGetWindowUserPointer(w));
		if (self->m_EventCallback) {
			AG::WindowCloseEvent event;
			self->m_EventCallback(event);
		}
		});

	glfwSetKeyCallback(window, [](GLFWwindow* w, int key, /*[[MAYBE_UNUSED]]*/ int, int action, int /*mods*/) {
		AGWindow* self = static_cast<AGWindow*>(glfwGetWindowUserPointer(w));
		if (!self->m_EventCallback) return;

		switch (action) {
		case GLFW_PRESS: {
			AG::KeyPressedEvent event(key, 0); // TODO: repeat count
			self->m_EventCallback(event);
			break;
		}
		case GLFW_RELEASE: {
			AG::KeyReleasedEvent event(key);
			self->m_EventCallback(event);
			break;
		}
		case GLFW_REPEAT: {
			AG::KeyPressedEvent event(key, 1); // TODO: repeat count
			self->m_EventCallback(event);
			break;
		}
		}
		});

	glfwSetCharCallback(window, [](GLFWwindow* w, unsigned int keycode) {
		AGWindow* self = static_cast<AGWindow*>(glfwGetWindowUserPointer(w));
		if (self->m_EventCallback) {
			AG::KeyTypedEvent event(keycode);
			self->m_EventCallback(event);
		}
		});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int /*mods*/) {
		AGWindow* self = static_cast<AGWindow*>(glfwGetWindowUserPointer(w));
		if (!self->m_EventCallback) return;

		switch (action) {
		case GLFW_PRESS: {
			AG::MouseButtonPressedEvent event(button);
			self->m_EventCallback(event);
			break;
		}
		case GLFW_RELEASE: {
			AG::MouseButtonReleasedEvent event(button);
			self->m_EventCallback(event);
			break;
		}
		}
		});

	glfwSetScrollCallback(window, [](GLFWwindow* w, double xOffset, double yOffset) {
		AGWindow* self = static_cast<AGWindow*>(glfwGetWindowUserPointer(w));
		if (self->m_EventCallback) {
			AG::MouseScrolledEvent event((float)xOffset, (float)yOffset);
			self->m_EventCallback(event);
		}
		});

	glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xPos, double yPos) {
		AGWindow* self = static_cast<AGWindow*>(glfwGetWindowUserPointer(w));
		if (self->m_EventCallback) {
			AG::MouseMovedEvent event((float)xPos, (float)yPos);
			self->m_EventCallback(event);
		}
		});


}

void AGWindow::destroyWindow()
{
	glfwDestroyWindow(window);
	for (auto fb : framebuffers)
	{
		glDeleteTextures(1, &fb.second);
	}
	framebuffers.clear();
	glfwTerminate();
}

void AGWindow::moveWindow(int x, int y)
{
	glfwSetWindowPos(window, smallWindow.position.first + x, smallWindow.position.second + y);
}

bool AGWindow::runningWindow()
{
	return !glfwWindowShouldClose(window);
}

void AGWindow::swapBuffer()
{
	glfwSwapBuffers(window);
}

void AGWindow::createFrameBuffer(std::string id_string)
{
	GLuint fb;
	glGenTextures(1, &fb);
	glBindTexture(GL_TEXTURE_2D, fb);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowSize().first, getWindowSize().second, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	framebuffers[id_string] = fb;
}

GLuint AGWindow::getFramebuffer(std::string id_string)
{
	return framebuffers[id_string];
}

void AGWindow::resizeUpdate()
{
	// 1. Toggle Fullscreen
	if (AG::Input::IsKeyTriggered(GLFW_KEY_F11))
	{
		if (!fullscreen_mode)
		{
			int w, h, x, y;
			glfwGetWindowSize(window, &w, &h);
			glfwGetWindowPos(window, &x, &y);

			if (smallWindow.size.first != w || smallWindow.size.second != h)
				smallWindow.size = { w, h };

			if (smallWindow.position.first != x || smallWindow.position.second != y)
				smallWindow.position = { x, y };

		}
		toggleFullScreen();
	}


}

int AGWindow::windowFocused()
{
	return glfwGetWindowAttrib(window, GLFW_FOCUSED);
}

void AGWindow::copyBackBuffer(std::string id_string)
{
	glBindTexture(GL_TEXTURE_2D, framebuffers[id_string]);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, getWindowSize().first, getWindowSize().second, 0);
}

void AGWindow::toggleFullScreen()
{

	fullscreen_mode = !fullscreen_mode;

	if (fullscreen_mode)
	{
		// Enter Fullscreen
		glfwSetWindowMonitor(window, monitor,
			fullscreenWindow.position.first, fullscreenWindow.position.second,
			fullscreenWindow.size.first, fullscreenWindow.size.second,
			mode->refreshRate);


		glViewport(0, 0, fullscreenWindow.size.first, fullscreenWindow.size.second);
	}
	else
	{
		// Enter Windowed
		glfwSetWindowMonitor(window, nullptr,
			smallWindow.position.first, smallWindow.position.second,
			smallWindow.size.first, smallWindow.size.second,
			mode->refreshRate);
		glViewport(0, 0, smallWindow.size.first, smallWindow.size.second);
	}
}

void AGWindow::closeWindow()
{
	glfwSetWindowShouldClose(window, true);
}

void AGWindow::iconifyWindow()
{
	glfwIconifyWindow(window);
}

void AGWindow::enlargeWindow()
{
	glfwMaximizeWindow(window);
}

void AGWindow::restoreWindow()
{
	glfwRestoreWindow(window);
}

void AGWindow::setWindowTitle(std::string /*new_title*/)
{
}

void AGWindow::setWindowSize(int /*x*/, int /*y*/)
{
}

std::pair<int, int> AGWindow::getViewportSize()
{
	return fullscreen_mode ? smallWindow.size : smallWindow.size;
}

std::pair<int, int>& AGWindow::getWindowSize()
{
	return fullscreen_mode ? fullscreenWindow.size : smallWindow.size;
}

std::pair<int, int> AGWindow::getWindowPosition()
{
	return fullscreen_mode ? smallWindow.position : smallWindow.position;
}
