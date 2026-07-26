/*
 ==============================================================================

 This file is part of the audio-utils library.
 Copyright (C) the audio-utils developers.

 See LICENSE.txt for more info.

 ==============================================================================
*/

#pragma once

#include <cmath>
#include <type_traits>

#include "utils.h"

namespace audio_utils
{

    template <typename FloatType>
    class SmoothedFloat
    {
    public:
        SmoothedFloat() = default;
        ~SmoothedFloat() = default;

        inline void init(FloatType sample_rate) noexcept
        {
            sample_rate_ = sample_rate;
            set_smoothing_time(smoothing_time_ms_);
        };
        inline void set_target_value(FloatType target_value) noexcept
        {
            target_value_ = target_value;
            countdown_ = smoothing_time_samples_;
            smoothing_step_ = (target_value_ - current_value_) * inverse_smoothing_time_samples_;
        };
        inline FloatType get_next_value() noexcept
        {
            if (!is_smoothing())
            {
                return target_value_;
            };
            --countdown_;
            if (is_smoothing())
            {
                current_value_ += smoothing_step_;
            }
            else
            {
                return target_value_;
            }
            return current_value_;
        };
        inline void get_next_block(FloatType *const data, const int block_size) noexcept
        {
            for (int i = 0; i < block_size; i++)
            {
                data[i] = get_next_value();
            }
        }
        inline bool is_smoothing() noexcept { return countdown_ > 0; };
        inline void set_smoothing_time(FloatType time_ms) noexcept
        {
            smoothing_time_ms_ = time_ms;
            smoothing_time_samples_ = static_cast<int>(smoothing_time_ms_ * one_div_thousand_ * sample_rate_);
            inverse_smoothing_time_samples_ =
                static_cast<FloatType>(1.) / static_cast<FloatType>(smoothing_time_samples_);
        };
        inline FloatType get_current_value() { return current_value_; };

#ifdef INCLUDE_PYTHON_BINDING
        pybind11::array_t<double> python_get_next_block(const int block_size)
        {
            auto result = pybind11::array_t<double>(block_size);
            pybind11::buffer_info buffer_info = result.request();
            double *output_data = static_cast<double *>(buffer_info.ptr);
            get_next_block(output_data, block_size);
            return result;
        };
#endif
    private:
        const FloatType one_div_thousand_ = static_cast<FloatType>(1. / 1000.);
        FloatType sample_rate_{48000.};
        int countdown_{0};
        int smoothing_time_samples_{0};
        FloatType inverse_smoothing_time_samples_{0.};
        FloatType target_value_{0.};
        FloatType current_value_{0.};
        FloatType smoothing_time_ms_{20.};
        FloatType smoothing_step_{0.};
    };

    namespace smoothing_types
    {
        struct Linear
        {
        };
        struct Exponential
        {
        };
    }

    template <typename FloatType, typename SmoothingType>
    class SmoothedFloatUpDown
    {
    public:
        SmoothedFloatUpDown() = default;
        ~SmoothedFloatUpDown() = default;

