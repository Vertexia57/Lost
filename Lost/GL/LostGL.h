#pragma once
#include <glad/glad.h>
#include <vector>
#include "Structs.h"
#include "Shaders/Shader.h"
#include "WindowContext.h"
#include "Renderer.h"
#include "ResourceManagers/GLResourceManagers.h"
#include "Camera.h"

namespace lost
{
	extern Shader _defaultShader;

	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _initGL(unsigned int rendererMode); // [!] TODO: Docs
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _initOpenGL();
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _exitGL();
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _updateGL();
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _windowResizeCallback(GLFWwindow* window, int width, int height);

	const Material getDefaultWhiteMaterial();
	const Texture getDefaultWhiteTexture();
	const Texture getDefaultBlackTexture();
	const Texture getDefaultNormalTexture();

	void beginFrame(Window context = nullptr);
	void endFrame();

	Window createWindow(int width, int height, const char* title = "Application");
	Window getWindow(unsigned int id = 0);
	Window getCurrentWindow();
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _setWindow(unsigned int id);
	unsigned int getCurrentWindowID();
	const std::vector<Window>& getWindows();

	// Pushes the current window context to a stack which can be popped off of with popWindow()
	void pushWindow();
	// Pops the top of the window context stack and sets the current stack to the context popped off
	void popWindow();

	// Returns the invisible context, this is the context which stores all VBOs, EBOs, Shaders, Programs and other large data used in OpenGL
	Window getInvisibleContext();

	// Returns if any window is currently still open
	bool windowOpen();

	// Sets the window given to be closed
	// Does not do it immediately, only on the next lost::endFrame() or lost::_updateGL() call
	void closeWindow(Window context = nullptr);

	// Sets the title of the window, changes the first window created if no context is set
	void setWindowTitle(const char* title, Window context = nullptr);

	// Closes all active windows
	void closeAllWindows();

	// Sets the close callback of the window given
	void setWindowCloseCallback(Window context, void(*callback)());

	// Sets the position of the window given, by default runs on the first window created
	void setWindowPosition(int x, int y, Window context = nullptr);

	// Sets the size of the window given, by default runs on the first window created
	void setWindowSize(int w, int h, Window context = nullptr);

	// Sets the attribute given with the enum WindowAttrib on the window given, by default runs on the first window created
	void setWindowAttrib(unsigned int attribute, bool state, Window context = nullptr);

	// Sets the window given to be fullscreen
	// "isWindowed" defines if it should "fake" being fullscreen as a window that takes the entire screen and has no decoration or if it should actually be fullscreen
	// "whichMonitor" defines which monitor it should go fullscreen on, by default this is -1, which is the primary monitor
	void makeWindowFullscreen(bool isWindowed = true, Window context = nullptr, int whichMonitor = -1);

	// Makes a window that previously was fullscreened, windowed
	void makeWindowWindowed(int w, int h, Window context = nullptr);

	// Set shader to default shader
	void unbindShader();

	// Enables or disables VSync, limiting the FPS to save on CPU / GPU usage
	void useVSync(bool state, Window context = nullptr);

	// Returns the windows width
	int getWidth(Window context = nullptr);
	// Returns the windows height
	int getHeight(Window context = nullptr);

	// Returns the main camera that is used in rendering
	_Camera* _getCurrentCamera(Window context = nullptr);
}