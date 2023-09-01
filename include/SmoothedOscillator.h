/*
 ==============================================================================

 This file is part of the audio-utils library.
 Copyright (C) the audio-utils developers.

 See LICENSE.txt for  more info.

 ==============================================================================
*/

#pragma once

#include "SmoothedFloat.h"
#include "Utils.h"
#include <cmath>

namespace audio_utils
{

    class SmoothedOscillator
    {
    public:
        SmoothedOscillator(const double samplerate, const double freq, const double phase, const double gain, const double smoothingTimeMs);
        ~SmoothedOscillator() = default;

        void init(const double samplerate, const double smoothingTimeMs);
        void setFrequency(const double freq) { mFrequency.setTargetValue(freq); };
        void setGain(const double gain) { mGain.setTargetValue(gain); };
        double getGain() { return mGain.getCurrentValue(); };

        double processSample();
        void processBlock(double *const data, const int blockSize);

#ifdef INCLUDE_PYTHON_BINDING
        pybind11::array_t<double> Python_processBlock(const int blockSize)
        {
            auto result = pybind11::array_t<double>(blockSize);
            pybind11::buffer_info buf = result.request();
            double *ptr = static_cast<double *>(buf.ptr);
            processBlock(ptr, blockSize);
            return result;
        };
#endif
    private:
        void calcPhaseIncr(const double freq);

        double mSamplerate{48000.};
        double mPhaseIncr{0.};
        double mPhase{0.};

        SmoothedFloat<double> mGain;
        SmoothedFloat<double> mFrequency;
    };

    SmoothedOscillator::SmoothedOscillator(const double samplerate, const double freq, const double phase, const double gain, const double smoothingTimeMs)
    {
        init(samplerate, smoothingTimeMs);
        setFrequency(freq);
        setGain(gain);
        mPhase = phase;
    }

    inline void SmoothedOscillator::init(const double samplerate, const double smoothingTimeMs)
    {
        mSamplerate = samplerate;
        mGain.init(samplerate);
        mFrequency.init(samplerate);
        mGain.setSmoothingTime(smoothingTimeMs);
        mFrequency.setSmoothingTime(smoothingTimeMs);
    }

    inline double SmoothedOscillator::processSample()
    {
        const double sample = std::sin(mPhase) * mGain.getNextValue();
        calcPhaseIncr(mFrequency.getNextValue());
        mPhase += mPhaseIncr;
        while (mPhase >= TwoPi<double>())
        {
            mPhase -= TwoPi<double>();
        }
        return sample;
    }

    inline void SmoothedOscillator::processBlock(double *const data, const int blockSize)
    {
        for (int i = 0; i < blockSize; i++)
        {
            data[i] = std::sin(mPhase) * mGain.getNextValue();
            calcPhaseIncr(mFrequency.getNextValue());
            mPhase += mPhaseIncr;
            while (mPhase >= TwoPi<double>())
            {
                mPhase -= TwoPi<double>();
            }
        }
    }

    inline void SmoothedOscillator::calcPhaseIncr(const double freq)
    {
        mPhaseIncr = freq * TwoPi<double>() / mSamplerate;
    }

}