        inline void init(FloatType sample_rate) noexcept
        {
            sample_rate_ = sample_rate;
            set_smoothing_time(smoothing_time_ms_up_, smoothing_time_ms_down_);
        };
        inline void set_target_value(FloatType target_value) noexcept
        {
            if (target_value != target_value_)
            {
                if (target_value > current_value_)
                {
                    target_value_ = target_value;
                    countdown_ = smoothing_time_samples_up_;
                    if constexpr (std::is_same_v<SmoothingType, smoothing_types::Linear>)
                    {
                        smoothing_step_ = (target_value_ - current_value_) * inverse_smoothing_time_samples_up_;
                    }
                    else
                    {
                        smoothing_step_ =
                            std::exp((std::log(std::abs(target_value_)) - std::log(std::abs(current_value_))) *
                                     inverse_smoothing_time_samples_up_);
                    }
                }
                else
                {
                    target_value_ = target_value;
                    countdown_ = smoothing_time_samples_down_;
                    if constexpr (std::is_same_v<SmoothingType, smoothing_types::Linear>)
                    {
                        smoothing_step_ = (target_value_ - current_value_) * inverse_smoothing_time_samples_down_;
                    }
                    else
                    {
                        smoothing_step_ =
                            std::exp((std::log(std::abs(target_value_)) - std::log(std::abs(current_value_))) *
                                     inverse_smoothing_time_samples_down_);
                    }
                }
            }
        };
        inline FloatType get_next_value() noexcept
        {
            if (!is_smoothing())
            {
                return target_value_;
            };
            --countdown_;
            if (is_smoothing())
            {
                if constexpr (std::is_same_v<SmoothingType, smoothing_types::Linear>)
                {
                    current_value_ += smoothing_step_;
                }
                else
                {
                    current_value_ *= smoothing_step_;
                }
            }
            else
            {
                return target_value_;
            }
            return current_value_;
        };
        inline void get_next_block(FloatType *const data, const int block_size) noexcept
        {
            for (int i = 0; i < block_size; i++)
            {
                data[i] = get_next_value();
            }
        }
        inline bool is_smoothing() noexcept { return countdown_ > 0; };
        inline void set_smoothing_time(FloatType time_ms_up, FloatType time_ms_down) noexcept
        {
            smoothing_time_ms_up_ = time_ms_up;
            smoothing_time_samples_up_ = static_cast<int>(smoothing_time_ms_up_ * one_div_thousand_ * sample_rate_);
            inverse_smoothing_time_samples_up_ =
                static_cast<FloatType>(1.) / static_cast<FloatType>(smoothing_time_samples_up_);

            smoothing_time_ms_down_ = time_ms_down;
            smoothing_time_samples_down_ = static_cast<int>(smoothing_time_ms_down_ * one_div_thousand_ * sample_rate_);
            inverse_smoothing_time_samples_down_ =
                static_cast<FloatType>(1.) / static_cast<FloatType>(smoothing_time_samples_down_);
        };
        inline FloatType get_current_value() { return current_value_; };

    private:
        const FloatType one_div_thousand_ = static_cast<FloatType>(1. / 1000.);
        FloatType sample_rate_{48000.};
        int countdown_{0};
        FloatType target_value_{0.};
        FloatType current_value_{0.};
        FloatType smoothing_step_{0.};
        int smoothing_time_samples_up_{0};
        int smoothing_time_samples_down_{0};
        FloatType smoothing_time_ms_up_{20.};
        FloatType smoothing_time_ms_down_{20.};
        FloatType inverse_smoothing_time_samples_up_{0.};
        FloatType inverse_smoothing_time_samples_down_{0.};
    };

    template <typename FloatType>
    class OnePoleUpDown
    {
    public:
        OnePoleUpDown() = default;
        ~OnePoleUpDown() = default;

        inline void init(FloatType sample_rate) noexcept { sample_rate_ = sample_rate; };
        inline void set_target_value(FloatType target_value) noexcept
        {
            if (target_value > target_value_)
                smoothing_up_ = true;
            else
                smoothing_up_ = false;
            target_value_ = target_value;
        };
        inline FloatType get_next_value() noexcept
        {
            if (smoothing_up_)
            {
                current_value_ =
                    factor_up_ * target_value_ + (static_cast<FloatType>(1.) - factor_up_) * current_value_;
            }
            else
            {
                current_value_ =
                    factor_down_ * target_value_ + (static_cast<FloatType>(1.) - factor_down_) * current_value_;
            }
            return current_value_;
        };
        inline void get_next_block(FloatType *const data, const int block_size) noexcept
        {
            for (int i = 0; i < block_size; i++)
            {
                data[i] = get_next_value();
            }
        }
        inline void set_smoothing_factors(FloatType factor_up, FloatType factor_down) noexcept
        {
            factor_up_ = factor_up;
            factor_down_ = factor_down;
            factor_up_ = clip<FloatType>(factor_up_, 0., 0.9999999999);
            factor_down_ = clip<FloatType>(factor_down_, 0., 0.9999999999);
        };
        inline FloatType get_current_value() { return current_value_; };

    private:
        FloatType sample_rate_{48000.};
        FloatType target_value_{0.};
        FloatType current_value_{0.};
        FloatType factor_up_{0.};
        FloatType factor_down_{0.};
        bool smoothing_up_{true};
    };

}
