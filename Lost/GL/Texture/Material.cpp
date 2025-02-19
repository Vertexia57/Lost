#include "Material.h"
#include <glad/glad.h>
#include <iostream>

namespace lost
{

	_Material::_Material(Shader shader, std::vector<Texture> textures, unsigned int renderQueue)
	{
		m_Shader = shader;
		m_Textures.resize(m_Shader->getTextureNameMap().size());

		if (!textures.empty())
		{
			m_Textures.erase(m_Textures.begin(), m_Textures.begin() + textures.size());
			m_Textures.insert(m_Textures.begin(), textures.begin(), textures.end());
		}
	}

	_Material::~_Material()
	{
	}

	void _Material::setTexture(const char* slotName, Texture texture)
	{
		m_Textures[m_Shader->getTextureNameMap().at(slotName)] = texture;
	}

	void _Material::bindTextures() const
	{
		for (int i = 0; i < m_Textures.size(); i++)
		{
#ifdef LOST_DEBUG_MODE
			if (m_Textures[i] == nullptr)
				debugLog("Missing textures for material, needed " + std::to_string(m_Textures.size()), LOST_LOG_WARNING);
			else
				m_Textures[i]->bind(i);
#else
			m_Textures[i]->bind(i);
#endif
		}
	}

	void _Material::bindShader() const
	{
		m_Shader->bind();
	}


}