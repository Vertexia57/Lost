#include "Text.h"
#include "../ResourceManagers/GLResourceManagers.h"
#include "../Renderer.h"
#include "../Shaders/ShaderCode.h"

#include "../../DeltaTime.h"

#include "ft2build.h"
#include FT_FREETYPE_H

int min(int a, int b)
{
	return (a > b) ? b : a;
}

int max(int a, int b)
{
	return (a > b) ? a : b;
}

namespace lost
{

	Shader _textShader = nullptr;

	void _initTextRendering()
	{
		_textShader = new _Shader();
		_textShader->buildShader(_baseVSCode, _baseTextFSCode);
	}

	void _destroyTextRendering()
	{
		delete _textShader;
	}

    Font lost::_loadFontNoManager(const char* filePath, float fontSize)
    {
		Font newFont = new _Font();

		FT_Library fontLibrary;
		FT_Init_FreeType(&fontLibrary);

		FT_Face fontFace;
		FT_New_Face(fontLibrary, filePath, 0, &fontFace);
		FT_Set_Pixel_Sizes(fontFace, 0, fontSize);

		int padding = 2;
		int row = 0;
		int col = padding;

		const int textureWidth = 512;
		newFont->textureSize = textureWidth;
		char* textureBuffer = new char[textureWidth * textureWidth];

		// Set the buffer to black
		for (unsigned int y = 0; y < textureWidth; ++y)
		{
			for (unsigned int x = 0; x < textureWidth; ++x)
				textureBuffer[y * textureWidth + x] = 0;
		}

		for (FT_ULong glyphIdx = 32; glyphIdx < 127; ++glyphIdx)
		{
			FT_UInt glyphIndex = FT_Get_Char_Index(fontFace, glyphIdx);
			FT_Load_Glyph(fontFace, glyphIndex, FT_LOAD_DEFAULT);
			FT_Error error = FT_Render_Glyph(fontFace->glyph, FT_RENDER_MODE_NORMAL);

			if (col + fontFace->glyph->bitmap.width + padding >= 512)
			{
				col = padding;
				row += fontSize;
			}

			// Font Height
			newFont->fontHeight = max((fontFace->size->metrics.ascender - fontFace->size->metrics.descender) >> 6, newFont->fontHeight);

			for (unsigned int y = 0; y < fontFace->glyph->bitmap.rows; ++y)
			{
				for (unsigned int x = 0; x < fontFace->glyph->bitmap.width; ++x)
				{
					textureBuffer[(row + y) * textureWidth + col + x] =
						min(fontFace->glyph->bitmap.buffer[y * fontFace->glyph->bitmap.width + x] + 1, 255);
				}
			}

			Glyph* glyph = &(newFont->glyphs[glyphIdx]);
			glyph->textureCoords = { col, row };
			glyph->size =
			{
				(int)fontFace->glyph->bitmap.width,
				(int)fontFace->glyph->bitmap.rows
			};
			glyph->advance =
			{
				(float)(fontFace->glyph->advance.x >> 6),
				(float)(fontFace->glyph->advance.y >> 6)
			};
			glyph->offset =
			{
				(float)fontFace->glyph->bitmap_left,
				(float)fontFace->glyph->bitmap_top,
			};

			col += fontFace->glyph->bitmap.width + padding;
		}

		FT_Done_Face(fontFace);
		FT_Done_FreeType(fontLibrary);

		// Upload OpenGL Texture
		newFont->fontTexture = makeTexture(textureBuffer, textureWidth, textureWidth, filePath, LOST_FORMAT_R);
		newFont->fontMaterial = makeMaterial({ newFont->fontTexture }, filePath, _textShader);
		newFont->fontMaterial->setDepthTestFunc(LOST_DEPTH_TEST_ALWAYS);
		newFont->fontMaterial->setDepthWrite(false);

		delete[] textureBuffer;
		return newFont;
    }

	Vec2 textBounds(const char* text, Font font, float scale)
	{
		Vec2 min = { 0, 0 };
		Vec2 max = { 0, 0 };

		if (text != nullptr) // Valid string
		{
			if (*text != '\0') // Not empty
			{
				Vec2 pos = { 0, 0 };

				for (int i = 0; text[i] != '\0'; i++)
				{
					Glyph& glyph = font->glyphs[text[i]];

					Vec2 charMin = { pos.x - glyph.offset.x * scale, pos.y - glyph.offset.y * scale };
					Vec2 charMax = charMin + Vec2{ (float)glyph.size.x * scale, (float)glyph.size.y * scale };

					min.x = fminf(charMin.x, min.x);
					min.y = fminf(charMin.y, min.y);
					max.x = fmaxf(charMax.x, max.x);
					max.y = fmaxf(charMax.y, max.y);

					pos.x += glyph.advance.x * scale;
					if (text[i] == '\n')
					{
						pos.x = 0.0f;
						pos.y += font->fontHeight * scale;
					}
				}
			}
		}

		return max - min;
	}

