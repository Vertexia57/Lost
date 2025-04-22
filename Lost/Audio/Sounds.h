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
		_SoundStream(unsigned int bufferSize = 512);
		~_SoundStream();

		void _initializeWithFile(const char* fileLocation);
		void _destroy();

		inline const _SoundInfo& _getSoundInfo() const { return m_SoundInfo; };

		// Only used by main thread
		inline void _setActive(bool active) { m_Active = active; };
		// Only used by main thread
		inline bool getActive() const { return m_Active; };

		// Ran by the audio thread
		unsigned int _getCurrentByte() const;   // The current byte the sound stream is at
		unsigned int _getDataByteSize() const;  // The size of the entire file's data section
		unsigned int _getDataBlockSize() const; // The size of the buffer in bytes
		unsigned int _getBytesLeftToPlay() const;
		unsigned int _getFormatFactor() const;
		unsigned int _getLoopCount() const;
		inline float _getVolume() { return a_Volume.read(); };
		inline float _getPanning() { return a_Panning.read(); };
		inline void  _setVolume(float volume) { a_Volume.write(volume); };
		inline void  _setPanning(float panning) { a_Panning.write(panning); };
		const char* _getNextDataBlock();

		inline bool isPlaying()					{ return a_Playing.read();  };
		inline void _setIsPlaying(bool playing) { a_Playing.write(playing); };

		void _prepareStartPlay(float volume, float panning, unsigned int loopCount);

		inline bool isFunctional() { return m_Functional; };
		// Ran by the file load thread (NOT MAIN)
		void _fillBuffer();
	private:
		FILE* m_File;

		// Anything marked with an "a" at the start is used by the audio thread
		// Anything marked with an "m" is a member variable that is only initialized when created
		// Anything marked with an "f" is used by the file read thread

		_SoundInfo m_SoundInfo;
		unsigned int a_FormatFactor;

		unsigned int a_CurrentByte;

		_HaltWrite<bool> a_Playing; // Actively being used by the audio thread
		bool m_Active;              // True even if it's in garbage
		unsigned int a_LoopCount;

		_HaltWrite<float> a_Volume;
		_HaltWrite<float> a_Panning;

		std::mutex a_FillingBufferMutex;
		std::thread a_FillingBufferThread;

		bool a_UsingBBuffer;
		unsigned int m_BufferSize; // The amount of samples in the buffer
		unsigned int m_ByteSize;   // The size of the buffer in bytes
		char* a_Buffer; // Twice the length of bufferSize, using dual buffering

		// Local
		bool m_Functional;
	};

	typedef _Sound* Sound;
	typedef _SoundStream* SoundStream;

}