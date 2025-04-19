#include "Effects.h"

#include <math.h>
#include "../GL/Vector.h"

namespace lost
{

	Effect::Effect() {}

	// This code is inspired by https://github.com/jimmyberg/LowPassFilter/tree/master
	// Credit to them isn't needed, but I felt it appropriate to do so
	// It is not directly from it. The only thing that is similar is the "tick" function and "ePow"

	LowPassFilter::LowPassFilter(float cutOffFrequency, float sampleDelta)
		: m_LastVal(0.0)
		, m_LastSampleDelta(sampleDelta)
		, m_ePow(1.0 - exp(-sampleDelta * 2.0 * PI * cutOffFrequency))
		, m_CutOffFrequency(cutOffFrequency)
	{
	}

	float LowPassFilter::tick(float input, float deltaTime)
	{
		return m_LastVal += (input - m_LastVal) * m_ePow;
	}

	void LowPassFilter::setCutOffFrequency(float cutOffFrequency)
	{
		m_CutOffFrequency = cutOffFrequency;
	}

}