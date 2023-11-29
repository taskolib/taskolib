/**
 * \file   Timeout.h
 * \author Lars Fröhlich
 * \date   Created on November 28, 2022
 * \brief  Declaration of the Timeout class.
 *
 * \copyright Copyright 2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the license, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef TASKOLIB_TIMEOUT_H_
#define TASKOLIB_TIMEOUT_H_

#include <chrono>
#include <cmath>
#include <limits>
#include <ostream>

#include "taskolib/exceptions.h"

namespace task {

/**
 * A type for storing a timeout duration.
 *
 * A timeout can be zero, positive, or infinite. A default-constructed Timeout is
 * infinite. The free functions isfinite() can be used to test for infinity:
 * \code
 * using namespace std::literals;
 *
 * auto a = Timeout{ 1min }; // 1 minute via std::chrono duration parameter
 * assert(isfinite(a));
 *
 * auto b = Timeout{ 60.0 }; // 60 seconds via floating-point parameter
 * assert(isfinite(b));
 *
 * auto c = Timeout{}; // default-constructed Timeout is infinite
 * assert(!isfinite(c));
 *
 * auto d = Timeout::infinity(); // static member function for generating infinite timeouts
 * assert(!isfinite(d));
 * \endcode
 *
 * For convenience, implicit conversion to floating-point types is supported, with an
 * output in seconds:
 * \code
 * double seconds = Timeout{ 2500ms };
 * assert(seconds == 2.5);
 *
 * float a = Timeout::infinity();
 * assert(std::isinf(a));
 * \endcode
 *
 * Internally, a Timeout is stored as a std::chrono::duration with sub-second precision.
 * The exact type used is made available as Timeout::Duration.
 */
class Timeout
{
public:
    using Duration = std::chrono::milliseconds;

    /// Default-construct an infinite timeout.
    Timeout() = default;

    /**
     * Construct a Timeout from a std::chrono::duration (implicit for convenience).
     *
     * \exception Error is thrown if the duration is negative.
     */
    template <typename Rep, typename Period>
    constexpr
    Timeout(std::chrono::duration<Rep, Period> duration)
    {
        if (duration < duration.zero())
            throw Error("Negative timeout");

        if (duration >= infinite_duration)
            timeout_ = infinite_duration;
        else
            timeout_ = std::chrono::round<Duration>(duration);
    }

    /**
     * Construct a Timeout from a floating-point value (only explicitly).
     *
     * \param seconds  The timeout in seconds
     *
     * \exception Error is thrown if seconds is negative or not-a-number.
     */
    template <typename FloatT,
              std::enable_if_t<std::is_floating_point<FloatT>::value, bool> = true>
    explicit constexpr
    Timeout(FloatT seconds)
    {
        if (seconds < FloatT{ 0.0 })
            throw Error("Negative timeout");

        if (std::isnan(seconds))
            throw Error("Timeout is not-a-number");

        const auto desired_duration = std::chrono::duration<FloatT>{ seconds };

        if (not std::isfinite(seconds) || desired_duration >= infinite_duration)
            timeout_ = infinite_duration;
        else
            timeout_ = std::chrono::round<Duration>(desired_duration);
    }

    /// A constant to use for "infinite" timeout durations
    static constexpr Timeout infinity() { return Timeout{ infinite_duration }; };

    /// Determine if the timeout has a finite duration.
    friend bool isfinite(Timeout timeout) noexcept
    {
        return timeout.timeout_ < infinite_duration;
    }

    /// Determine if two timeouts are equal.
    friend constexpr bool
    operator==(Timeout lhs, Timeout rhs) { return lhs.timeout_ == rhs.timeout_; }

    /// Determine if two timeouts are not equal.
    friend constexpr bool
    operator!=(Timeout lhs, Timeout rhs) { return lhs.timeout_ != rhs.timeout_; }

    /// Determine if the left timeout is longer than the right one.
    friend constexpr bool
    operator> (Timeout lhs, Timeout rhs) { return lhs.timeout_ >  rhs.timeout_; }

    /// Determine if the left timeout is shorter than the right one.
    friend constexpr bool
    operator< (Timeout lhs, Timeout rhs) { return lhs.timeout_ <  rhs.timeout_; }

    /// Determine if the left timeout is longer than or equal to the right one.
    friend constexpr bool
    operator>=(Timeout lhs, Timeout rhs) { return lhs.timeout_ >= rhs.timeout_; }

    /// Determine if the left timeout is shorter than or equal to the right one.
    friend constexpr bool
    operator<=(Timeout lhs, Timeout rhs) { return lhs.timeout_ <= rhs.timeout_; }

    /**
     * Implicitly cast the timeout to a floating-point type.
     * Infinity is returned for an infinite timeout.
     */
    template <typename FloatT,
              std::enable_if_t<std::is_floating_point<FloatT>::value, bool> = true>
    operator FloatT() const
    {
        if (isfinite(*this))
            return std::chrono::duration_cast<std::chrono::duration<FloatT>>(timeout_).count();
        else
            return std::numeric_limits<FloatT>::infinity();
    }

    /**
     * Explicitly cast the timeout to a std::chrono::duration type.
     * If the timeout is set to infinite, DurationT::max() is returned.
     */
    template <typename DurationT,
              std::enable_if_t<std::is_arithmetic_v<typename DurationT::rep>, bool> = true>
    operator DurationT() const
    {
        if (isfinite(*this))
            return std::chrono::duration_cast<DurationT>(timeout_);
        else
            return DurationT::max();
    }

private:
    static constexpr Duration infinite_duration{ Duration::max() };

    Duration timeout_{ infinite_duration };
};

inline std::ostream& operator<<(std::ostream& stream, const Timeout& timeout) {
    if (!isfinite(timeout))
        stream << "infinite";
    else
        stream << static_cast<std::chrono::milliseconds>(timeout).count();
    return stream;
}

} // namespace task

#endif
