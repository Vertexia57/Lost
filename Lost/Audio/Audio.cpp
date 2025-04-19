#include "Audio.h"
#include "../Log.h"
#include "../DeltaTime.h"

#include "ResourceManagers/AudioResourceManagers.h"

#include <stdio.h>
#include <vector>

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
		int* outData = (int*)outputBuffer;

		unsigned int formatSize = sizeof(int);

		memset((char*)outputBuffer, 0, nBufferFrames * formatSize);

		std::vector<PlaybackSound*> sounds = inData->activeSounds.read();
		for (unsigned int i = 0; i < sounds.size(); i++)
		{
			_PlaybackData& playbackData = sounds.at(i)->_getPlaybackData();

			unsigned int dataLeft  = (playbackData.dataCount - playbackData.currentByte) / formatSize;
			unsigned int dataWrite = nBufferFrames < dataLeft ? nBufferFrames : dataLeft;

			if (i != 0)
			{
				for (unsigned int sample = 0; sample < dataWrite; sample++)
				{
					outData[sample] += *(int*)(playbackData.data + playbackData.currentByte + formatSize * sample);
				}
			}
			else
			{
				memcpy_s(outputBuffer, nBufferFrames * formatSize, playbackData.data + playbackData.currentByte, dataWrite * formatSize);
				if (dataWrite == nBufferFrames)
				{
					memset((char*)outputBuffer + dataWrite * formatSize, 0, (nBufferFrames - dataWrite) * formatSize);
				}
			}

			playbackData.currentByte += dataWrite * formatSize;

			if (dataWrite != nBufferFrames)
				sounds.at(i)->_setIsPlaying();
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

			m_Dac.openStream(&m_OutputParameters, NULL, RTAUDIO_SINT16, 44100, &m_BufferFrames, &playRaw, (void*)&m_SamplerPassInInfo);
			m_Dac.startStream();
			
			debugLog("Successfully initialized audio on device: " + m_CurrentDeviceName, LOST_LOG_SUCCESS);
		}

		void exit()
		{
			if (m_Dac.isStreamOpen()) m_Dac.closeStream();

			const std::vector<PlaybackSound*>& list1 = m_SamplerPassInInfo.activeSounds.read();
			for (PlaybackSound* sound : list1)
				delete sound;
			const std::vector<_SoundStream*>& list2 = m_SamplerPassInInfo.activeStreams.read();
			for (_SoundStream* sound : list2)
				delete sound;
		}

		RtAudio& getDac() { return m_Dac; }
		RtAudio::StreamParameters& getOutputStreamParams() { return m_OutputParameters; }
		RtAudioFormat getAudioFormat() const { return m_Format; };
		
		const PlaybackSound* playSound(const _Sound* sound) // [!] TODO: Add Volume and Pan
		{
			std::mutex& soundMutex = m_SamplerPassInInfo.activeSounds.getMutex();
			PlaybackSound* pbSound = new PlaybackSound(sound);

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

		void update(float deltaTime)
		{
			endSounds(deltaTime);
		}

	private:
		RtAudio m_Dac;
		RtAudio::StreamParameters m_OutputParameters;
		std::string m_CurrentDeviceName;

		SamplerPassInInfo m_SamplerPassInInfo;

		// Audio stream settings
		RtAudioFormat m_Format;
		unsigned int m_BufferFrames = 512;

		// lifetime + playbackSound*
		std::vector<std::pair<double, PlaybackSound*>> m_GarbageSounds;
		double m_CullTime = 100.0;
	};

	AudioHandler _audioHandler;

	PlaybackSound::PlaybackSound(const _Sound* soundPlaying)
		: a_Playing{ true }
	{
		m_FormatFactor = (int)sqrt(soundPlaying->_getSoundInfo().format) - (int)sqrt(_audioHandler.getAudioFormat());

		a_PlaybackData.currentByte = 0;
		a_PlaybackData.dataCount = soundPlaying->getDataSize();
		a_PlaybackData.bytesPerSample = soundPlaying->_getSoundInfo().sampleSize / soundPlaying->_getSoundInfo().channelCount;
		a_PlaybackData.data = soundPlaying->getData();
		a_PlaybackData.formatFactor = m_FormatFactor;
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

	const PlaybackSound* playSound(Sound sound)
	{
		return _audioHandler.playSound(sound);
	}

	void stopSound(const PlaybackSound* sound)
	{
		return _audioHandler.stopSound(sound);
	}
}