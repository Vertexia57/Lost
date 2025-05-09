#include "Audio.h"
#include "../Log.h"
#include "../DeltaTime.h"

#include "ResourceManagers/AudioResourceManagers.h"

#include <stdio.h>
#include <vector>

typedef short _ChannelQuality;

namespace lost
{

	class _Sound;
	class _SoundStream;

	struct SamplerPassInInfo
	{
		_HaltWrite<std::vector<PlaybackSound*>> activeSounds;
		_HaltWrite<std::vector<_SoundStream*>>  activeStreams;
	};

	int playRaw(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
		double streamTime, RtAudioStreamStatus status, void* data)
	{

		// This will compile all sounds and soundstreams into one buffer
		
		// Since the audio is on a different thread we need to make sure we don't run into any
		// race conditions. We can use mutex to do this

		// Solution of race conditions:
		// Update function is ran:
		//  > Main thread will pause when audio thread is using main_sounddata
		//  > It will check the status of each sound
		//  > Then it will remove ownership of main_sounddata
		// When sound is added or removed:
		//  > Main thread will pause when audio thread is using main_sounddata
		//  > Then it will take ownership of the main_sounddata
		//  > Then it will add / remove the sound
		//  > Then remove ownership of main_sounddata
		// When this function is ran:
		// 
		// All sounds are ran with audio_sounddata, this lets audio work while the main thread is adding sounds
		// 
		// Note: audio_sounddata is never accessed by the main thread, and so the audio thread owns it completely.
		
		// The sounds may be in different PCM formats, we will need to convert them into the same as this one
		// It should be processed beforehand

		// Eg. Sound format is 32-PCM and out is 16-PCM
		// Sound in >> 16 (Bitshift the input 16 bits to the right)
		// Eg. Sound format is 8-PCM and out is 16-PCM
		// Sound in << 8 (Bitshift the input 8 bits to the left)

		// The "middle man" format will need to be the maximum that RtAudio supports
		// which is 32 bits/an integer

		SamplerPassInInfo* inData = (SamplerPassInInfo*)data;
		
		// Will ALWAYS be 2!!!
		const unsigned int channelCount = 2;
		const float volumeDecrement = 1.0f / 4.0f; // The second number is prefered to be a power of 2

		_ChannelQuality* outData = (_ChannelQuality*)outputBuffer;
		unsigned int outDataCapacity = sizeof(_ChannelQuality) * nBufferFrames * channelCount;

		memset((char*)outputBuffer, 0, outDataCapacity);

		// [----------------------]
		//       Update Sounds
		// [----------------------]

		std::vector<PlaybackSound*> sounds = inData->activeSounds.read();
		for (unsigned int i = 0; i < sounds.size(); i++)
		{
			_PlaybackData& playbackData = sounds.at(i)->_getPlaybackData();

			unsigned int inFormatSize = playbackData.bytesPerSample;
			int formatFactor = playbackData.formatFactor;

			int inChannelCount = playbackData.channelCount;

			unsigned int mask = 0;
			bool isSigned = true;
			switch (inFormatSize)
			{
			case 1:
				//isSigned = false;
				//mask = 0x000000FF;
				//break; 
				sounds.at(i)->_setIsPlaying();
				continue; // Skip to the next sound, currently there's no support for this
			case 2:
				mask = 0x0000FFFF;
				break;
			case 3:
				mask = 0x00FFFFFF;
				break;
			case 4:
				mask = 0xFFFFFFFF;
				break;
			default:
				break;
			}

			unsigned int bytesLeft = playbackData.loopCount > 0 ? UINT_MAX : playbackData.dataCount - playbackData.currentByte;
			if (bytesLeft == 0)
				continue;
			
			// Checks if the amount of data left in the raw sound is enough to fill the buffer
			// byteWrite is the amount of samples it will write into the outBuffer
			bool fillsBuffer = nBufferFrames * channelCount < bytesLeft / inFormatSize * channelCount / inChannelCount;
			unsigned int sampleWrite = fillsBuffer ? nBufferFrames * channelCount : bytesLeft / inFormatSize * channelCount / inChannelCount;

			float volume = sounds.at(i)->_getVolume() * volumeDecrement * getMasterVolume();
			float panning = fmaxf(fminf(sounds.at(i)->_getPanning(), 1.0f), -1.0f);

			for (int sample = 0; sample < sampleWrite / channelCount; sample++)
			{

				_ChannelQuality channelOutputs[channelCount];

				// Calculate the per channel data
				for (int channel = 0; channel < channelCount; channel++)
				{
					// Get the amount of bytes to go through the data for this sample
					unsigned int sampleOffset = playbackData.currentByte + (sample * inChannelCount + (inChannelCount == 2 ? channel : 0)) * inFormatSize;

					// Loop sound read offset
					if (playbackData.loopCount > 0 && sampleOffset > playbackData.dataCount)
					{
						playbackData.currentByte -= playbackData.dataCount;
						sampleOffset = sampleOffset % playbackData.dataCount;
						if (playbackData.loopCount != UINT_MAX)
							playbackData.loopCount--;
					}

					// The value of the sample cast to an integer, doesn't scale to fit range
					int outSample = (*(int*)(playbackData.data + sampleOffset) & mask);

					// Scale output and store it for pan processing, apply volume here
					if (formatFactor >= 0)
						channelOutputs[channel] = (_ChannelQuality)(outSample >> (formatFactor * 8)) * volume;
					else
						channelOutputs[channel] = (_ChannelQuality)(outSample << (-formatFactor * 8)) * volume;
				}

				// This is the amount to merge the right channel into the left channel
				float leftPanAmount  = -fminf(panning, 0.0f) * PI / 2.0f;
				float rightPanAmount =  fmaxf(panning, 0.0f) * PI / 2.0f;

				// Apply pan
				outData[sample * channelCount + 0] += channelOutputs[0] * fmaxf(sinf(leftPanAmount),  0.0f);
				outData[sample * channelCount + 1] += channelOutputs[0] * fmaxf(cosf(leftPanAmount),  0.0f);
				outData[sample * channelCount + 1] += channelOutputs[1] * fmaxf(sinf(rightPanAmount), 0.0f);
				outData[sample * channelCount + 0] += channelOutputs[1] * fmaxf(cosf(rightPanAmount), 0.0f);
			}

			// Seek to the next data we need to read, if it's the end of the data and we've finished looping, stop the sound
			if (playbackData.currentByte + sampleWrite * inFormatSize / channelCount * inChannelCount < playbackData.dataCount - 1)
				playbackData.currentByte += sampleWrite * inFormatSize / channelCount * inChannelCount;
			else if (playbackData.loopCount == 0)
			{
				playbackData.currentByte = playbackData.dataCount;
				sounds.at(i)->_setIsPlaying();
			}
		}

		// [----------------------]
		//   Update Sound Streams
		// [----------------------]

		std::vector<_SoundStream*> streams = inData->activeStreams.read();
		for (unsigned int i = 0; i < streams.size(); i++)
		{
			_SoundStream& stream = *streams.at(i);
			const _SoundInfo& streamInfo = stream._getSoundInfo();

			unsigned int inFormatSize = streamInfo.bitsPerSample / 8;
			int formatFactor = stream._getFormatFactor();

			int inChannelCount = stream._getSoundInfo().channelCount;

			unsigned int mask = 0;
			bool isSigned = true;
			switch (inFormatSize)
			{
			case 1:
				//isSigned = false;
				//mask = 0x000000FF;
				//break; 
				stream._setIsPlaying(false);
				continue; // Skip to the next sound, currently there's no support for this
			case 2:
				mask = 0x0000FFFF;
				break;
			case 3:
				mask = 0x00FFFFFF;
				break;
			case 4:
				mask = 0xFFFFFFFF;
				break;
			default:
				break;
			}

			// Figure out how many samples to write (samples * channels)
			unsigned int bytesLeft = stream._getBytesLeftToPlay();
			unsigned int sampleWrite = nBufferFrames * channelCount < bytesLeft / inFormatSize ? nBufferFrames * channelCount : bytesLeft / inFormatSize;

			// Get data in const char* form
			const char* data = stream._getNextDataBlock();

			// Get volume and panning info
			float volume = stream._getVolume();
			float panning = fmaxf(fminf(stream._getPanning(), 1.0f), -1.0f);

			for (int sample = 0; sample < sampleWrite / channelCount; sample++)
			{
				_ChannelQuality channelOutputs[channelCount];

				// Get sample data
				for (int channel = 0; channel < channelCount; channel++)
				{
					// Get the amount of bytes to go through the data for this sample
					unsigned int sampleOffset = (sample * inChannelCount + (inChannelCount == 2 ? channel : 0)) * inFormatSize;

					// We don't need to do loop processing here as it is done by _getNextDataBlock()

					// The value of the sample cast to an integer, doesn't scale to fit range
					int outSample = (*(int*)(data + sampleOffset) & mask);

					// Scale output and store it for pan processing, apply volume here
					if (formatFactor >= 0)
						channelOutputs[channel] = (_ChannelQuality)(outSample >> (formatFactor * 8)) * volume;
					else
						channelOutputs[channel] = (_ChannelQuality)(outSample << (-formatFactor * 8)) * volume;
				}

				// This is the amount to merge the right channel into the left channel
				float leftPanAmount = -fminf(panning, 0.0f) * PI / 2.0f;
				float rightPanAmount = fmaxf(panning, 0.0f) * PI / 2.0f;

				// Apply pan
				outData[sample * channelCount + 0] += channelOutputs[0] * fmaxf(sinf(leftPanAmount), 0.0f);
				outData[sample * channelCount + 1] += channelOutputs[0] * fmaxf(cosf(leftPanAmount), 0.0f);
				outData[sample * channelCount + 1] += channelOutputs[1] * fmaxf(sinf(rightPanAmount), 0.0f);
				outData[sample * channelCount + 0] += channelOutputs[1] * fmaxf(cosf(rightPanAmount), 0.0f);
			}

			// If it's the end of the data and we've finished looping, stop the sound
			if (sampleWrite == bytesLeft / inFormatSize)
				stream._setIsPlaying(false);
		}
		return 0;
	}

