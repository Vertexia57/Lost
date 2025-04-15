#include "PostProcessingShader.h"

#include "../Renderer.h"
#include "../LostGL.h"

namespace lost
{

	static void _standardFunction(PostProcessingShader shader)
	{
		shader->getShader(0)->bind();

		// Render across the entire screen, this is rendered to the current render buffer textures
		_renderFullScreenQuadImmediate();
	}

	_PostProcessingShader::_PostProcessingShader(Shader shader, void (*funcOverride)(_PostProcessingShader*))
		: m_Shaders({ shader })
	{
		m_FunctionOverride = (funcOverride == nullptr ? &_standardFunction : funcOverride);
	}

	_PostProcessingShader::_PostProcessingShader(const std::vector<Shader>& shaders, void(*funcOverride)(_PostProcessingShader*))
		: m_Shaders(shaders)
	{
		m_FunctionOverride = (funcOverride == nullptr ? &_standardFunction : funcOverride);
	}

	_PostProcessingShader::~_PostProcessingShader()
	{
	}

	void _PostProcessingShader::run()
	{
		// Render the instance queue, we want anything that was called to get rendered before this function rendered
		renderInstanceQueue();

		// Get the render textures of the current window
		const std::vector<unsigned int>& renderBufferTextures = getRenderTextures();

		// Set the inputs for the texture, the outputs are already set by the beginFrame function
		for (int i = 0; i < renderBufferTextures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, renderBufferTextures.at(i));
		}

		m_FunctionOverride(this);
	}

	Shader _PostProcessingShader::getShader(unsigned int index)
	{
		return m_Shaders.at(index);
	}

	void runPostProcessingShader(PostProcessingShader shader)
	{
		shader->run();
	}

}