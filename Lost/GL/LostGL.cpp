#include "LostGL.h"
#include "../Log.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <stack>
#include <iostream>
#include "../DeltaTime.h"
#include "Text/Text.h"
#include "../Input/Input.h"

namespace lost
{
	unsigned int _currentContextID = -1;

	// This is used as a share point for all assets
	// It helps with setting up data as you can load data at anypoint after _initGL()
	// This context is not visible at all, and cannot render to the screen, but it isn't tested like a normal window
	// on WindowOpen()
	Window _invisibleContext = nullptr;

	std::vector<Window> _windowContexts = {};
	bool _glfwInitialized = false;
	Shader _defaultShader = nullptr;

	std::stack<unsigned int> _windowContextStack;

	// .cpp only function, runs the default hints for the currently enabled GLFW context
	// this works with CONTEXT so it runs on the currently active GLFW context
	void setGLFWWindowHints()
	{
		// Initialize glfw context info
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE); // To enable this one over the invisible context
#ifdef LOST_DEBUG_MODE // Debug
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
	}

	// Initialize LostGL
	void _initGL(unsigned int rendererMode)
	{
		setStateData(LOST_STATE_RENDERER_MODE, (void*)rendererMode);

		if (!glfwInit())
			log("GLFW failed to initialize!", LOST_LOG_FATAL); // Assert
		_glfwInitialized = true;

		_initOpenGL();
		_initRMs();
		_initRenderer(rendererMode);
		_initTextRendering();

		setStateData(LOST_STATE_GL_INITIALIZED, (void*)true);

		debugLog("Initialized LostGL", LOST_LOG_SUCCESS);

#ifndef NDEBUG & LOST_DEBUG_MODE
		debugLog("Compiled in debug mode! Optimisation might not be enabled, Rendering large amounts of objects may be very slow!", LOST_LOG_WARNING_NO_NOTE);
#endif
	}

	// Initialize the OpenGL context, creating an invisible window which holds all the main data
	void _initOpenGL()
	{
		setGLFWWindowHints(); // Sets the default hints for a normal window
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Then makes the window invisible

		GLFWwindow* invisibleGLFWContext = glfwCreateWindow(1000, 1000, "a", NULL, NULL); // Creates the window with the hints given
		glfwMakeContextCurrent(invisibleGLFWContext);

		_invisibleContext = new WindowContext(invisibleGLFWContext);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // Load OpenGL
		{
			log("Failed to load OpenGL!", LOST_LOG_FATAL); // Assert
			glfwTerminate();
		}

		// If in 2D rendering mode transparency in the default shader
		_defaultShader = new _Shader(nullptr, nullptr);
	}

	// Terminate GLFW and remove all window contexts from heap
	void _exitGL()
	{
		_destroyTextRendering();

		_destroyRMs();
		_destroyRenderer();

		glfwTerminate();
		for (Window context : _windowContexts)
			delete context;
		delete _invisibleContext;
		delete _defaultShader;
	}

	void _updateGL()
	{
		_pollInputs();
	}

	void _windowResizeCallback(GLFWwindow* window, int width, int height)
	{
		int id = -1;
		Window windowContext = nullptr;

		for (int i = 0; i < _windowContexts.size(); i++)
		{
			if (_windowContexts[i]->glfwWindow == window)
			{
				id = i;
				windowContext = _windowContexts[i];
				break;
			}
		}

		if (windowContext)
		{
			windowContext->width = width;
			windowContext->height = height;
			windowContext->camera->_updatePerspective(width, height);
		}
		lost::_resizeFrameBuffers(id, width, height);
	}

	Window createWindow(int width, int height, const char* title)
	{
		// Check if glfw was initialized
		if (_glfwInitialized)
		{
			
			debugLogIf(getLostState().usingNonWindowedFullscreen, "Created a window after making a screen fullscreen\nThis minimizes the fullscreen window as the new window gains focus\nConsider making the screen before fullscreening any", LOST_LOG_WARNING);

			// Create window context
			setGLFWWindowHints(); // Sets the window hints for the to be created window
			GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, _invisibleContext->glfwWindow);
			
			glfwMakeContextCurrent(window);

			glfwSetWindowSizeCallback(window, _windowResizeCallback);
			glfwSetKeyCallback(window, _windowKeyCallback);
			glfwSetCursorPosCallback(window, _mouseMoveCallback);
			glfwSetMouseButtonCallback(window, _mouseButtonCallback); 
			glfwSetScrollCallback(window, _mouseScrollCallback);

			// Initialize this window contexts OpenGL context settings
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			// Enable backface culling
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

#ifdef LOST_DEBUG_MODE // Debug
			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
				debugLog(std::string("OpenGL Debug Message: ") + message, LOST_LOG_WARNING);
				}, NULL);