	class AudioHandler
	{
	public:
		AudioHandler()
			: m_SamplerPassInInfo{ { {} }, { {} } }
		{

		}

		void init(RtAudioFormat format = RTAUDIO_SINT16)
		{
			std::vector<unsigned int> deviceIds = m_Dac.getDeviceIds();

			debugLogIf(deviceIds.size() < 1, "No audio devices found!", LOST_LOG_ERROR);

			m_OutputParameters.deviceId = m_Dac.getDefaultOutputDevice();
			m_OutputParameters.nChannels = 2;
			m_OutputParameters.firstChannel = 0;

			m_CurrentDeviceName = m_Dac.getDeviceInfo(m_OutputParameters.deviceId).name;

			m_Format = format;

			m_Dac.openStream(&m_OutputParameters, NULL, format, 44100, &m_BufferFrames, &playRaw, (void*)&m_SamplerPassInInfo);
			m_Dac.startStream();
			
			debugLog("Successfully initialized audio on device: " + m_CurrentDeviceName, LOST_LOG_SUCCESS);
		}

		void exit()
		{
			if (m_Dac.isStreamOpen()) m_Dac.closeStream();

			const std::vector<PlaybackSound*>& list1 = m_SamplerPassInInfo.activeSounds.read();
			for (PlaybackSound* sound : list1)
				delete sound;
		}

