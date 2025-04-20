#include "AudioResourceManagers.h"

#include <unordered_map>
#include <iostream>
#include <algorithm>

#include "../Audio.h"

template <typename Out>
static void split(const std::string& s, char delim, Out result) {
	std::istringstream iss(s);
	std::string item;
	while (std::getline(iss, item, delim)) {
		*result++ = item;
	}
}

static std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

struct IndexData
{
	int vertex;
	int texcoord;
	int normal;
	std::size_t hash = 0;

	IndexData(int _vertex, int _texcoord, int _normal)
	{
		vertex = _vertex;
		texcoord = _texcoord;
		normal = _normal;
		hash = ((std::hash<int>()(vertex)
			^ (std::hash<int>()(texcoord) << 1)) >> 1)
			^ (std::hash<int>()(normal) << 1);
	};

	bool operator==(const IndexData& other) const
	{
		return vertex == other.vertex && texcoord == other.texcoord && normal == other.normal;
	}
};

template <>
struct std::hash<IndexData>
{
	std::size_t operator()(const IndexData& k) const
	{
		return k.hash;
	}
};

namespace lost
{

	ResourceManager<Sound>* _soundRM = nullptr;
	ResourceManager<SoundStream>* _streamRM = nullptr;

	void _initAudioRMs()
	{
		_soundRM = new ResourceManager<Sound>("Sounds");
		_streamRM = new ResourceManager<SoundStream>("Sound Streams");
	}

	void _destroyAudioRMs()
	{
		delete _soundRM;
		delete _streamRM;
	}

	Sound loadSound(const char* soundLoc, const char* id)
	{
		lost::Sound sound = nullptr;

		// If "id" is nullptr set it to the filename
		if (!id) id = soundLoc;

		if (!_soundRM->hasValue(id))
		{
			sound = new _Sound();
			sound->_initializeWithFile(soundLoc);
		}
		else
			sound = _soundRM->getValue(id);

		_soundRM->addValue(sound, id);
		return sound;
	}

	Sound getSound(const char* id)
	{
		return _soundRM->getValue(id);
	}

	void unloadSound(const char* id)
	{
		_soundRM->destroyValue(id);
	}

	void unloadSound(Sound& sound)
	{
		_soundRM->destroyValueByValue(sound);
	}

	void forceUnloadSound(const char* id)
	{
		_soundRM->forceDestroyValue(id);
	}

	void forceUnloadSound(Sound& sound)
	{
		_soundRM->forceDestroyValueByValue(sound);
	}

	SoundStream loadSoundStream(const char* soundLoc, const char* id)
	{
		lost::SoundStream sound = nullptr;

		// If "id" is nullptr set it to the filename
		if (!id) id = soundLoc;

		if (!_streamRM->hasValue(id))
		{
			sound = new _SoundStream(_getAudioHandlerBufferSize());
			sound->_initializeWithFile(soundLoc);
		}
		else
			sound = _streamRM->getValue(id);

		_streamRM->addValue(sound, id);
		return sound;
	}

	SoundStream getSoundStream(const char* id)
	{
		return _streamRM->getValue(id);
	}

	void unloadSoundStream(const char* id)
	{
		_streamRM->destroyValue(id);
	}

	void unloadSoundStream(SoundStream& sound)
	{
		_streamRM->destroyValueByValue(sound);
	}

	void forceUnloadSoundStream(const char* id)
	{
		_streamRM->forceDestroyValue(id);
	}

	void forceUnloadSoundStream(SoundStream& sound)
	{
		_streamRM->forceDestroyValueByValue(sound);
	}

}