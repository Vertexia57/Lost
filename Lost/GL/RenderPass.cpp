#include "RenderPass.h"
#include "LostGL.h"
#include <iostream>

namespace lost
{
	_RenderPass::_RenderPass(int width, int height, std::vector<RenderBufferData> buffers, Window context)
	{
		makePass(width, height, buffers, context);
	}

	_RenderPass::~_RenderPass()
	{
		glDeleteTextures(textures.size(), textures.data());
		glDeleteTextures(1, &depthStencilTexture);
		glDeleteBuffers(1, &FBO);
	}

	void _RenderPass::makePass(int width, int height, std::vector<RenderBufferData> buffers, Window context)
	{
		pushWindow();
		glfwMakeContextCurrent(context->glfwWindow);

		storedBuffers = buffers;

		debugLogIf(buffers.size() > 32, "Tried to create render pass with more buffers than OpenGL supports (" + std::to_string(buffers.size()) + " > 32)", LOST_LOG_ERROR);

		// Generate Frame Buffer
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		// Create color texture
		textures.resize(buffers.size());
		glGenTextures(buffers.size(), textures.data());
		for (int i = 0; i < textures.size(); i++)
		{
			textureMap[buffers[i].name] = textures[i];

			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, buffers[i].format, width, height, 0, buffers[i].format, GL_UNSIGNED_BYTE, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);

			setClearColor(i, buffers[i].defaultColor);
		}

		// Create depth stencil
		glGenTextures(1, &depthStencilTexture);
		glBindTexture(GL_TEXTURE_2D, depthStencilTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthStencilTexture, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		popWindow();
	}

	void _RenderPass::setClearColor(int passID, lost::Vec4 color)
	{
		// Store current frame buffer to return to afterwards
		int activeFrameBuffer = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &activeFrameBuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		unsigned int buffers[1] = { GL_COLOR_ATTACHMENT0 + passID };
		glDrawBuffers(1, buffers);
		glClearColor(color.x, color.y, color.z, color.w); // Only set the clear color in the active draw buffer

		glBindFramebuffer(GL_FRAMEBUFFER, activeFrameBuffer); // Return back to old frame buffer
	}

	void _RenderPass::resize(int width, int height, Window context)
	{
		if (width > 0 && height > 0)
		{
			pushWindow();
			glfwMakeContextCurrent(context->glfwWindow);

			int activeFrameBuffer = 0;
			glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &activeFrameBuffer);

			glBindFramebuffer(GL_FRAMEBUFFER, FBO);

			for (int i = 0; i < textures.size(); i++)
			{
				glBindTexture(GL_TEXTURE_2D, textures[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, storedBuffers[i].format, width, height, 0, storedBuffers[i].format, GL_UNSIGNED_BYTE, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
			}

			glBindTexture(GL_TEXTURE_2D, depthStencilTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthStencilTexture, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, activeFrameBuffer); // Return back to old frame buffer

			popWindow();
		}
	}

	void _RenderPass::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		std::vector<unsigned int> buffers;
		buffers.reserve(textures.size());
		for (int i = 0; i < textures.size(); i++)
			buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
		glDrawBuffers(buffers.size(), buffers.data());

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	}

	std::stack<RenderTexture*> _renderTextureStack;

	RenderTexture::RenderTexture(int width, int height)
		: m_RenderPass(width, height, lost::getLostState().currentBuffers, lost::getCurrentWindow() ? lost::getCurrentWindow() : lost::getWindow())
	{
		std::vector<unsigned int>& currentTextures = m_RenderPass.textures;
		unsigned int index = 0;
		for (unsigned int buffer : currentTextures)
		{
			lost::Vec4 color = m_RenderPass.storedBuffers[index].defaultColor;
			m_RenderPass.storedBuffers[index].defaultColor = { color.r, color.g, color.b, 0.0f };
			index++;

			_Texture* texture = new _Texture();
			texture->makeTexture(buffer, false);
			m_Textures.push_back(texture);
		}
	}

	RenderTexture::~RenderTexture()
	{
		//delete m_Material;
		for (Texture texture : m_Textures)
			delete texture;
	}

	void RenderTexture::clear()
	{
		for (int i = 0; i < m_RenderPass.storedBuffers.size(); i++)
		{
			const RenderBufferData& renderBuffer = m_RenderPass.storedBuffers[i];
			glClearBufferfv(GL_COLOR, i, renderBuffer.defaultColor.v);
		}

	}

	void RenderTexture::bind()
	{
		_renderTextureStack.push(this);
		lost::renderInstanceQueue();
		_bindPass();
	}

	void RenderTexture::unbind()
	{
		std::stack<RenderTexture*>& stack = _renderTextureStack;
		if (stack.top() == this)
			stack.pop();
		else
		{
			debugLog("Tried to unbind a render texture that wasn't currently bound", LOST_LOG_ERROR);
			return;
		}

		lost::renderInstanceQueue();

		if (stack.empty())
		{
			lost::_getCurrentRenderPass()->bind();
		}
		else
		{
			stack.top()->_bindPass();
		}
	}

	void RenderTexture::resize(int width, int height)
	{
		m_RenderPass.resize(width, height, lost::getCurrentWindow());
		for (int i = 0; i < m_Textures.size(); i++)
		{
			m_Textures.at(i)->makeTexture(m_RenderPass.textures.at(i), false);
		}
	}

	Texture RenderTexture::getTexture(int slot) const
	{
		return m_Textures.at(slot);
	}

	void RenderTexture::_bindPass()
	{
		m_RenderPass.bind();
	}

}
