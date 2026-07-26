/*
 ==============================================================================

 This file is part of the audio-utils library.
 Copyright (C) the audio-utils developers.

 See LICENSE.txt for more info.

 ==============================================================================
*/

#pragma once

#include "smoothed_float.h"
#include "utils.h"
#include <cmath>

namespace audio_utils
{

    class SmoothedOscillator
    {
    public:
        SmoothedOscillator(const double sample_rate,
                           const double frequency,
                           const double phase,
                           const double gain,
                           const double smoothing_time_ms);
        ~SmoothedOscillator() = default;

        void init(const double sample_rate, const double smoothing_time_ms);
        void set_frequency(const double frequency) { frequency_.set_target_value(frequency); };
        void set_gain(const double gain) { gain_.set_target_value(gain); };
        double get_gain() { return gain_.get_current_value(); };

        double process_sample();
        void process_block(double *const data, const int block_size);

#ifdef INCLUDE_PYTHON_BINDING
        pybind11::array_t<double> python_process_block(const int block_size)
        {
            auto result = pybind11::array_t<double>(block_size);
            pybind11::buffer_info buffer_info = result.request();
            double *output_data = static_cast<double *>(buffer_info.ptr);
            process_block(output_data, block_size);
            return result;
        };
#endif
    private:
        void calculate_phase_increment(const double frequency);

        double sample_rate_{48000.};
        double phase_increment_{0.};
        double phase_{0.};

        SmoothedFloat<double> gain_;
        SmoothedFloat<double> frequency_;
    };

    SmoothedOscillator::SmoothedOscillator(const double sample_rate,
                                           const double frequency,
                                           const double phase,
                                           const double gain,
                                           const double smoothing_time_ms)
    {
        init(sample_rate, smoothing_time_ms);
        set_frequency(frequency);
        set_gain(gain);
        phase_ = phase;
    }

    inline void SmoothedOscillator::init(const double sample_rate, const double smoothing_time_ms)
    {
        sample_rate_ = sample_rate;
        gain_.init(sample_rate);
        frequency_.init(sample_rate);
        gain_.set_smoothing_time(smoothing_time_ms);
        frequency_.set_smoothing_time(smoothing_time_ms);
    }

    inline double SmoothedOscillator::process_sample()
    {
        const double sample = std::sin(phase_) * gain_.get_next_value();
        calculate_phase_increment(frequency_.get_next_value());
        phase_ += phase_increment_;
        while (phase_ >= two_pi<double>())
        {
            phase_ -= two_pi<double>();
        }
        return sample;
    }

    inline void SmoothedOscillator::process_block(double *const data, const int block_size)
    {
        for (int i = 0; i < block_size; i++)
        {
            data[i] = std::sin(phase_) * gain_.get_next_value();
            calculate_phase_increment(frequency_.get_next_value());
            phase_ += phase_increment_;
            while (phase_ >= two_pi<double>())
            {
                phase_ -= two_pi<double>();
            }
        }
    }

    inline void SmoothedOscillator::calculate_phase_increment(const double frequency)
    {
        phase_increment_ = frequency * two_pi<double>() / sample_rate_;
    }

}