	float textWidth(const char* text, Font font, float scale)
	{
		float min = 0;
		float max = 0;

		if (text != nullptr) // Valid string
		{
			if (*text != '\0') // Not empty
			{
				float xPos = 0;

				for (int i = 0; text[i] != '\0'; i++)
				{
					Glyph& glyph = font->glyphs[text[i]];

					float charMin = xPos - glyph.offset.x * scale;
					float charMax = charMin + (float)glyph.size.x * scale;

					min = fminf(charMin, min);
					max = fmaxf(charMax, max);

					xPos += glyph.advance.x * scale;
					if (text[i] == '\n')
					{
						xPos = 0.0f;
					}
				}
			}
		}

		return max - min;
	}

	float textHeight(const char* text, Font font, float scale)
	{
		float min = 0;
		float max = 0;

		if (text != nullptr) // Valid string
		{
			if (*text != '\0') // Not empty
			{
				float yPos = 0;

				for (int i = 0; text[i] != '\0'; i++)
				{
					Glyph& glyph = font->glyphs[text[i]];

					float charMin = yPos - glyph.offset.y * scale;
					float charMax = charMin + (float)glyph.size.y * scale;

					min = fminf(charMin, min);
					max = fmaxf(charMax, max);

					if (text[i] == '\n')
					{
						yPos += font->fontHeight * scale;
					}
				}
			}
		}

		return max - min;
	}

	void renderText(const char* text, Font font, Vec2 position, float scale, int hAlign, int vAlign, Shader shaderOverride)
	{
		if (text != nullptr) // Valid string
		{
			if (*text != '\0') // Not empty
			{
				Vec2 origin = position;

				float textW = 0;
				float textH = 0;
				if (hAlign != LOST_TEXT_ALIGN_LEFT && vAlign != LOST_TEXT_ALIGN_TOP) // Runs less code
				{
					Vec2 bounds = textBounds(text, font, scale);
					textW = bounds.x;
					textH = bounds.y;
				}
				else if (hAlign != LOST_TEXT_ALIGN_LEFT)
					textW = (float)textWidth(text, font, scale);
				else if (vAlign != LOST_TEXT_ALIGN_LEFT)
					textH = (float)textHeight(text, font, scale);
					
				switch (hAlign)
				{
				case LOST_TEXT_ALIGN_LEFT:
					break;
				case LOST_TEXT_ALIGN_MIDDLE:
					origin.x -= textW / 2.0f;
					break;
				case LOST_TEXT_ALIGN_RIGHT:
					origin.x -= textW;
					break;
				}

				switch (vAlign)
				{
				case LOST_TEXT_ALIGN_TOP:
					origin.y += (float)font->fontHeight * scale * 2.0f / 3.0f;
					break;
				case LOST_TEXT_ALIGN_MIDDLE:
					origin.y += (float)font->fontHeight * scale * 2.0f / 3.0f - textH / 2.0f;
					break;
				case LOST_TEXT_ALIGN_BOTTOM:
					origin.y += (float)font->fontHeight * scale * 2.0f / 3.0f - textH;
					break;
				}

				Vec2 pos = origin;

				for (int i = 0; text[i] != '\0'; i++)
				{
					Glyph& glyph = font->glyphs[text[i]];

					// Render textured rect of char
					lost::_renderChar(
						{ pos.x - glyph.offset.x * scale, pos.y - glyph.offset.y * scale, (float)glyph.size.x * scale, (float)glyph.size.y * scale },
						{ (float)glyph.textureCoords.x / font->textureSize, (float)glyph.textureCoords.y / font->textureSize, (float)glyph.size.x / font->textureSize, (float)glyph.size.y / font->textureSize },
						font->fontMaterial, shaderOverride
					);

					pos.x += glyph.advance.x * scale;
					if (text[i] == '\n')
					{
						pos.x = origin.x;
						pos.y += font->fontHeight * scale;
					}
				}
			}
		}
	}

	void renderTextPro3D(const char* text, Font font, Vec3 position, Vec3 rotation, Vec3 scale, int hAlign, int vAlign, Shader shaderOverride)
	{
		// [!] TODO
	}

	_Font::~_Font()
	{
		lost::unloadTexture(fontTexture);
		lost::destroyMaterial(fontMaterial);
	}

}
