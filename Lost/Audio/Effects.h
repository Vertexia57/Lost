#pragma once

namespace lost
{

	class Effect
	{
	public:
		Effect();

		// The function ran for every sample of audio 
		virtual float tick(float input, float sampleDelta) = 0;
	private:
	};

	struct LowPassFilter : public Effect
	{
	public:
		LowPassFilter(float cutOffFrequency, float sampleDelta);

		// The function ran for every sample of audio 
		virtual float tick(float input, float sampleDelta);

		// Sets the cut-off frequency of the low pass filter
		void setCutOffFrequency(float cutOffFrequency);
	private:
		float m_CutOffFrequency;
		double m_LastVal;
		double m_LastSampleDelta;
		double m_ePow;
	};

}