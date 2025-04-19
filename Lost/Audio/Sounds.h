#pragma once

#include "External/RtAudio.h"
#include "ThreadSafeTemplate.h"

namespace lost
{

	class _Sound;

	struct _SoundInfo
	{
		// Constants, sound information
		unsigned int sampleRate;
		unsigned int channelCount;
		unsigned int byteCount;   // The count of the sound data in bytes
		unsigned int sampleCount; // The count of samples in the sound
		unsigned int sampleSize;  // The count of bytes in a single sample (formatSize * channelCount)
		unsigned int bitsPerSample;
		RtAudioFormat format;
	};

	// Initlializes the sound onto RAM, this is very innefficient for large sounds like music
	class _Sound
	{
	public:
		_Sound();
		~_Sound();

		void _initializeWithFile(const char* fileLocation);
		void _initializeWithRaw(void* data, size_t dataSize);

		void _destroy();

		inline const char* getData()     const { return m_Data; };
		inline unsigned int getDataSize() const { return m_SoundInfo.byteCount; };

		inline const _SoundInfo& _getSoundInfo() const { return m_SoundInfo; };

		bool isFunctional() const { return m_Functional; };
	private:
		_SoundInfo m_SoundInfo;
		char* m_Data;

		// Local
		bool m_Functional;
	};

	class _SoundStream
	{
	public:
		_SoundStream();
		~_SoundStream();

		void _initializeWithFile(const char* fileLocation);

		void _initializeStream();

		void _destroy();
	private:
	};

	typedef _Sound* Sound;
	typedef _SoundStream* SoundStream;

}