#pragma once
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "../Log.h"

#ifndef IMGUI_DISABLE
#include "../lostImGui.h"
#endif

enum WindowAttribs
{
	// Sets if the window currently has user focus
	// Equivalent of GLFW_FOCUSED
	LOST_WINDOW_FOCUSED = GLFW_FOCUSED,

	// Sets if the window is currently minimized/iconified
	// Equivalent of GLFW_ICONIFIED
	LOST_WINDOW_ICONIFIED = GLFW_ICONIFIED,

	// Sets if the window is visible at all
	// Equivalent of GLFW_VISIBILE
	LOST_WINDOW_VISIBLE = GLFW_VISIBLE,

	// Sets if the window is currently resizable
	// Equivalent of GLFW_RESIZABLE
	LOST_WINDOW_RESIZABLE = GLFW_RESIZABLE,

	// Sets if the window has any decorations like a border or a close widget
	// Equivalent of GLFW_DECORATED
	LOST_WINDOW_DECORATED = GLFW_DECORATED
};

namespace lost
{

	struct WindowContext
	{
		WindowContext(GLFWwindow* window)
		{
			glfwWindow = window;
			camera = new _Camera();

			glfwGetWindowSize(glfwWindow, &width, &height);

			camera->_updatePerspective(width, height);
		};

		~WindowContext()
		{
			delete camera;

#ifndef IMGUI_DISABLE
			if (_hasImGui) // This is only active on the invisble context
				_closeImGui();
#endif

		};

		GLFWwindow* glfwWindow = nullptr;
		_Camera* camera = nullptr;

		void(*_closeCallback)() = nullptr; // God I hate this syntax
		bool _shouldClose = false;

		void _setCloseCallback(void(*newCallback)())
		{
			_closeCallback = newCallback;
		}

		int width;
		int height;

		bool _isFullScreen = false; // Is fullscreen
		bool _isBorderless = false; // Is borderless fullscreen

		bool vSyncEnabled = false;

		bool _hasImGui = false;

		std::string title = "Application";
	};

	// A reference to a window
	typedef WindowContext* Window;
}