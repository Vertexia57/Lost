#pragma once
#include "Sounds.h"
#include "ResourceManagers/AudioResourceManagers.h"

#include "External/RtAudio.h"
#include "ThreadSafeTemplate.h"

namespace lost
{
	class _Sound;
	
	struct _PlaybackData
	{
		unsigned int currentByte;    // The sample the playback is currently at
		unsigned int dataCount;      // The amount of bytes left in the data
		unsigned int formatFactor;   // The amount of bits to shift the playback of this sound
		unsigned int bytesPerSample; // The amount of bytes in one channel's sample
		const char* data; // This is not normalized audio, it is the bit representation of the audio, not caring for format

		void* extraData = nullptr;
	};

	// Returned whenever a sound object is played
	class PlaybackSound
	{
	public:
		PlaybackSound(const _Sound* soundPlaying);

		inline bool isPlaying() { return a_Playing.read(); };

		inline void _setIsPlaying() { a_Playing.write(false); };
		inline _PlaybackData& _getPlaybackData() { return a_PlaybackData; };
	private:
		// Any variables marked with a_ are accessed by the audio thread, otherwise they are main thread only
		_PlaybackData a_PlaybackData; // Read only
		// Halt read is used here so that the main thread waits for the audio thread to finish processing
		_HaltRead<bool> a_Playing; // Read/Write

		// Playback data
		unsigned int m_FormatFactor;
	};

	void _initAudio();
	void _exitAudio();
	void _updateAudio();

	void setAudioOutputDevice();

	// Returns a pointer to the sound being played, you can check if the sound has finished playing or not by dereferencing it
	const PlaybackSound* playSound(Sound sound); // [!] TODO: Volume and Panning 
	// Stops the sound being played
	void stopSound(const PlaybackSound* sound);

}
//
//// Two-channel sawtooth wave generator.
//int saw(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
//	double streamTime, RtAudioStreamStatus status, void* userData)
//{
//	static unsigned int iterate = 0;
//	
//	const float volume = 0.01f;
//	const float hzList[] = { 261.6, 329.6, 392.0 };
//	const int noteCount = 3;
//	const int octave = 2;
//
//	unsigned int i, j;
//	double* buffer = (double*)outputBuffer;
//	double* lastValues = (double*)userData;
//
//	if (status)
//		std::cout << "Stream underflow detected!" << std::endl;
//
//	// Write interleaved audio data.
//	for (i = 0; i < nBufferFrames; i++) {
//		for (j = 0; j < 2; j++) {
//			*buffer++ = lastValues[j];
//			iterate++;
//
//			for (int noteIndex = 0; noteIndex < noteCount; noteIndex++)
//				lastValues[j] += sinf((double)iterate * (1.0 / 44100.0) * (double)hzList[noteIndex] * (double)octave) * volume;
//		}
//	}
//
//	return 0;
//}