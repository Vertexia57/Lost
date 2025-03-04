#include "Input.h"

#include "../Log.h"
#include "../GL/LostGL.h"

#include <array>

namespace lost
{

	class InputManager
	{
	public:
		InputManager()
		{
			// Initialise the arrays which store the keys pressed

			// Keyboard
			keysPressed.resize(_keyCodeLength);
			keysTapped.resize(_keyCodeLength);
			keysReleased.resize(_keyCodeLength);

			for (int i = 0; i < _keyCodeLength; i++)
			{
				keysPressed[i] = false;
				keysTapped[i] = false;
				keysReleased[i] = false;
			}

			// Mouse
			mousePressed.resize(_mouseButtonLength);
			mouseTapped.resize(_mouseButtonLength);
			mouseReleased.resize(_mouseButtonLength);

			for (int i = 0; i < _mouseButtonLength; i++)
			{
				mousePressed[i] = false;
				mouseTapped[i] = false;
				mouseReleased[i] = false;
			}
		}

		void _pollInputs()
		{
			for (int i = 0; i < _keyCodeLength; i++)
			{
				keysTapped[i] = false;
				keysReleased[i] = false;
			}

			for (int i = 0; i < _mouseButtonLength; i++)
			{
				mouseTapped[i] = false;
				mouseReleased[i] = false;
			}

			mouseScrollX = 0.0f;
			mouseScrollY = 0.0f;

			glfwPollEvents();
		}

		// Keyboard
		std::vector<bool> keysPressed;
		std::vector<bool> keysTapped;
		std::vector<bool> keysReleased;

		// Mouse
		std::vector<bool> mousePressed;
		std::vector<bool> mouseTapped;
		std::vector<bool> mouseReleased;

		Vec2 mousePosition = { 0.0f, 0.0f };

		double mouseScrollX = 0.0f;
		double mouseScrollY = 0.0f;
	};

	InputManager inputManager;

	void _windowKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key > LOST_KEY_LAST || key < LOST_KEY_FIRST)
		{
			debugLogIf(key != -1, std::string("Unknown key input with code: ") + std::to_string(key), LOST_LOG_WARNING_NO_NOTE);
			return;
		}

		if (action == GLFW_PRESS)
		{
			inputManager.keysTapped[key - LOST_KEY_FIRST] = true;
			inputManager.keysPressed[key - LOST_KEY_FIRST] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			inputManager.keysReleased[key - LOST_KEY_FIRST] = true;
			inputManager.keysPressed[key - LOST_KEY_FIRST] = false;
		}
	}

	void _mouseMoveCallback(GLFWwindow* window, double xPos, double yPos)
	{
		inputManager.mousePosition = { (float)xPos, (float)yPos };
	}

	void _mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			inputManager.mouseTapped[button] = true;
			inputManager.mousePressed[button] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			inputManager.mouseReleased[button] = true;
			inputManager.mousePressed[button] = false;
		}
	}

	void _mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		inputManager.mouseScrollX += xOffset;
		inputManager.mouseScrollY += yOffset;
	}

	void _pollInputs()
	{
		inputManager._pollInputs();
	}

	bool getKeyDown(int keyCode)
	{
		return inputManager.keysPressed[keyCode - LOST_KEY_FIRST];
	}

	bool getKeyDown(char key)
	{
		if (key < 128)
			return inputManager.keysPressed[key - LOST_KEY_FIRST];
		debugLog("Tried to test for a non-unicode key in getKeyDown()", LOST_LOG_WARNING);
		return false;
	}

	bool getKeyTapped(int keyCode)
	{
		return inputManager.keysTapped[keyCode - LOST_KEY_FIRST];
	}

	bool getKeyTapped(char key)
	{
		if (key < 128)
			return inputManager.keysTapped[key - LOST_KEY_FIRST];
		debugLog("Tried to test for a non-unicode key in getKeyTapped()", LOST_LOG_WARNING);
		return false;
	}

	bool getKeyReleased(int keyCode)
	{
		return inputManager.keysReleased[keyCode - LOST_KEY_FIRST];
	}

	bool getKeyReleased(char key)
	{
		if (key < 128)
			return inputManager.keysPressed[key - LOST_KEY_FIRST];
		debugLog("Tried to test for a non-unicode key in getKeyReleased()", LOST_LOG_WARNING);
		return false;
	}

	Vec2 getMousePosition()
	{
		return inputManager.mousePosition;
	}

	float getMouseX()
	{
		return inputManager.mousePosition.x;
	}

	float getMouseY()
	{
		return inputManager.mousePosition.y;
	}

	void setMousePosition(float x, float y, Window window)
	{
		if (window == nullptr)
			window = getWindow(0);
		glfwSetCursorPos(window->glfwWindow, x, y);
		inputManager.mousePosition.x = x;
		inputManager.mousePosition.y = y;
	}

	bool getMouseDown(int mouseButton)
	{
		return inputManager.mousePressed[mouseButton];
	}

	bool getMouseTapped(int mouseButton)
	{
		return inputManager.mouseTapped[mouseButton];
	}

	bool getMouseReleased(int mouseButton)
	{
		return inputManager.mouseReleased[mouseButton];
	}

	int getMouseScroll(bool horizontal)
	{
		if (horizontal)
			return inputManager.mouseScrollX;
		return inputManager.mouseScrollY;
	}

}