#endif

			debugLog(std::string("Created Window with bounds: (") + std::to_string(width) + ", " + std::to_string(height) + ")", LOST_LOG_SUCCESS);

			// Add window context to list and return the id for the window
			Window windowContext = new WindowContext(window);
			windowContext->camera->setPerspective(45.0f, (float)width / (float)height, 0.01f, 100.0f);
			windowContext->title = title;

			_windowContexts.push_back(windowContext);

			// Generate a VAO needed for the renderer
			_generateNewVAO();

			glfwMakeContextCurrent(_invisibleContext->glfwWindow);
			return windowContext;
		}

		log("Window could not be created!\nForgot to put lost::init() before lost::createWindow()", LOST_LOG_FATAL); // Assert
		return nullptr;
	}

	Window getWindow(unsigned int id)
	{
#ifdef LOST_DEBUG_MODE
		if (id >= _windowContexts.size())
		{
			debugLog("Tried to get a window that didn't exist\n(id > _windowContexts.size)\n\"id\" was: " + std::to_string(id), LOST_LOG_WARNING);
			return nullptr;
		}
#endif
		return _windowContexts[id];
	}

	Window getCurrentWindow()
	{
		if (_currentContextID != -1)
			return _windowContexts[_currentContextID];
		return nullptr;
	}

	void _setWindow(unsigned int id)
	{
		_currentContextID = id;
		glfwMakeContextCurrent(_windowContexts[id]->glfwWindow);
	}

	unsigned int getCurrentWindowID()
	{
		return _currentContextID;
	}

	const std::vector<Window>& getWindows()
	{
		return _windowContexts;
	}

	Window getInvisibleContext()
	{
		return _invisibleContext;
	}

	void pushWindow()
	{
		_windowContextStack.push(_currentContextID);
	}

	void popWindow()
	{
		unsigned int popVal = _windowContextStack.top();
		_windowContextStack.pop();

		_currentContextID = popVal;
		if (popVal != -1)
			glfwMakeContextCurrent(_windowContexts[popVal]->glfwWindow);
		else
			glfwMakeContextCurrent(_invisibleContext->glfwWindow);
	}

	bool windowOpen()
	{
		lost::recalcDeltaTime();

		// Main loop, run closeCallbacks and update shouldClose list
		for (int i = 0; i < _windowContexts.size(); i++) 
		{
			Window context = _windowContexts[i];

			// Check if window should close
			if (glfwWindowShouldClose(context->glfwWindow) || context->_shouldClose)
			{
				if (context->_closeCallback != nullptr) // Check if closeCallback was set and run it
					context->_closeCallback();
				else
					context->_shouldClose = true;
			}
		}

		// Destruction loop, loop over all windowContexts and remove any that are marked for closing
		for (int i = _windowContexts.size() - 1; i >= 0; i--) // Loop backwards
		{
			Window context = _windowContexts[i];

			// Check if window should close
			if (context->_shouldClose)
			{
				// Destroy any window VAO which was bound to this context
				_destroyWindowVAO(i);

				glfwDestroyWindow(context->glfwWindow);

				// Remove window context from _windowContexts
				delete context;
				_windowContexts.erase(_windowContexts.begin() + i);
			}

			// No code can go here as the current context may have been deleted
		}

		return _windowContexts.size() > 0;
	}

	void closeWindow(Window context)
	{
		if (context == nullptr)
		{
#ifdef LOST_DEBUG_MODE
			if (!_windowContexts.empty())
				_windowContexts[0]->_shouldClose = true;
			else
				debugLog("Tried to close window before first window was created.", LOST_LOG_ERROR);
#else
			_windowContexts[0]->_shouldClose = true;
#endif
		}
		else
		{
			context->_shouldClose = true;
		}
	}

	void setWindowTitle(const char* title, Window context)
	{
		if (context == nullptr)
		{
			context = _windowContexts[0];
#ifdef LOST_DEBUG_MODE
			if (!_windowContexts.empty())
			{
				glfwSetWindowTitle(context->glfwWindow, title);
				context->title = title;
			}
			else
				debugLog("Tried to set title of window before first window was created.", LOST_LOG_ERROR);
#else
			glfwSetWindowTitle(context->glfwWindow, title);
			context->title = title;
#endif
		}
		else
		{
			glfwSetWindowTitle(context->glfwWindow, title);
			context->title = title;
		}
	}

	void closeAllWindows()
	{
		for (Window context : _windowContexts)
			lost::closeWindow(context);
	}

	void setWindowCloseCallback(Window context, void(*callback)())
	{
		context->_setCloseCallback(callback);
	}

	void setWindowPosition(int x, int y, Window context)
	{
		if (context != nullptr)
		{
			glfwSetWindowPos(context->glfwWindow, x, y);
		}
		else
		{
#ifdef LOST_DEBUG_MODE
			if (!_windowContexts.empty())
				glfwSetWindowPos(_windowContexts[0]->glfwWindow, x, y);
			else
				debugLog("Tried to set position of window before first window was initialized.", LOST_LOG_ERROR);
#else
			glfwSetWindowPos(_windowContexts[0]->glfwWindow, x, y);
#endif
		}
	}

	void setWindowSize(int w, int h, Window context)
	{
		if (context != nullptr)
		{
			glfwSetWindowSize(context->glfwWindow, w, h);
		}
		else
		{
#ifdef LOST_DEBUG_MODE
			if (!_windowContexts.empty())
				glfwSetWindowSize(_windowContexts[0]->glfwWindow, w, h);
			else
				debugLog("Tried to set size of window before first window was initialized.", LOST_LOG_ERROR);
#else
			glfwSetWindowSize(_windowContexts[0]->glfwWindow, w, h);
#endif
		}
	}

	void setWindowAttrib(unsigned int attribute, bool state, Window context)
	{
		if (context != nullptr)
		{
			glfwSetWindowAttrib(context->glfwWindow, attribute, state);
		}
		else
		{
#ifdef LOST_DEBUG_MODE
			if (!_windowContexts.empty())
				glfwSetWindowAttrib(_windowContexts[0]->glfwWindow, attribute, state);
			else
				debugLog("Tried to set size of window before first window was initialized.", LOST_LOG_ERROR);
#else
			glfwSetWindowAttrib(_windowContexts[0]->glfwWindow, attribute, state);
#endif
		}
	}

	void makeWindowFullscreen(bool isWindowed, Window context, int whichMonitor)
	{
#ifdef LOST_DEBUG_MODE
		if (!_windowContexts.empty())
		{
			if (context == nullptr)
				context = _windowContexts[0];
		}
		else
		{
			debugLog("Tried to make a window fullscreen before first window was initialized.", LOST_LOG_ERROR);
			return;
		}
#else
		context = _windowContexts[0];
#endif

		// Get monitor to go fullscreen on
		GLFWmonitor* monitor;
		if (whichMonitor == -1) // Use primary
			monitor = glfwGetPrimaryMonitor();
		else // Use select
		{
			// Get monitors
			int count;
			GLFWmonitor** monitors = glfwGetMonitors(&count);

			if (whichMonitor < count) // Check if monitor exists
				monitor = monitors[whichMonitor];
			else // It doesn't
			{
				debugLog("Tried to set window to monitor that doesn't exist, reverting to primary", LOST_LOG_WARNING);
				monitor = glfwGetPrimaryMonitor();
			}
		}

		// Monitor resolution
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		int resX = mode->width;
		int resY = mode->height;
		int monitorOffsetX, monitorOffsetY;
		glfwGetMonitorPos(monitor, &monitorOffsetX, &monitorOffsetY);

		if (isWindowed) // Windowed fullscreen / Fake fullscreen
		{
			if (!getLostState().usingNonWindowedFullscreen)
				setStateData(LOST_STATE_WINDOW_FULLSCREEN, (void*)true);
			else
			{
				log("Tried to make more than one window fullscreen in non-windowed mode.\nGLFW does not support this.", LOST_LOG_ERROR);
				return;
			}

			glfwSetWindowMonitor(context->glfwWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

			context->_isFullScreen = true;
			context->_isBorderless = true;

			debugLog("Made window fullscreen, using Borderless Fullscreen", LOST_LOG_INFO);
		}
		else // Non-windowed fullscreen / Real fullscreen
		{
			if (!getLostState().usingNonWindowedFullscreen)
				setStateData(LOST_STATE_WINDOW_FULLSCREEN, (void*)true);
			else
			{
				log("Tried to make more than one window fullscreen in non-windowed mode.\nGLFW does not support this.", LOST_LOG_ERROR);
				return;
			}

			glfwSetWindowMonitor(context->glfwWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

			context->_isFullScreen = true;
			context->_isBorderless = false;

			debugLog("Made window fullscreen, using Normal Fullscreen (Not advised)", LOST_LOG_INFO);
			debugLog("This type of fullscreen isn't properly implemented yet, not allowing for change in resolution or refresh rate", LOST_LOG_INFO);
		}
	}

	void makeWindowWindowed(int w, int h, Window context)
	{
		if (context == nullptr)
		{
#ifdef LOST_DEBUG_MODE
			if (_windowContexts.empty())
				debugLog("Tried to make window windowed before first window created", LOST_LOG_ERROR);
			else
				context = _windowContexts[0];
#else
			context = _windowContexts[0];
#endif
		};

		if (context->_isFullScreen)
		{
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			int resX = mode->width;
			int resY = mode->height;
			int monitorOffsetX, monitorOffsetY;
			glfwGetMonitorPos(monitor, &monitorOffsetX, &monitorOffsetY);

			glfwSetWindowMonitor(context->glfwWindow, NULL, monitorOffsetX + resX / 2 - w / 2, monitorOffsetY + resY / 2 - h / 2, w, h, mode->refreshRate);

			setStateData(LOST_STATE_WINDOW_FULLSCREEN, (void*)false);
			context->_isFullScreen = false;
		}
		else
		{
			debugLog("Attempted to make already windowed window windowed, resizing instead", LOST_LOG_INFO);
			setWindowSize(w, h, context);
		}
	}

	void unbindShader()
	{
		_defaultShader->bind();
	}

	void useVSync(bool state, Window context)
	{
		pushWindow();
		if (context != nullptr)
		{
			glfwMakeContextCurrent(context->glfwWindow);
		}
		else
		{
#ifdef LOST_DEBUG_MODE
			if (_windowContexts.empty())
				debugLog("Tried to enable VSync before first window created", LOST_LOG_ERROR);
			else
			{
				context = _windowContexts[0];
				glfwMakeContextCurrent(context->glfwWindow);
			}
#else
			context = _windowContexts[0];
			glfwMakeContextCurrent(context->glfwWindow);
#endif
		}

		glfwSwapInterval(state);
		context->vSyncEnabled = state;
		popWindow();
	}

	int getWidth(Window context)
	{
		if (context != nullptr)
			return context->width;

#ifdef LOST_DEBUG_MODE
		if (_windowContexts.empty())
			debugLog("Tried to get window height before first window created", LOST_LOG_ERROR);
		else
			return _windowContexts[0]->width;
#else
		return _windowContexts[0]->width;
#endif
	}

	int getHeight(Window context)
	{
		if (context != nullptr)
			return context->height;

#ifdef LOST_DEBUG_MODE
		if (_windowContexts.empty())
			debugLog("Tried to get window height before first window created", LOST_LOG_ERROR);
		else
			return _windowContexts[0]->height;
#else
		return _windowContexts[0]->height;
#endif
	}

	_Camera* _getCurrentCamera(Window context)
	{
		if (context == nullptr)
			return getCurrentWindow()->camera;
		else
			return context->camera;
	}

	void beginFrame(Window context)
	{
		if (context != nullptr) // A specific window to render to was given, set the context to that one
		{
			for (int i = 0; i < _windowContexts.size(); i++)
			{
				if (_windowContexts[i] == context)
				{
					_currentContextID = i;
					break;
				}

#ifdef LOST_DEBUG_MODE
				// Test if window exists
				debugLogIf(i == _windowContexts.size() - 1, "Tried to start frame on window that has been destroyed", LOST_LOG_FATAL);
#endif
			}
			glfwMakeContextCurrent(context->glfwWindow);
		}
		else // No context was given, use the first window created
		{
			context = _windowContexts[0];
			glfwMakeContextCurrent(context->glfwWindow);
			_currentContextID = 0;
		}

		glViewport(0, 0, context->width, context->height);
		lost::_updateGL();
		lost::_startRender();

		_defaultShader->bind();
	}

	void endFrame()
	{
		renderInstanceQueue();

		lost::_finalizeRender();

		glfwSwapBuffers(glfwGetCurrentContext());
	}
}