		unsigned int getBufferFrameCount() const { return m_BufferFrames; };
		RtAudio& getDac() { return m_Dac; }
		RtAudio::StreamParameters& getOutputStreamParams() { return m_OutputParameters; }
		RtAudioFormat getAudioFormat() const { return m_Format; };
		
		// Loopcount - when UINT_MAX / -1 - will cause the sound to loop forever, only stopped by stopSound
		PlaybackSound* playSound(_Sound* sound, float volume, float panning, unsigned int loopCount) // [!] TODO: Add Volume and Pan
		{
			std::mutex& soundMutex = m_SamplerPassInInfo.activeSounds.getMutex();
			PlaybackSound* pbSound = new PlaybackSound(sound, volume, panning, loopCount);

			soundMutex.lock();
			m_SamplerPassInInfo.activeSounds.getWriteRef().push_back(pbSound);
			m_SamplerPassInInfo.activeSounds.forceDirty();
			soundMutex.unlock();

			return pbSound;
		}

		void stopSound(const PlaybackSound* sound)
		{
			std::mutex& soundMutex = m_SamplerPassInInfo.activeSounds.getMutex();

			unsigned int location = -1;
			PlaybackSound* ref = nullptr;
			std::vector<PlaybackSound*>& writeRef = m_SamplerPassInInfo.activeSounds.getWriteRef();
			for (int i = 0; i < writeRef.size(); i++)
			{
				if (writeRef.at(i) == sound)
				{
					location = i;
					ref = writeRef.at(i);
					break;
				}
			}

			if (ref)
			{
				m_GarbageSounds.push_back({ 0.0, ref });
				soundMutex.lock();
				writeRef.erase(writeRef.begin() + location);
				m_SamplerPassInInfo.activeSounds.forceDirty();
				soundMutex.unlock();
			}
			else
			{
				debugLog("Tried to stop a sound that had already finished playing or doesn't exist", LOST_LOG_WARNING_NO_NOTE);
			}
		}

