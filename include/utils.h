/*
 ==============================================================================

 This file is part of the audio-utils library.
 Copyright (C) the audio-utils developers.

 See LICENSE.txt for more info.

 ==============================================================================
*/

#pragma once

namespace audio_utils
{

    template <typename FloatType>
    static inline FloatType pi()
    {
        return static_cast<FloatType>(3.14159265358979323846);
    }

    template <typename FloatType>
    static inline FloatType two_pi()
    {
        return static_cast<FloatType>(6.283185307179586476925286766);
    }

    template <typename T>
    static inline T clip(const T input, const T lower_bound, const T upper_bound)
    {
        T clipped = input > upper_bound ? upper_bound : input;
        clipped = clipped < lower_bound ? lower_bound : clipped;
        return clipped;
    }

    template <typename T>
    static inline T clip_delete(const T input, const T lower_bound, const T upper_bound)
    {
        T clipped = input > upper_bound ? static_cast<T>(-1) : input;
        clipped = clipped < lower_bound ? static_cast<T>(-1) : clipped;
        return clipped;
    }

}
