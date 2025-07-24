#pragma once
#include "Sounds.h"
#include "ResourceManagers/AudioResourceManagers.h"

#include "External/RtAudio.h"
#include "ThreadSafeTemplate.h"

// [!] TODO: Playback speed option ??? Would require subsampling and sample rate changes
// [!] TODO: Change audio device option
// [!] TODO: Audio device list option, need to filter by audio OUTPUTS

#define LOST_AUDIO_BUFFER_FRAMES 1024 // The amount of samples out to EACH channel
#define LOST_AUDIO_CHANNELS 2 // Will ALWAYS be 2!!!
#define LOST_AUDIO_BUFFER_COUNT 2048 // LOST_AUDIO_CHANNELS * LOST_AUDIO_BUFFER_FRAMES, the amount of samples in total
#define LOST_AUDIO_SAMPLE_RATE 44100

namespace lost
{

#if defined(LOST_AUDIO_QUALITY_HIGH)
	typedef int32_t _ChannelQuality;
	constexpr int _EngineBytesPerSample = 4;
	constexpr RtAudioFormat _RTAudioFormat = RTAUDIO_SINT32;
#elif defined(LOST_AUDIO_QUALITY_LOW)
	typedef int8_t _ChannelQuality;
	constexpr int _EngineBytesPerSample = 1;
	constexpr RtAudioFormat _RTAudioFormat = RTAUDIO_SINT8;
#else // LOST_AUDIO_QUALITY_MEDIUM
	typedef int16_t _ChannelQuality;
	constexpr RtAudioFormat _RTAudioFormat = RTAUDIO_SINT16;
	constexpr int _EngineBytesPerSample = 2;
#endif

	// A typedef to the quality of the audio engine
	// Is mainly used in effects, evaluates to an integer
	typedef _ChannelQuality AudioSample;

	float audioSampleToFloat(AudioSample sample);
	AudioSample floatToAudioSample(float sample);

	float PCM32ToFloat(int32_t val);
	float PCM16ToFloat(int16_t val);
	float PCM8ToFloat(int8_t val);
	int32_t floatToPCM32(float val);
	int16_t floatToPCM16(float val);
	int8_t floatToPCM8(float val);

	class _Sound;
	class Effect;
	
	struct _PlaybackData
	{
		unsigned int currentByte;    // The sample the playback is currently at
		unsigned int loopCount;      // The amount of times the sound should loop -1 / UINT_MAX
		unsigned int dataCount;      // The amount of bytes left in the data
		unsigned int format;         // The amount of bytes in each channel's samples;
		int			 formatFactor;   // The amount of bits to shift the playback of this sound
		unsigned int bytesPerSample; // The amount of bytes in one channel's sample
		unsigned int channelCount;   // The amount of channels
		const char* data; // This is not normalized audio, it is the bit representation of the audio, not caring for format

		void* extraData = nullptr;
	};

	// Returned whenever a sound object is played
	class PlaybackSound
	{
	public:
		PlaybackSound(_Sound* soundPlaying, float volume = 1.0f, float panning = 0.0f, unsigned int loopCount = 0); // if loop count is -1 or UINT_MAX it will loop forever

		inline bool isPlaying() { return a_Playing.read(); };

		inline void _setIsPlaying() { a_Playing.write(false); };
		inline _PlaybackData& _getPlaybackData() { return a_PlaybackData; };

		inline bool _getPaused() { return a_Paused.read(); };
		inline void _setPaused(bool paused) { a_Paused.write(paused); };

		inline float _getVolume() { return a_Volume.read(); };
		inline void _setVolume(float volume) { a_Volume.write(volume); }

		inline float _getPanning() { return a_Panning.read(); };
		inline void _setPanning(float panning) { a_Panning.write(fminf(fmaxf(panning, -1.0f), 1.0f)); }

		inline _Sound* getParentSound() const { return m_ParentSound; };
	private:
		// Any variables marked with a_ are accessed by the audio thread, otherwise they are main thread only
		_PlaybackData a_PlaybackData; // Read only
		// Halt read is used here so that the main thread waits for the audio thread to finish processing
		_HaltRead<bool> a_Playing; // Read/Write

		_HaltWrite<float> a_Volume;
		_HaltWrite<float> a_Panning;
		_HaltWrite<bool>  a_Paused;

		// Playback data
		int m_FormatFactor;

		_Sound* m_ParentSound;
	};

	void _initAudio();
	void _exitAudio();
	void _updateAudio();

	unsigned int _getAudioHandlerFormat();
	unsigned int _getAudioHandlerBufferSize(); // Returns the size of the nBufferSize of the audio buffer

	// [!] TODO: Docs

	// Sets the master volume of the audio in the engine, effects every sound
	void setMasterVolume(float volume);
	float getMasterVolume();

	// [==========================]
	//       Sound Functions
	// [==========================]

	/// <summary>
	/// Returns a pointer to the sound being played, you can check if the sound has finished playing or not by running isSoundPlaying()
	/// </summary>
	/// <param name="sound">The sound to play</param>
	/// <param name="volume">The volume of the sound</param>
	/// <param name="panning">The panning of the sound -1.0f is left ear, 1.0f is right ear, 0.0f is center</param>
	/// <param name="loopCount">The amount of times to loop, setting this as -1 or UINT_MAX loops it forever. It can be stopped with stopSound</param>
	/// <returns></returns>
	PlaybackSound* playSound(Sound sound, float volume = 1.0f, float panning = 0.0f, unsigned int loopCount = 0); 
	
	// Stops the sound being played
	void stopSound(const PlaybackSound* sound);
	// Stops the all sounds playing that are using this sound, use the PlaybackSound return to manage individual ones
	void stopSound(Sound sound);
	void setSoundPaused(PlaybackSound* sound, bool paused);

	void setSoundVolume(PlaybackSound* sound, float volume);
	// The panning of the sound -1.0f is left ear, 1.0f is right ear, 0.0f is center
	void setSoundPanning(PlaybackSound* sound, float panning);

	bool isSoundPlaying(PlaybackSound* sound);

	// [==========================]
	//    Sound Stream Functions
	// [==========================]

	/// <summary>
	/// Starts playing a sound stream, a sound stream can only be played once at a time unlike sounds
	/// </summary>
	/// <param name="soundStream">The sound to play</param>
	/// <param name="volume">The volume of the sound</param>
	/// <param name="panning">The panning of the sound -1.0f is left ear, 1.0f is right ear, 0.0f is center</param>
	/// <param name="loopCount">The amount of times to loop, setting this as -1 or UINT_MAX loops it forever</param>
	/// <returns></returns>
	void playSoundStream(SoundStream soundStream, float volume = 1.0f, float panning = 0.0f, unsigned int loopCount = 0);
	void stopSoundStream(SoundStream soundStream); 
	
	void setSoundStreamPaused(SoundStream soundStream, bool paused);

	void setSoundStreamVolume(SoundStream sound, float volume);
	// The panning of the sound -1.0f is left ear, 1.0f is right ear, 0.0f is center
	void setSoundStreamPanning(SoundStream sound, float panning);

	bool isSoundStreamPlaying(SoundStream sound); 

	// [==========================]
	//    Audio Effect Functions
	// [==========================]

	void addGlobalEffect(Effect* effect);
	void removeGlobalEffect(Effect* effect);
	void addEffectToSound(Effect* effect, PlaybackSound* sound);
	void removeEffectFromSound(Effect* effect, PlaybackSound* sound);
	void addEffectToSoundStream(Effect* effect, SoundStream soundStream);
	void removeEffectFromSoundStream(Effect* effect, SoundStream soundStream);
}