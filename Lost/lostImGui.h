#pragma once

#include "imgui/imgui.h"

namespace lost
{

	// Sets up ImGui for the window given, if no window was given uses the first window created 
	void setupImGui(float scale = 1.0f);

	// Closes ImGui for the window given, if no window was given uses the first window created 
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _closeImGui();

	// Displays a large amount of the programs info.
	// Everything within this function is only ran by this function.
	// None of the debug features within this function are ran by the engine.
	// All of the overhead of this function comes from running this.
	// The only cost that this incures is a slight memory cost, which can be removed my disabling imgui.
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

	//// Graph

	//enum GraphType
	//{
	//	ImGuiGraphType_BarX,
	//	ImGuiGraphType_BarY
	//};

	//struct Graph
	//{
	//	unsigned int graphType;
	//	float minX;
	//	float maxX;
	//	float minY;
	//	float maxY;
	//};
	//
}