		void stopSounds(const _Sound* sound)
		{
			std::mutex& soundMutex = m_SamplerPassInInfo.activeSounds.getMutex();

			unsigned int location = -1;
			PlaybackSound* ref = nullptr;
			std::vector<PlaybackSound*>& writeRef = m_SamplerPassInInfo.activeSounds.getWriteRef();
			for (int i = writeRef.size() - 1; i >= 0; i--)
			{
				if (writeRef.at(i)->getParentSound() == sound)
				{
					location = i;
					ref = writeRef.at(i);

					m_GarbageSounds.push_back({ 0.0, ref });
					soundMutex.lock();
					writeRef.erase(writeRef.begin() + location);
					m_SamplerPassInInfo.activeSounds.forceDirty();
					soundMutex.unlock();
				}
			}
		}

		void endSounds(float deltaTime)
		{
			std::mutex& soundMutex = m_SamplerPassInInfo.activeSounds.getMutex();
			std::vector<PlaybackSound*>& writeRef = m_SamplerPassInInfo.activeSounds.getWriteRef();
			for (int i = writeRef.size() - 1; i >= 0; i--)
			{
				if (!writeRef.at(i)->isPlaying())
				{
					m_GarbageSounds.push_back({ 0.0, writeRef.at(i) });
					soundMutex.lock();
					writeRef.erase(writeRef.begin() + i);
					m_SamplerPassInInfo.activeSounds.forceDirty();
					soundMutex.unlock();
				}
			}

			for (int i = m_GarbageSounds.size() - 1; i >= 0; i--)
			{
				m_GarbageSounds.at(i).first += deltaTime;
				if (m_GarbageSounds.at(i).first >= m_CullTime)
				{
					delete m_GarbageSounds.at(i).second;
					m_GarbageSounds.erase(m_GarbageSounds.begin() + i);
				}
			}
		}

