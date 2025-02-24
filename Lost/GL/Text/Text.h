#pragma once
#include <ft2build.h>
#include "../Texture/Texture.h"
#include "../Texture/Material.h"
#include <string>
#include <map>
#include <vector>
#include "../Vector.h"

enum {
	LOST_TEXT_ALIGN_LEFT = 0,
	LOST_TEXT_ALIGN_TOP = 0,
	LOST_TEXT_ALIGN_MIDDLE = 1,
	LOST_TEXT_ALIGN_RIGHT = 2,
	LOST_TEXT_ALIGN_BOTTOM = 2
};

namespace lost
{
	struct Glyph
	{
		Vec2 offset;
		Vec2 advance;
		IVec2 textureCoords;
		IVec2 size;
	};

	struct _Font
	{
		~_Font();

		int textureSize;
		int fontHeight;
		Glyph glyphs[127];
		Texture fontTexture;
		Material fontMaterial;
	};

	// A reference to a font
	typedef _Font* Font;

	void _initTextRendering();
	void _destroyTextRendering();

	Font _loadFontNoManager(const char* filePath, float fontSize);

	// Returns the width and height of what the text given would take up when rendered
	Vec2 textBounds(const char* text, Font font, float scale);
	// Returns the width of what the text given would take up when rendered
	float textWidth(const char* text, Font font, float scale);
	// Returns the height of what the text given would take up when rendered
	float textHeight(const char* text, Font font, float scale);

	// Renders text to the screen, using screenspace, allows for alignment
	void renderText(const char* text, Font font, Vec2 position, float scale, int hAlign = 0, int vAlign = 0);
	// Renders text to the scene, using 3D space, allows for alignment
	void renderTextPro3D(const char* text, Font font, Vec3 position, Vec3 rotation, Vec3 scale, int hAlign = 0, int vAlign = 0);

}