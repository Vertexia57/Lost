#pragma once
#include "Audio.h"

#ifndef PI
#define PI 3.141592653589
#endif // !PI

namespace lost
{

	class Effect
	{
	public:
		Effect();

		/// <summary>
		/// This function gets ran with the audio thread, audio goes in audio goes out.
		/// The format of the audio depends on the quality set when LostAudio was built, but by default it is int16_t.
		/// The format is also PCM, and will most likely need to be converted to and from a float.
		/// 
		/// The samples are INTERLACED going from left ear to right ear repeatedly.
		/// </summary>
		/// <param name="inBuffer">An array of AudioSamples of size LOST_AUDIO_BUFFER_COUNT, it is prefilled with data to process</param>
		/// <param name="outBuffer">An array of AudioSamples of size LOST_AUDIO_BUFFER_COUNT, it is empty but already innitialized, fill this with new data</param>
		virtual void processChunk(AudioSample inBuffer[LOST_AUDIO_BUFFER_COUNT], AudioSample outBuffer[LOST_AUDIO_BUFFER_COUNT]) = 0;
	private:
	};

	class LowPassFilter : public Effect
	{
	public:
		LowPassFilter(float cutOffFrequency);

		// The function ran for every sample of audio 
		virtual void processChunk(AudioSample inBuffer[LOST_AUDIO_BUFFER_COUNT], AudioSample outBuffer[LOST_AUDIO_BUFFER_COUNT]);

		// Sets the cut-off frequency of the low pass filter
		void setCutOffFrequency(float cutOffFrequency);
	private:
		float m_CutOffFrequency;
		double m_LastVal[2];
		double m_ePow;
	};

	class HighPassFilter : public Effect
	{
	public:
		HighPassFilter(float cutOffFrequency);

		// The function ran for every sample of audio 
		virtual void processChunk(AudioSample inBuffer[LOST_AUDIO_BUFFER_COUNT], AudioSample outBuffer[LOST_AUDIO_BUFFER_COUNT]);

		// Sets the cut-off frequency of the low pass filter
		void setCutOffFrequency(float cutOffFrequency);
	private:
		void m_CalculateCoefficients();

		float m_CutOffFrequency;
		double m_LastInVal[2];
		double m_LastOutVal[2];

		float m_Alpha;
	};

	class DelayEffect : public Effect
	{
	public:
		DelayEffect(float delayTime);
		~DelayEffect();

		// The function ran for every sample of audio 
		virtual void processChunk(AudioSample inBuffer[LOST_AUDIO_BUFFER_COUNT], AudioSample outBuffer[LOST_AUDIO_BUFFER_COUNT]);

		// Sets the cut-off frequency of the low pass filter
		void setDelayTime(float delayTime);
	private:
		void m_RecreateDelayBuffer();

		float m_DelayTime;
		AudioSample* m_Buffer;
		size_t m_BufferSize;

		size_t m_ReadOffset;
		size_t m_WriteOffset;
	};
}