		bool hasSound(const PlaybackSound* sound)
		{
			std::vector<PlaybackSound*>& writeRef = m_SamplerPassInInfo.activeSounds.getWriteRef();
			for (int i = 0; i < writeRef.size(); i++)
			{
				if (writeRef.at(i) == sound)
					return true;
			}
			return false;
		}

		bool hasSoundStream(const SoundStream sound)
		{
			std::vector<_SoundStream*>& writeRef = m_SamplerPassInInfo.activeStreams.getWriteRef();
			for (int i = 0; i < writeRef.size(); i++)
			{
				if (writeRef.at(i) == sound)
					return true;
			}
			return false;
		}

		void playSoundStream(_SoundStream* soundStream, float volume, float panning, unsigned int loopCount)
		{
			std::mutex& streamMutex = m_SamplerPassInInfo.activeStreams.getMutex();

			soundStream->_prepareStartPlay(volume, panning, loopCount);
			soundStream->_setActive(true);
			soundStream->_setIsPlaying(true);

			streamMutex.lock();
			m_SamplerPassInInfo.activeStreams.getWriteRef().push_back(soundStream);
			m_SamplerPassInInfo.activeStreams.forceDirty();
			streamMutex.unlock();
		}

		void stopSoundStream(_SoundStream* soundStream)
		{
			std::mutex& streamMutex = m_SamplerPassInInfo.activeStreams.getMutex();

			unsigned int location = -1;
			_SoundStream* ref = nullptr;
			std::vector<_SoundStream*>& writeRef = m_SamplerPassInInfo.activeStreams.getWriteRef();
			for (int i = 0; i < writeRef.size(); i++)
			{
				if (writeRef.at(i) == soundStream)
				{
					location = i;
					ref = writeRef.at(i);
					break;
				}
			}

			if (ref)
			{
				m_GarbageStreams.push_back({ 0.0, ref });
				streamMutex.lock();
				writeRef.erase(writeRef.begin() + location);
				m_SamplerPassInInfo.activeStreams.forceDirty();
				streamMutex.unlock();
			}
			else
			{
				debugLog("Tried to stop a sound stream that had already finished playing or doesn't exist", LOST_LOG_WARNING_NO_NOTE);
			}
		}

		void endSoundStreams(float deltaTime)
		{
			std::mutex& streamMutex = m_SamplerPassInInfo.activeStreams.getMutex();
			std::vector<_SoundStream*>& writeRef = m_SamplerPassInInfo.activeStreams.getWriteRef();
			for (int i = writeRef.size() - 1; i >= 0; i--)
			{
				if (!writeRef.at(i)->isPlaying())
				{
					m_GarbageStreams.push_back({ 0.0, writeRef.at(i) });
					streamMutex.lock();
					writeRef.erase(writeRef.begin() + i);
					m_SamplerPassInInfo.activeStreams.forceDirty();
					streamMutex.unlock();
				}
			}

			for (int i = m_GarbageStreams.size() - 1; i >= 0; i--)
			{
				m_GarbageStreams.at(i).first += deltaTime;
				if (m_GarbageStreams.at(i).first >= m_CullTime)
				{
					m_GarbageStreams.at(i).second->_setActive(false);
					m_GarbageStreams.erase(m_GarbageStreams.begin() + i);
				}
			}
		}

		void update(float deltaTime)
		{
			endSounds(deltaTime);
			endSoundStreams(deltaTime);
		}

		void setMasterVolume(float volume)
		{
			a_MasterVolume.write(volume);
		}

		float getMasterVolume()
		{
			return a_MasterVolume.read();
		}

	private:
		RtAudio m_Dac;
		RtAudio::StreamParameters m_OutputParameters;
		std::string m_CurrentDeviceName;

		SamplerPassInInfo m_SamplerPassInInfo;

		// Audio stream settings
		RtAudioFormat m_Format;
		unsigned int m_BufferFrames = 1024;

		_HaltWrite<float> a_MasterVolume = 1.0f;

