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
#include <stdexcept>
#include <vector>

namespace audio_utils
{

    /*
     * A counted ring buffer that can represent every occupancy from empty to
     * completely full. Sequential pulls consume samples. Delay reads only
     * inspect the zero-initialized history and do not change the occupancy.
     *
     * Pushing into a full buffer overwrites its oldest sample. This policy
     * keeps delay-line users current; queue users can check
     * get_free_sample_count() before pushing when data loss is unacceptable.
     */
    template <typename T>
    class CircularBuffer
    {
    public:
        CircularBuffer(std::size_t buffer_size = 128) { change_size(buffer_size); };
        ~CircularBuffer() = default;
        void change_size(std::size_t buffer_size);
        std::size_t get_buffer_size() const noexcept { return buffer_size_; };
        std::size_t get_available_sample_count() const noexcept { return sample_count_; };
        std::size_t get_free_sample_count() const noexcept { return buffer_size_ - sample_count_; };
        bool is_empty() const noexcept { return sample_count_ == 0; };
        bool is_full() const noexcept { return sample_count_ == buffer_size_; };
        void clear() noexcept
        {
            read_position_ = write_position_;
            sample_count_ = 0;
        };

        void push_sample(const T value);
        void push_block(const T *const data, const int block_size);
        T pull_sample();
        void pull_block(T *const data, const int block_size);
        void pull_block_add(T *const data, const int block_size);
        T pull_delay_sample(const int delay);
        void pull_delay_block(T *const data, const int delay, const int block_size);
        void pull_delay_block_interpolating(T *const data, const double delay, const int block_size);
        void modulate_delay_block(const T *const data, const int delay, const int block_size);
        void modulate_delay_block_double(const double *const data, const int delay, const int block_size);
        inline std::size_t next_power_of_two(std::size_t size)
        {
            return std::pow(2, std::ceil(std::log(size) / std::log(2)));
        };

    protected:
        std::vector<T> buffer_;
        std::size_t buffer_size_{0};
        std::size_t buffer_size_minus_one_{0};
        std::size_t write_position_{0};
        std::size_t read_position_{0};
        std::size_t sample_count_{0};
    };

    template <typename T>
    inline void CircularBuffer<T>::change_size(std::size_t buffer_size)
    {
        if (buffer_size == 0)
        {
            throw std::invalid_argument("CircularBuffer size must be positive");
        }

        buffer_size_ = next_power_of_two(buffer_size);
        buffer_size_minus_one_ = buffer_size_ - 1;
        buffer_.assign(buffer_size_, static_cast<T>(0.));
        write_position_ = 0;
        read_position_ = 0;
        sample_count_ = 0;
    }

    template <typename T>
    inline void CircularBuffer<T>::push_sample(const T value)
    {
        write_position_ += 1;
        write_position_ = write_position_ & buffer_size_minus_one_;
        buffer_[write_position_] = value;

        if (is_full())
        {
            // Delay lines need the newest history, so a full buffer explicitly
            // discards its oldest sample.
            read_position_ += 1;
            read_position_ = read_position_ & buffer_size_minus_one_;
        }
        else
        {
            ++sample_count_;
        }
    }

    template <typename T>
    inline void CircularBuffer<T>::push_block(const T *const data, const int block_size)
    {
        if (block_size < 0 || (data == nullptr && block_size > 0))
        {
            throw std::invalid_argument("CircularBuffer input block is invalid");
        }

        for (int i = 0; i < block_size; i++)
        {
            push_sample(data[i]);
        }
    }

    template <typename T>
    inline T CircularBuffer<T>::pull_delay_sample(const int delay)
    {
        int position = (static_cast<int>(write_position_) - delay);
        if (position < 0)
        {
            position += buffer_size_;
        }
        return buffer_[position];
    }

    template <typename T>
    inline void CircularBuffer<T>::pull_delay_block(T *const data, const int delay, const int block_size)
    {
        int position = (static_cast<int>(write_position_) - delay);
        if (position < 0)
        {
            position += buffer_size_;
        }
        std::size_t read_position = static_cast<std::size_t>(position);
        for (int i = 0; i < block_size; i++)
        {
            data[i] = buffer_[read_position];
            read_position += 1;
            read_position = read_position & buffer_size_minus_one_;
        }
    }

