#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>
#include "GL/Vector.h"

#ifdef NDEBUG // VSCC default preprocessor
//#define LOST_RELEASE_MODE
#endif

#ifndef LOST_RELEASE_MODE // If not in release mode, enter debug mode
#define LOST_DEBUG_MODE
#define LOST_IS_DEBUG_MODE true
#else
#define LOST_IS_DEBUG_MODE false
#endif

// A bit mask used in lost::setErrorMode() and lost::getErrorMode()
enum ErrorMode
{
	LOST_ERROR_MODE_SHADER = 0x1
};

enum StateData
{
	// External
	LOST_STATE_TEXTURE_SLOT,
	LOST_STATE_USE_DATA_IDS,

	// Internal
	LOST_STATE_GL_INITIALIZED,
	LOST_STATE_WINDOW_FULLSCREEN,
	LOST_STATE_RENDERER_MODE
};

enum BufferFormats
{
	LOST_FORMAT_RGBA = GL_RGBA,
	LOST_FORMAT_RGB = GL_RGB,
	LOST_FORMAT_RG = GL_RG,
	LOST_FORMAT_R = GL_RED
};

namespace lost
{

	const char* formatToName(unsigned int format);

	struct RenderBufferData
	{
		std::string name;
		unsigned int format = LOST_FORMAT_RGBA;
		Vec4 defaultColor = { 0.0, 0.0, 0.0, 1.0 };
	};

	static constexpr unsigned int _defaultTextureSlots = 3;
	static constexpr unsigned int _defaultErrorMode = LOST_ERROR_MODE_SHADER;
	static std::vector<RenderBufferData> _default2DBuffers = {
			{ "color", LOST_FORMAT_RGBA, { 0.0, 0.0, 0.0, 1.0 } }
	};
	static std::vector<RenderBufferData> _default3DBuffers = {
			{ "color", LOST_FORMAT_RGBA, { 0.0, 0.0, 0.0, 1.0 } }
	};

	// The global state of the lost engine
	struct LostState
	{
		const bool debugMode = LOST_IS_DEBUG_MODE;

		// If true lost will use texture IDs and store extra data for each image
		bool useTextureIDs = true;
		// The amount of textureSlots the shaders will check for and bind
		unsigned int textureSlots = _defaultTextureSlots;
		// The current state of all error modes, using the enum ErrorMode
		unsigned int errorMode = _defaultErrorMode;

		bool lostGLInitialized = false;
		bool usingNonWindowedFullscreen = false;
		unsigned int rendererMode = 0;

		std::vector<RenderBufferData> buffersToAdd = {};
		std::vector<RenderBufferData> currentBuffers = _default2DBuffers;
	};

	// Sets the mode of the error state at the enum given's location, uses the enum ErrorMode
	void setErrorMode(unsigned int mask, bool state);
	// Gets the mode of the error state at the enum given's location, uses the enum ErrorMode
	bool getErrorMode(unsigned int mask);

	// Sets the data at the location given in "which" with the StateData enum.
	// "data" is not always interperated as a pointer, void* is only being used to show an arbitrary type.
	// In code "data" is cast with (int)data, it does not dereference data, just using the bits that "data" has and casting them to the type used
	void setStateData(unsigned int which, const void* data);
	
	// Returns the data that the state currently has at the location given in "which".
	// This void* is a pointer to the data at "which", which will need to be cast and dereferenced
	// void* getStateData(unsigned int which);
	// This function is gross and makes code bad

	// Returns the current state of the lost engine
	const LostState& getLostState();

	// Adds a output buffer to the renderer, these buffers are added on lost::init(), by default fills with black, regardless of format
	void addOutputBuffer(const char* bufferName, Vec4 fillColor = { 0.0f, 0.0f, 0.0f, 1.0f }, unsigned int format = LOST_FORMAT_RGBA);

	// Reverts the current Lost State to it's default settings
	void setDefaultMode();

}