		// lifetime + playbackSound*
		std::vector<std::pair<double, PlaybackSound*>> m_GarbageSounds;
		std::vector<std::pair<double, _SoundStream*>> m_GarbageStreams;
		double m_CullTime = 100.0;
	};

	AudioHandler _audioHandler;

	PlaybackSound::PlaybackSound(_Sound* soundPlaying, float volume, float panning, unsigned int loopCount)
		: a_Playing{ true }
		, a_Volume{ volume }
		, a_Panning{ fmaxf(fminf(panning, 1.0f), -1.0f) }
	{
		// Bytes per channel's samples
		unsigned int soundFormat = soundPlaying->_getSoundInfo().format;
		// The bit selected is the amount of bytes per channel sample
		unsigned int audioFormat = _audioHandler.getAudioFormat();

		m_FormatFactor = soundFormat - (log2(audioFormat) + 1);

		a_PlaybackData.currentByte = 0;
		a_PlaybackData.dataCount = soundPlaying->getDataSize();
		a_PlaybackData.bytesPerSample = soundPlaying->_getSoundInfo().sampleSize / soundPlaying->_getSoundInfo().channelCount;
		a_PlaybackData.data = soundPlaying->getData();
		a_PlaybackData.formatFactor = m_FormatFactor;
		a_PlaybackData.format = soundPlaying->_getSoundInfo().format;
		a_PlaybackData.channelCount = soundPlaying->_getSoundInfo().channelCount;
		a_PlaybackData.loopCount = loopCount;

		m_ParentSound = soundPlaying;
	}

	void _initAudio()
	{
		_audioHandler.init();
		_initAudioRMs();
	}

	void _exitAudio()
	{
		_audioHandler.exit();
		_destroyAudioRMs();
	}

	void _updateAudio()
	{
		_audioHandler.update(getDeltaTime());
	}

	unsigned int _getAudioHandlerFormat()
	{
		return _audioHandler.getAudioFormat();
	}

	unsigned int _getAudioHandlerBufferSize()
	{
		return _audioHandler.getBufferFrameCount();
	}

	void setMasterVolume(float volume)
	{
		_audioHandler.setMasterVolume(fmaxf(volume, 0.0f));
	}

	float getMasterVolume()
	{
		return _audioHandler.getMasterVolume();
	}

	PlaybackSound* playSound(Sound sound, float volume, float panning, unsigned int loopCount)
	{
		if (sound->isFunctional())
			return _audioHandler.playSound(sound, volume, panning, loopCount);
		return nullptr;
	}

	void stopSound(const PlaybackSound* sound)
	{
		return _audioHandler.stopSound(sound);
	}

	void stopSound(Sound sound)
	{
		return _audioHandler.stopSounds(sound);
	}

	void setSoundVolume(PlaybackSound* sound, float volume)
	{
		sound->_setVolume(volume);
	}

	void setSoundPanning(PlaybackSound* sound, float panning)
	{
		sound->_setPanning(panning);
	}

	bool isSoundPlaying(PlaybackSound* sound)
	{
		if (!_audioHandler.hasSound(sound))
			return false;
		return sound->isPlaying();
	}

	void playSoundStream(SoundStream soundStream, float volume, float panning, unsigned int loopCount)
	{
		// Check if it's already being played
		if (!soundStream->getActive() && soundStream->isFunctional())
		{
			_audioHandler.playSoundStream(soundStream, volume, panning, loopCount);
		}
	}

	void stopSoundStream(SoundStream soundStream)
	{
		_audioHandler.stopSoundStream(soundStream);
	}

	bool isSoundStreamPlaying(SoundStream sound)
	{
		if (!_audioHandler.hasSoundStream(sound))
			return false;
		return sound->isPlaying();
	}
	void setSoundStreamVolume(SoundStream sound, float volume)
	{
		sound->_setVolume(volume);
	}

	void setSoundStreamPanning(SoundStream sound, float panning)
	{
		sound->_setPanning(panning);
	}
}