#include "State.h"
#include "Log.h"
#include <glad/glad.h>
#include <map>
#include "GL/Renderer.h"

namespace lost
{

	LostState _state = {};

	std::map<unsigned int, const char*> _formatToName = {
		{ LOST_FORMAT_R, "R" },
		{ LOST_FORMAT_RG, "RG" },
		{ LOST_FORMAT_RGB, "RGB" },
		{ LOST_FORMAT_RGBA, "RGBA" },
	};

	const char* formatToName(unsigned int format)
	{
		return _formatToName.at(format);
	}

	void setErrorMode(unsigned int mask, bool state)
	{
		// Set the individual bit given in mask to state
		_state.errorMode = (_state.errorMode & ~mask) | (mask * state);
	}

	bool getErrorMode(unsigned int mask)
	{
		return _state.errorMode & mask;
	}

	void setStateData(unsigned int which, const void* data)
	{
		switch (which)
		{
		case LOST_STATE_TEXTURE_SLOT:
#ifdef LOST_DEBUG_MODE
			if (_state.lostGLInitialized)
			{
				debugLog("Tried to change LOST_STATE_TEXTURE_SLOT after LostGL had been initialized.\nPut lost::setStateData(LOST_STATE_TEXTURE_SLOT, x) BEFORE lost::init()", LOST_LOG_ERROR);
				break;
			}

			if ((int)data > GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
			{
				debugLog(
					std::string("Tried to set the amount of texture slots used to a number greater than what OpenGL supports (") + 
					std::to_string(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS) + " < " + std::to_string((int)data) + ")\n" +
					"Change was not set, maybe \"data\" was set improperly?", 
					LOST_LOG_WARNING
				);
				break;
			}
#endif
			_state.textureSlots = (int)data;
			break;
		case LOST_STATE_USE_DATA_IDS:
			_state.useTextureIDs = (bool)data;
			break;
		case LOST_STATE_GL_INITIALIZED:
			_state.lostGLInitialized = (bool)data;
			break;
		case LOST_STATE_WINDOW_FULLSCREEN:
			_state.usingNonWindowedFullscreen = (bool)data;
			break;
		case LOST_STATE_RENDERER_MODE:
#ifdef LOST_DEBUG_MODE
			if (_state.lostGLInitialized)
			{
				debugLog("Tried to change the renderer mode after LostGL had been initialized.", LOST_LOG_ERROR);
				break;
			}
#endif
			_state.rendererMode = (unsigned int)data;
			_state.currentBuffers = (_state.rendererMode == LOST_RENDER_3D) ? _default3DBuffers : _default2DBuffers;
			_state.currentBuffers.insert(_state.currentBuffers.end(), _state.buffersToAdd.begin(), _state.buffersToAdd.end());
			break;
		default:
			debugLog("The state enum given wasn't recognised, invalid enum value?", LOST_LOG_WARNING);
			break;
		}
	}

	//void* getStateData(unsigned int which)
	//{
	//	switch (which)
	//	{
	//	case LOST_STATE_TEXTURE_SLOT:
	//		return &_state.textureSlots;
	//	case LOST_STATE_GL_INITIALIZED:
	//		return &_state.lostGLInitialized;
	//	default:
	//		debugLog("The state enum given wasn't recognised, invalid enum value?", LOST_LOG_WARNING);
	//		return nullptr;
	//	}
	//}

	const LostState& getLostState()
	{
		return _state;
	}

	void addOutputBuffer(const char* bufferName, Vec4 fillColor, unsigned int format)
	{
		if (_state.buffersToAdd.size() < 8)
			_state.buffersToAdd.push_back(RenderBufferData{ bufferName, format, fillColor });
		else
		{
			lost::log("Tried to create more than 8 output buffers, which isn't supported by OpenGL", LOST_LOG_ERROR);
		}
	}

	void setDefaultMode()
	{
		if (!_state.lostGLInitialized)
		{
			_state.errorMode = _defaultErrorMode;
			_state.textureSlots = _defaultTextureSlots;
		}
		else
		{
			debugLog("Tried to set the Lost State to it's original settings after openGL was initialized", LOST_LOG_WARNING);
		}
	}

}