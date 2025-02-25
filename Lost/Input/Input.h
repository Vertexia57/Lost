#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../GL/Vector.h"
#include "../GL/WindowContext.h"

enum LostKeyCodes
{
	// Ascii mapped keys

	LOST_KEY_SPACE = 32,
	LOST_KEY_FIRST = LOST_KEY_SPACE, // Marks the first entry in this enum
	LOST_KEY_APOSTROPHE = 39, /* ' */
	LOST_KEY_COMMA = 44, /* , */
	LOST_KEY_MINUS = 45, /* - */
	LOST_KEY_PERIOD = 46, /* . */
	LOST_KEY_SLASH = 47, /* / */
	LOST_KEY_0 = 48,
	LOST_KEY_1 = 49,
	LOST_KEY_2 = 50,
	LOST_KEY_3 = 51,
	LOST_KEY_4 = 52,
	LOST_KEY_5 = 53,
	LOST_KEY_6 = 54,
	LOST_KEY_7 = 55,
	LOST_KEY_8 = 56,
	LOST_KEY_9 = 57,
	LOST_KEY_SEMICOLON = 59, /* ; */
	LOST_KEY_EQUAL = 61, /* = */
	LOST_KEY_A = 65,
	LOST_KEY_B = 66,
	LOST_KEY_C = 67,
	LOST_KEY_D = 68,
	LOST_KEY_E = 69,
	LOST_KEY_F = 70,
	LOST_KEY_G = 71,
	LOST_KEY_H = 72,
	LOST_KEY_I = 73,
	LOST_KEY_J = 74,
	LOST_KEY_K = 75,
	LOST_KEY_L = 76,
	LOST_KEY_M = 77,
	LOST_KEY_N = 78,
	LOST_KEY_O = 79,
	LOST_KEY_P = 80,
	LOST_KEY_Q = 81,
	LOST_KEY_R = 82,
	LOST_KEY_S = 83,
	LOST_KEY_T = 84,
	LOST_KEY_U = 85,
	LOST_KEY_V = 86,
	LOST_KEY_W = 87,
	LOST_KEY_X = 88,
	LOST_KEY_Y = 89,
	LOST_KEY_Z = 90,
	LOST_KEY_LEFT_BRACKET = 91, /* [ */
	LOST_KEY_BACKSLASH = 92, /* \ */
	LOST_KEY_RIGHT_BRACKET = 93, /* ] */
	LOST_KEY_GRAVE_ACCENT = 96, /* ` */ 

	// Non ascii mapped keys

	LOST_KEY_WORLD_1 = 161, /* non-US #1 */
	LOST_KEY_WORLD_2 = 162, /* non-US #2 */
	LOST_KEY_ESCAPE = 256,
	LOST_KEY_ENTER = 257,
	LOST_KEY_TAB = 258,
	LOST_KEY_BACKSPACE = 259,
	LOST_KEY_INSERT = 260,
	LOST_KEY_DELETE = 261,
	LOST_KEY_RIGHT = 262,
	LOST_KEY_LEFT = 263,
	LOST_KEY_DOWN = 264,
	LOST_KEY_UP = 265,
	LOST_KEY_PAGE_UP = 266,
	LOST_KEY_PAGE_DOWN = 267,
	LOST_KEY_HOME = 268,
	LOST_KEY_END = 269,
	LOST_KEY_CAPS_LOCK = 280,
	LOST_KEY_SCROLL_LOCK = 281,
	LOST_KEY_NUM_LOCK = 282,
	LOST_KEY_PRINT_SCREEN = 283,
	LOST_KEY_PAUSE = 284,
	LOST_KEY_F1 = 290,
	LOST_KEY_F2 = 291,
	LOST_KEY_F3 = 292,
	LOST_KEY_F4 = 293,
	LOST_KEY_F5 = 294,
	LOST_KEY_F6 = 295,
	LOST_KEY_F7 = 296,
	LOST_KEY_F8 = 297,
	LOST_KEY_F9 = 298,
	LOST_KEY_F10 = 299,
	LOST_KEY_F11 = 300,
	LOST_KEY_F12 = 301,
	LOST_KEY_F13 = 302,
	LOST_KEY_F14 = 303,
	LOST_KEY_F15 = 304,
	LOST_KEY_F16 = 305,
	LOST_KEY_F17 = 306,
	LOST_KEY_F18 = 307,
	LOST_KEY_F19 = 308,
	LOST_KEY_F20 = 309,
	LOST_KEY_F21 = 310,
	LOST_KEY_F22 = 311,
	LOST_KEY_F23 = 312,
	LOST_KEY_F24 = 313,
	LOST_KEY_F25 = 314,
	LOST_KEY_KP_0 = 320,
	LOST_KEY_KP_1 = 321,
	LOST_KEY_KP_2 = 322,
	LOST_KEY_KP_3 = 323,
	LOST_KEY_KP_4 = 324,
	LOST_KEY_KP_5 = 325,
	LOST_KEY_KP_6 = 326,
	LOST_KEY_KP_7 = 327,
	LOST_KEY_KP_8 = 328,
	LOST_KEY_KP_9 = 329,
	LOST_KEY_KP_DECIMAL = 330,
	LOST_KEY_KP_DIVIDE = 331,
	LOST_KEY_KP_MULTIPLY = 332,
	LOST_KEY_KP_SUBTRACT = 333,
	LOST_KEY_KP_ADD = 334,
	LOST_KEY_KP_ENTER = 335,
	LOST_KEY_KP_EQUAL = 336,
	LOST_KEY_LEFT_SHIFT = 340,
	LOST_KEY_LEFT_CONTROL = 341,
	LOST_KEY_LEFT_ALT = 342,
	LOST_KEY_LEFT_SUPER = 343,
	LOST_KEY_RIGHT_SHIFT = 344,
	LOST_KEY_RIGHT_CONTROL = 345,
	LOST_KEY_RIGHT_ALT = 346,
	LOST_KEY_RIGHT_SUPER = 347,
	LOST_KEY_MENU = 348,
	LOST_KEY_LAST = LOST_KEY_MENU // Marks the last entry in this enum
};

