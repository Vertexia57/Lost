#include "Texture.h"

#include <glad/glad.h>
#include <iostream>

#include "Material.h"
#include "../LostGL.h"
#include "../Renderer.h"
#include "../../Log.h"

#include "../STB/STBImplementation.h"

namespace lost
{

	_Texture::_Texture()
	{
	}

	_Texture::~_Texture()
	{
		if (m_HandleDeletion)
			glDeleteTextures(1, &m_Texture);
		delete m_TextureMaterial;
	}

	void _Texture::loadTexture(const char* dir)
	{
		m_Directory = dir;

		int channels;
		unsigned char* data = stbi_load(dir, &m_Width, &m_Height, &channels, STBI_rgb_alpha);

		if (data == 0)
		{
			debugLog(std::string("Failed to load image at \"") + dir + "\", file may be broken, open by another program, missing or inaccessible", LOST_LOG_WARNING);

			bool overridingTexture = m_Texture != -1;
			
			m_Texture = getDefaultWhiteTexture()->getTexture();
			if (!overridingTexture)
				m_TextureMaterial = new _Material(lost::_defaultShader, { this });
			else
				*m_TextureMaterial = _Material(lost::_defaultShader, { this });

			return;
		}

		// Check if texture has already been loaded
		bool overridingTexture = false;
		if (m_Texture != -1)
		{
			overridingTexture = true;
			glDeleteTextures(1, &m_Texture);
		}

		//make the texture
		glGenTextures(1, &m_Texture);

		glBindTexture(GL_TEXTURE_2D, m_Texture);

		//load data
		glTexImage2D(GL_TEXTURE_2D,
			0, GL_RGBA, m_Width, m_Height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, data); 

		//free data
		stbi_image_free(data);

		//Configure sampler
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (!overridingTexture)
			m_TextureMaterial = new _Material(lost::_defaultShader, { this });
		else
			*m_TextureMaterial = _Material(lost::_defaultShader, { this });

		debugLog(std::string("Successfully loaded image at \"") + dir + "\"", LOST_LOG_SUCCESS);

		m_HandleDeletion = true;
	}

	void _Texture::makeTexture(const char* data, int width, int height, unsigned int format)
	{
		m_Width = width;
		m_Height = height;

		// Check if texture has already been loaded
		if (m_Texture != -1)
		{
			glDeleteTextures(1, &m_Texture);
			delete m_TextureMaterial;
		}

		//make the texture
		glGenTextures(1, &m_Texture);

		glBindTexture(GL_TEXTURE_2D, m_Texture);

		//load data
		glTexImage2D(GL_TEXTURE_2D,
			0, LOST_FORMAT_RGBA, width, height, 0,
			format, GL_UNSIGNED_BYTE, data);

		//Configure sampler
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		m_TextureMaterial = new _Material(lost::_defaultShader, { this });

		m_HandleDeletion = true;
	}

	void _Texture::makeTexture(unsigned int openGLTexture, bool handleDelete)
	{
		m_HandleDeletion = handleDelete;

		if (m_Texture != -1)
		{
			if (handleDelete)
				glDeleteTextures(1, &m_Texture);
			delete m_TextureMaterial;
		}

		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_Width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_Height);

		m_Texture = openGLTexture;
		m_TextureMaterial = new _Material(lost::_defaultShader, { this });
	}

	void _Texture::bind(int slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot); 
		glBindTexture(GL_TEXTURE_2D, m_Texture);
	}

}