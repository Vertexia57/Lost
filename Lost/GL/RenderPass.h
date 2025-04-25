#pragma once
#include <glad/glad.h>
#include <map>
#include <vector>
#include <string>
#include "../Log.h"
#include "vector.h"
#include "WindowContext.h"
#include "Vector.h"

#include "Texture/Texture.h"

namespace lost
{

	struct _RenderPass
	{
		_RenderPass(int width, int height, std::vector<RenderBufferData> buffers, Window context);
		~_RenderPass();

		std::vector<RenderBufferData> storedBuffers;
		unsigned int FBO; // Frame Buffer Object
		std::vector<unsigned int> textures;
		std::map<std::string, unsigned int> textureMap;
		unsigned int depthStencilTexture;

		void makePass(int width, int height, std::vector<RenderBufferData> buffers, Window context);
		void setClearColor(int passID, lost::Vec4 color);
		void resize(int width, int height, Window context);
		void bind();
	};

	// A reference to a renderpass
	typedef _RenderPass* RenderPass;

	class RenderTexture
	{
	public:
		RenderTexture(int width, int height);
		~RenderTexture();

		void clear();

		void bind();
		void unbind();

		void resize(int width, int height);

		Texture getTexture(int slot) const;
		//Material getMaterial() const;

	private:
		void _bindPass();

		//Material m_Material;
		std::vector<Texture> m_Textures;

		_RenderPass m_RenderPass;
	};

	extern std::stack<RenderTexture*> _renderTextureStack;
}