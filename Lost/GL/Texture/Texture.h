#pragma once
#include "../STB/stb_image.h"
#include <string>

namespace lost
{

	class _Material;

	class _Texture
	{
	public:
		_Texture();
		~_Texture();

		void loadTexture(const char* dir);
		void makeTexture(const char* data, int width, int height, unsigned int format);
		void makeTexture(unsigned int openGLTexture, bool handleDelete = false);

		// Binds the texture to the shader being used, by default using slot 0 "texture0"
		void bind(int slot = 0) const;

		inline int getWidth() const { return m_Width; };
		inline int getHeight() const { return m_Height; };

		inline unsigned int getTexture() const { return m_Texture; };
		inline _Material* getMaterial() const { return m_TextureMaterial; };

		inline const char* getDirectory() const { return m_Directory.c_str(); };
	private:
		int m_Width;
		int m_Height;

		unsigned int m_Texture = -1;
		_Material* m_TextureMaterial = nullptr;

		std::string m_Directory = "No directory";

		bool m_HandleDeletion = true;
	};

	// A reference to a texture
	typedef _Texture* Texture;

}