    template <typename T>
    inline void
    CircularBuffer<T>::pull_delay_block_interpolating(T *const data, const double delay, const int block_size)
    {
        int lower_delay = static_cast<int>(std::floor(delay));
        int upper_delay = lower_delay + 1;

        int lower_position = (static_cast<int>(write_position_) - lower_delay);
        int upper_position = (static_cast<int>(write_position_) - upper_delay);
        if (lower_position < 0)
        {
            lower_position += buffer_size_;
        }
        if (upper_position < 0)
        {
            upper_position += buffer_size_;
        }

        const double fraction = delay - static_cast<double>(lower_delay);
        const double one_minus_fraction = (1. - fraction);

        std::size_t lower_read_position = static_cast<std::size_t>(lower_position);
        std::size_t upper_read_position = static_cast<std::size_t>(upper_position);
        for (int i = 0; i < block_size; i++)
        {
            const T lower_sample = buffer_[lower_read_position];
            const T upper_sample = buffer_[upper_read_position];

            data[i] = lower_sample * one_minus_fraction + upper_sample * fraction;

            lower_read_position += 1;
            lower_read_position = lower_read_position & buffer_size_minus_one_;
            upper_read_position += 1;
            upper_read_position = upper_read_position & buffer_size_minus_one_;
        }
    }

    template <typename T>
    inline void CircularBuffer<T>::modulate_delay_block(const T *const data, const int delay, const int block_size)
    {
        int position = (static_cast<int>(write_position_) - delay);
        if (position < 0)
        {
            position += buffer_size_;
        }
        std::size_t read_position = static_cast<std::size_t>(position);
        for (int i = 0; i < block_size; i++)
        {
            buffer_[read_position] *= data[i];
            read_position += 1;
            read_position = read_position & buffer_size_minus_one_;
        }
    }

    template <typename T>
    inline void
    CircularBuffer<T>::modulate_delay_block_double(const double *const data, const int delay, const int block_size)
    {
        int position = (static_cast<int>(write_position_) - delay);
        if (position < 0)
        {
            position += buffer_size_;
        }
        std::size_t read_position = static_cast<std::size_t>(position);
        for (int i = 0; i < block_size; i++)
        {
            buffer_[read_position] *= data[i];
            read_position += 1;
            read_position = read_position & buffer_size_minus_one_;
        }
    }

    template <typename T>
    inline T CircularBuffer<T>::pull_sample()
    {
        if (is_empty())
        {
            throw std::underflow_error("Cannot pull a sample from an empty CircularBuffer");
        }

        read_position_ += 1;
        read_position_ = read_position_ & buffer_size_minus_one_;
        --sample_count_;
        return buffer_[read_position_];
    }

    template <typename T>
    inline void CircularBuffer<T>::pull_block(T *const data, const int block_size)
    {
        if (block_size < 0 || (data == nullptr && block_size > 0))
        {
            throw std::invalid_argument("CircularBuffer output block is invalid");
        }
        if (static_cast<std::size_t>(block_size) > sample_count_)
        {
            throw std::underflow_error("CircularBuffer does not contain enough samples for the requested block");
        }

        for (int i = 0; i < block_size; i++)
        {
            read_position_ += 1;
            read_position_ = read_position_ & buffer_size_minus_one_;
            data[i] = buffer_[read_position_];
        }
        sample_count_ -= static_cast<std::size_t>(block_size);
    }

    template <typename T>
    inline void CircularBuffer<T>::pull_block_add(T *const data, const int block_size)
    {
        if (block_size < 0 || (data == nullptr && block_size > 0))
        {
            throw std::invalid_argument("CircularBuffer output block is invalid");
        }
        if (static_cast<std::size_t>(block_size) > sample_count_)
        {
            throw std::underflow_error("CircularBuffer does not contain enough samples for the requested block");
        }

        for (int i = 0; i < block_size; i++)
        {
            read_position_ += 1;
            read_position_ = read_position_ & buffer_size_minus_one_;
            data[i] += buffer_[read_position_];
        }
        sample_count_ -= static_cast<std::size_t>(block_size);
    }

}
