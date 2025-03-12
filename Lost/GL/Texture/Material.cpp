#include "Material.h"
#include <glad/glad.h>
#include <iostream>

#include "../LostGL.h"

namespace lost
{
	static size_t getBytesForType(unsigned int dataType)
	{
		switch (dataType)
		{
		case LOST_TYPE_FLOAT:
		case LOST_TYPE_INT:
			return 1 * sizeof(float);
		case LOST_TYPE_VEC2:
		case LOST_TYPE_IVEC2:
			return 2 * sizeof(float);
		case LOST_TYPE_VEC3:
		case LOST_TYPE_IVEC3:
			return 3 * sizeof(float);
		case LOST_TYPE_VEC4:
		case LOST_TYPE_IVEC4:
			return 4 * sizeof(float);
		default:
			return 0;
		}
	}

	_Material::_Material(Shader shader, std::vector<Texture> textures, unsigned int renderQueue)
	{
		m_Shader = shader;
		m_Textures.clear();
		m_Textures.reserve(m_Shader->getTextureNameMap().size());
		m_QueueLevel = renderQueue;

		if (!textures.empty())
		{
			m_Textures.insert(m_Textures.begin(), textures.begin(), textures.end());
			if (textures.size() < m_Shader->getTextureNameMap().size())
			{
				m_Textures.insert(m_Textures.end(), m_Shader->getTextureNameMap().size() - textures.size(), getDefaultWhiteTexture());
			}
		}
	}

	_Material::~_Material()
	{
		deleteMaterialUniforms();
	}

	void _Material::setTexture(const char* slotName, Texture texture)
	{
		m_Textures[m_Shader->getTextureNameMap().at(slotName)] = texture;
	}

	Texture _Material::getTexture(const char* slotName) const
	{
		return m_Textures.at(m_Shader->getTextureNameMap().at(slotName));
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

	void _Material::setMaterialUniform(const char* uniformName, const void* data, unsigned int dataType)
	{
		// Check if uniform has already been set with that name
		for (MaterialUniform& it : m_MaterialUniforms)
		{
			if (it.uniformID == uniformName)
			{
				if (it.type != dataType)
				{
					debugLog("Tried to set a material uniform (\"" + std::string(uniformName) + "\") with an mismatched dataType", LOST_LOG_WARNING);
					return;
				}

				// Copy the data in from the pointer given to the location on the heap we are storing it
				// We need to use memcpy because it is a variable type, we just need to copy the data
				memcpy_s(it.data, getBytesForType(it.type), data, getBytesForType(dataType));
				return;
			}
		}

		// Uniform had not been set before, we need to initialize it

		// Check if the shader used has a matching uniform
		if (m_Shader->getUniformNameMap().count(uniformName))
		{
			MaterialUniform uniform = {};

			uniform.uniformID = uniformName;
			uniform.type = dataType;
			uniform.location = m_Shader->getUniformLocation(uniformName);

			// Intialize the memory on the heap
			size_t uniformDataSize = getBytesForType(uniform.type); // Get the size of the data given
			uniform.data = ::operator new(uniformDataSize); // Initialize the area of data on the heap
			memcpy_s(uniform.data, uniformDataSize, data, uniformDataSize); // Copy the data into that allocated space

			m_MaterialUniforms.push_back(uniform);
		}
	}

	void _Material::deleteMaterialUniforms()
	{
		for (MaterialUniform& it : m_MaterialUniforms)
			::operator delete(it.data);
	}

	void _Material::bindMaterialUniforms()
	{
		for (MaterialUniform& it : m_MaterialUniforms)
			m_Shader->setUniform(it.data, it.location, it.type);
	}


	void setMaterialUniform(Material mat, const char* uniformName, const void* data, unsigned int dataType)
	{
		mat->setMaterialUniform(uniformName, data, dataType);
	}

}