enum LostMouseButtons
{
	LOST_MOUSE_LEFT = GLFW_MOUSE_BUTTON_1,
	LOST_MOUSE_BUTTON_1 = GLFW_MOUSE_BUTTON_1,
	LOST_MOUSE_RIGHT = GLFW_MOUSE_BUTTON_2,
	LOST_MOUSE_BUTTON_2 = GLFW_MOUSE_BUTTON_2,
	LOST_MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_3,
	LOST_MOUSE_BUTTON_3 = GLFW_MOUSE_BUTTON_3,
	LOST_MOUSE_BUTTON_4 = GLFW_MOUSE_BUTTON_4,
	LOST_MOUSE_BUTTON_5 = GLFW_MOUSE_BUTTON_5,
	LOST_MOUSE_BUTTON_6 = GLFW_MOUSE_BUTTON_6,
	LOST_MOUSE_BUTTON_7 = GLFW_MOUSE_BUTTON_7,
	LOST_MOUSE_BUTTON_8 = GLFW_MOUSE_BUTTON_8
};

namespace lost
{

	static unsigned int _keyCodeLength = LOST_KEY_LAST - LOST_KEY_FIRST + 1;
	static unsigned int _mouseButtonLength = LOST_MOUSE_BUTTON_8 - LOST_MOUSE_BUTTON_1 + 1;

	// The callback used by Lost ran by GLFW
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _windowKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	// The callback used by Lost ran by GLFW
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _mouseMoveCallback(GLFWwindow* window, double xPos, double yPos);

	// The callback used by Lost ran by GLFW
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	// The callback used by Lost ran by GLFW
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

	// Runs glfwPollInputs() and updates taps
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _pollInputs();

	// Returns if a specific key is down, updates once per frame
	bool getKeyDown(int keyCode);
	// Returns if a specific key is down, updates once per frame
	bool getKeyDown(char key);

	// Returns true when a specific key starts being pressed down, updates once per frame
	bool getKeyTapped(int keyCode);
	// Returns true when a specific key starts being pressed down (Works with unicode characters <126 / <FF, updates once per frame
	bool getKeyTapped(char key);

	// Returns true when a specific key has been released, updates once per frame
	bool getKeyReleased(int keyCode);
	// Returns true when a specific key has been released, updates once per frame
	bool getKeyReleased(char key);

	// Returns the current position of the mouse relative to the focussed window
	Vec2 getMousePosition();
	// Returns the current x position of the mouse relative to the focussed window
	float getMouseX();
	// Returns the current y position of the mouse relative to the focussed window
	float getMouseY();

	// Sets the current position of the mouse relative to the window given, by default using the first window created
	void setMousePosition(float x, float y, Window window = nullptr);

	bool getMouseDown(int mouseButton);
	bool getMouseTapped(int mouseButton);
	bool getMouseReleased(int mouseButton);

	int getMouseScroll(bool horizontal = false);
}