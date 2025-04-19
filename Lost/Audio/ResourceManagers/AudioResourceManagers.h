#pragma once
#include "../../ResourceManager.h"
#include "../Sounds.h"

namespace lost
{

	extern ResourceManager<Sound>* _soundRM;

	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	extern void _initAudioRMs();
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	extern void _destroyAudioRMs();

	// Sounds Load Functions
	Sound loadSound(const char* soundLoc, const char* id = nullptr);
	Sound getSound(const char* id);
	void  unloadSound(const char* id);
	void  unloadSound(Sound& sound);
	void  forceUnloadSound(const char* id);
	void  forceUnloadSound(Sound& sound);
}