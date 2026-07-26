/*
 ==============================================================================

 This file is part of the audio-utils library.
 Copyright (C) the audio-utils developers.

 See LICENSE.txt for more info.

 ==============================================================================
*/

#pragma once

#include <complex>
#include <cstddef>

#include "utils.h"

using namespace std::complex_literals;

namespace audio_utils
{

    template <size_t WavetableSize>
    class StaticCplxWavetable
    {
    public:
        StaticCplxWavetable();
        ~StaticCplxWavetable() = default;
        inline const double *const get_sine_wavetable() { return sine_wavetable_; };
        inline const double *const get_cosine_wavetable() { return cosine_wavetable_; };

    private:
        double sine_wavetable_[WavetableSize];
        double cosine_wavetable_[WavetableSize];
    };

    template <size_t WavetableSize>
    StaticCplxWavetable<WavetableSize>::StaticCplxWavetable()
    {
        if (WavetableSize > 0)
        {
            const double phase_increment = audio_utils::two_pi<double>() / static_cast<double>(WavetableSize);
            double phase = 0.;
            for (size_t i = 0u; i < WavetableSize; i++)
            {
                sine_wavetable_[i] = std::sin(phase);
                cosine_wavetable_[i] = std::cos(phase);
                phase += phase_increment;
            }
        }
    }

    template <size_t WavetableSize>
    class CplxWavetableOscillator
    {
    public:
        CplxWavetableOscillator() = default;
        ~CplxWavetableOscillator() = default;
        void init(const double sample_rate, StaticCplxWavetable<WavetableSize> *static_wavetable);
        void set_frequency(const double frequency);

        std::complex<double> generate_sample();
        void generate_block(std::complex<double> *const data, const int block_size);

    private:
        void update_increment();
        std::complex<double> interpolate_wavetable();
        double frequency_{440.0};
        double phase_{0.};
        double sample_rate_{44100.};
        double phase_increment_{0.};
        StaticCplxWavetable<WavetableSize> *static_wavetable_;
        static constexpr std::size_t WAVETABLE_SIZE_MINUS_ONE{WavetableSize - 1};
    };

    template <size_t WavetableSize>
    inline void CplxWavetableOscillator<WavetableSize>::init(const double sample_rate,
                                                             StaticCplxWavetable<WavetableSize> *static_wavetable)
    {
        sample_rate_ = sample_rate;
        static_wavetable_ = static_wavetable;
        update_increment();
        phase_ = 0.;
    }

    template <size_t WavetableSize>
    inline void CplxWavetableOscillator<WavetableSize>::set_frequency(const double frequency)
    {
        frequency_ = frequency;
        update_increment();
    }

    template <size_t WavetableSize>
    inline void CplxWavetableOscillator<WavetableSize>::update_increment()
    {
        phase_increment_ = frequency_ * static_cast<double>(WavetableSize) / sample_rate_;
    }

    template <size_t WavetableSize>
    inline std::complex<double> CplxWavetableOscillator<WavetableSize>::generate_sample()
    {
        return interpolate_wavetable();
    }

    template <size_t WavetableSize>
    inline void CplxWavetableOscillator<WavetableSize>::generate_block(std::complex<double> *const data,
                                                                       const int block_size)
    {
        for (int i = 0; i < block_size; i++)
        {
            data[i] = interpolate_wavetable();
        }
    }

    template <size_t WavetableSize>
    inline std::complex<double> CplxWavetableOscillator<WavetableSize>::interpolate_wavetable()
    {
        // Linear interpolation
        const double lower_index_value = std::floor(phase_);
        const unsigned lower_index = static_cast<unsigned>(lower_index_value);
        const unsigned upper_index = lower_index + 1;
        const double fraction = phase_ - lower_index_value;
        const double one_minus_fraction = (1. - fraction);
        // Wrap around
        phase_ += phase_increment_;
        phase_ = static_cast<double>(static_cast<unsigned>(phase_) & WAVETABLE_SIZE_MINUS_ONE) + fraction;
        // Return
        const double *const cosine_wavetable = static_wavetable_->get_cosine_wavetable();
        const double *const sine_wavetable = static_wavetable_->get_sine_wavetable();
        const std::complex<double> value = {
            cosine_wavetable[lower_index] * one_minus_fraction + cosine_wavetable[upper_index] * fraction,
            -(sine_wavetable[lower_index] * one_minus_fraction + sine_wavetable[upper_index] * fraction)};
        return value;
    }

}
