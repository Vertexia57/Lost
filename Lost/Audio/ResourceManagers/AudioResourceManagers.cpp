#include "AudioResourceManagers.h"

#include <unordered_map>
#include <iostream>
#include <algorithm>

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

	void _initAudioRMs()
	{
		_soundRM = new ResourceManager<Sound>("Sounds");
	}

	void _destroyAudioRMs()
	{
		delete _soundRM;
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

}