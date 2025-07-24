#include "Effects.h"

#include <math.h>

namespace lost
{

	Effect::Effect() {}

	// This code is inspired by https://github.com/jimmyberg/LowPassFilter/tree/master
	// Credit to them isn't needed, but I felt it appropriate to do so
	// It is not directly from it. The only thing that is similar is the "tick" function and "ePow"

	LowPassFilter::LowPassFilter(float cutOffFrequency)
		: m_LastVal{ 0.0, 0.0 }
		, m_ePow(1.0 - exp(-(1.0f / (float)LOST_AUDIO_SAMPLE_RATE) * 2.0 * PI * cutOffFrequency))
		, m_CutOffFrequency(cutOffFrequency)
	{
	}

	void LowPassFilter::processChunk(AudioSample inBuffer[LOST_AUDIO_BUFFER_COUNT], AudioSample outBuffer[LOST_AUDIO_BUFFER_COUNT])
	{
		for (int i = 0; i < LOST_AUDIO_BUFFER_FRAMES; i++)
		{
			m_LastVal[0] += (audioSampleToFloat(inBuffer[i * 2]) - m_LastVal[0]) * m_ePow;
			m_LastVal[1] += (audioSampleToFloat(inBuffer[i * 2 + 1]) - m_LastVal[1]) * m_ePow;

			outBuffer[i * 2] = floatToAudioSample(m_LastVal[0]);
			outBuffer[i * 2 + 1] = floatToAudioSample(m_LastVal[1]);
		}
	}

	void LowPassFilter::setCutOffFrequency(float cutOffFrequency)
	{
		m_CutOffFrequency = cutOffFrequency;
		m_ePow = 1.0 - exp(-(1.0f / (float)LOST_AUDIO_SAMPLE_RATE) * 2.0 * PI * cutOffFrequency);
	}

	HighPassFilter::HighPassFilter(float cutOffFrequency)
		: m_CutOffFrequency(cutOffFrequency)
		, m_LastInVal{ 0.0f, 0.0f }
		, m_LastOutVal{ 0.0f, 0.0f }
	{
		m_CalculateCoefficients();
	}

	void HighPassFilter::processChunk(AudioSample inBuffer[LOST_AUDIO_BUFFER_COUNT], AudioSample outBuffer[LOST_AUDIO_BUFFER_COUNT])
	{
		for (int i = 0; i < LOST_AUDIO_BUFFER_FRAMES; i++)
		{
			float in = audioSampleToFloat(inBuffer[i * 2]);
			float output = m_Alpha * (m_LastOutVal[0] + in - m_LastInVal[0]);
			m_LastInVal[0] = in;
			m_LastOutVal[0] = output;
			outBuffer[i * 2] = floatToAudioSample(output);

			in = audioSampleToFloat(inBuffer[i * 2 + 1]);
			output = m_Alpha * (m_LastOutVal[1] + in - m_LastInVal[1]);
			m_LastInVal[1] = in;
			m_LastOutVal[1] = output;
			outBuffer[i * 2 + 1] = floatToAudioSample(output);
		}
	}

	void HighPassFilter::setCutOffFrequency(float cutOffFrequency)
	{
		m_CutOffFrequency = cutOffFrequency;
		m_CalculateCoefficients();
	}

	void HighPassFilter::m_CalculateCoefficients()
	{
		float RC = 1.0f / (2.0f * PI * m_CutOffFrequency);
		float dt = 1.0f / (float)LOST_AUDIO_SAMPLE_RATE;

		m_Alpha = RC / (RC + dt);
	}

	DelayEffect::DelayEffect(float delayTime)
		: m_DelayTime(delayTime)
		, m_Buffer(nullptr)
	{
		m_RecreateDelayBuffer();
	}

	DelayEffect::~DelayEffect()
	{
		delete[] m_Buffer;
	}

	void DelayEffect::processChunk(AudioSample inBuffer[LOST_AUDIO_BUFFER_COUNT], AudioSample outBuffer[LOST_AUDIO_BUFFER_COUNT])
	{
		for (int i = 0; i < LOST_AUDIO_BUFFER_COUNT; i++)
		{
			m_Buffer[m_WriteOffset] = inBuffer[i];
			outBuffer[i] = m_Buffer[m_ReadOffset] + inBuffer[i];

			m_ReadOffset++;
			if (m_ReadOffset > m_BufferSize)
				m_ReadOffset = 0;
			m_WriteOffset++;
			if (m_WriteOffset > m_BufferSize)
				m_WriteOffset = 0;
		}
	}

	void DelayEffect::setDelayTime(float delayTime)
	{
		m_DelayTime = delayTime;
	}

	void DelayEffect::m_RecreateDelayBuffer()
	{
		if (m_Buffer)
			delete[] m_Buffer;

		const unsigned int padding = 10;
		unsigned int sampleOffset = (unsigned int)ceil(m_DelayTime * (float)(LOST_AUDIO_SAMPLE_RATE * LOST_AUDIO_CHANNELS));
		unsigned int sampleCount = sampleOffset + padding;

		m_BufferSize = sampleCount;
		m_Buffer = new AudioSample[sampleCount];
		memset(m_Buffer, 0, sampleCount * sizeof(AudioSample));

		m_ReadOffset = 0;
		m_WriteOffset = sampleOffset;
	}

}