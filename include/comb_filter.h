/*
 ==============================================================================

 This file is part of the audio-utils library.
 Copyright (C) the audio-utils developers.

 See LICENSE.txt for more info.

 ==============================================================================
*/

#pragma once

#include <cmath>
#include <cstddef>

#include "circular_buffer.h"
#include "utils.h"

namespace audio_utils
{
    class CombFilter
    {
    public:
        CombFilter() { delay_line_.change_size(1000u); };
        ~CombFilter() = default;

        double process_sample(const double sample);
        void process_block(double *const data, const int block_size);

        void init_sample_rate(double sample_rate);
        void set_frequency(double frequency);
        void set_delay(double delay);
        void set_feedback_enabled(bool feedback_enabled) { feedback_enabled_ = feedback_enabled; };
        void set_feedback_intensity(double feedback_intensity);

    private:
        double pull_interpolated_sample();
        void ensure_buffer_size();

        CircularBuffer<double> delay_line_;
        double delay_{20.};
        double frequency_{100.};
        double sample_rate_{48000.};
        bool feedback_enabled_{false};
        double feedback_intensity_{0.};
    };

    inline double CombFilter::process_sample(const double sample)
    {
        double output = 0.;
        if (feedback_enabled_)
        {
            output = sample + feedback_intensity_ * pull_interpolated_sample();
            delay_line_.push_sample(output);
        }
        else
        {
            delay_line_.push_sample(sample);
            output = feedback_intensity_ * pull_interpolated_sample() + sample;
        }
        return output;
    }

    inline void CombFilter::process_block(double *const data, const int block_size)
    {
        if (feedback_enabled_)
        {
            for (int i_sample = 0; i_sample < block_size; i_sample++)
            {
                data[i_sample] = data[i_sample] + feedback_intensity_ * pull_interpolated_sample();
                delay_line_.push_sample(data[i_sample]);
            }
        }
        else
        {
            for (int i_sample = 0; i_sample < block_size; i_sample++)
            {
                delay_line_.push_sample(data[i_sample]);
                data[i_sample] = feedback_intensity_ * pull_interpolated_sample() + data[i_sample];
            }
        }
    }

    void CombFilter::init_sample_rate(double sample_rate)
    {
        sample_rate_ = sample_rate;
        set_delay(delay_);
    }

    void CombFilter::set_frequency(double frequency)
    {
        frequency_ = frequency;
        delay_ = sample_rate_ / frequency_;
        ensure_buffer_size();
    }

    void CombFilter::set_delay(double delay)
    {
        delay_ = delay;
        frequency_ = sample_rate_ / delay_;
        ensure_buffer_size();
    }

    void CombFilter::set_feedback_intensity(double feedback_intensity)
    {
        feedback_intensity_ = clip<double>(feedback_intensity, 0.0, 0.99999999);
    }

    inline double CombFilter::pull_interpolated_sample()
    {
        const double lower_delay = std::floor(delay_);
        const double upper_delay = lower_delay + 1.;
        const double fraction = delay_ - lower_delay;
        return delay_line_.pull_delay_sample(static_cast<int>(upper_delay)) * (fraction) +
               delay_line_.pull_delay_sample(static_cast<int>(lower_delay)) * (1. - fraction);
    }

    void CombFilter::ensure_buffer_size()
    {
        if ((static_cast<std::size_t>(delay_) + 1u) > delay_line_.get_buffer_size())
        {
            delay_line_.change_size(static_cast<std::size_t>(delay_) + 1u);
        }
    }

}
