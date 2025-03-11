#pragma once

#include "imgui/imgui.h"

namespace lost
{

	// Sets up ImGui for the window given, if no window was given uses the first window created 
	void setupImGui();

	// Closes ImGui for the window given, if no window was given uses the first window created 
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _closeImGui();

	// Displays a large amount of the programs info
	void imGuiDisplayProgramInfo();
}

namespace ImGui
{

	// Extra ImGui functions

	static bool renderFrameContent(const char* title, bool* value, void(*insert)() = nullptr);
	bool BeginChildEx(const char* id, float sizeX, float paddingX, float paddingY);
	void EndChildEx();
	bool BeginCollapsingHeaderEx(const char* id, const char* title, bool* isOpen = nullptr, float sizeX = 0.0f, float paddingX = 0.0f, float paddingY = 0.0f);
	bool BeginCollapsingHeaderWithInsert(const char* id, const char* title, void(*titleInsert)(), bool* isOpen = nullptr, float sizeX = 0.0f, float paddingX = 0.0f, float paddingY = 0.0f);
	void EndCollapsingHeaderEx(bool isOpen);
}