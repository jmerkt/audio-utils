/*
 ==============================================================================

 This file is part of the audio-utils library.
 Copyright (C) the audio-utils developers.

 See LICENSE.txt for  more info.

 ==============================================================================
*/

#pragma once

#include "CircularBuffer.h"
#include "Utils.h"

namespace audio_utils
{
	class CombFilter
	{
	public:
		CombFilter()
		{
			mDelayLine.changeSize(1000u);
		};
		~CombFilter() = default;

		double processSample(const double sample);
		void processBlock(double *const data, const int blockSize);

		void initFs(double fs);
		void setFrequency(double frequency);
		void setDelay(double delay);
		void setIsFeedback(bool isFeedback) { mIsFeedback = isFeedback; };
		void setFeedbackIntensity(double intensity);

	private:
		double pullSampleInterpolated();
		void checkBufferSize();

		CircularBuffer<double> mDelayLine;
		double mDelay{20.};
		double mFrequency{100.};
		double mFs{48000.};
		bool mIsFeedback{false};
		double mFeedbackIntensity{0.};
	};

	inline double CombFilter::processSample(const double sample)
	{
		double output = 0.;
		if (mIsFeedback)
		{
			output = sample + mFeedbackIntensity * pullSampleInterpolated();
			mDelayLine.pushSample(output);
		}
		else
		{
			mDelayLine.pushSample(sample);
			output = mFeedbackIntensity * pullSampleInterpolated() + sample;
		}
		return output;
	}

	inline void CombFilter::processBlock(double *const data, const int blockSize)
	{
		if (mIsFeedback)
		{
			for (int i_sample = 0; i_sample < blockSize; i_sample++)
			{
				data[i_sample] = data[i_sample] + mFeedbackIntensity * pullSampleInterpolated();
				mDelayLine.pushSample(data[i_sample]);
			}
		}
		else
		{
			for (int i_sample = 0; i_sample < blockSize; i_sample++)
			{
				mDelayLine.pushSample(data[i_sample]);
				data[i_sample] = mFeedbackIntensity * pullSampleInterpolated() + data[i_sample];
			}
		}
	}

	void CombFilter::initFs(double fs)
	{
		mFs = fs;
		setDelay(mDelay);
	}

	void CombFilter::setFrequency(double frequency)
	{
		mFrequency = frequency;
		mDelay = mFs / mFrequency;
		checkBufferSize();
	}

	void CombFilter::setDelay(double delay)
	{
		mDelay = delay;
		mFrequency = mFs / mDelay;
		checkBufferSize();
	}

	void CombFilter::setFeedbackIntensity(double intensity)
	{
		mFeedbackIntensity = Clip<double>(intensity, 0.0, 0.99999999);
	}

	inline double CombFilter::pullSampleInterpolated()
	{
		const double x_0 = floor(mDelay);
		const double x_1 = x_0 + 1.;
		const double x_diff = mDelay - x_0;
		return mDelayLine.pullDelaySample(static_cast<int>(x_1)) * (x_diff) + mDelayLine.pullDelaySample(static_cast<int>(x_0)) * (1. - x_diff);
	}

	void CombFilter::checkBufferSize()
	{
		if ((static_cast<size_t>(mDelay) + 1u) > mDelayLine.getBufferSize())
		{
			mDelayLine.changeSize(static_cast<size_t>(mDelay) + 1u);
		}
	}

}
