/**
 * \file   TimeoutTrigger.h
 * \author Marcus Walla
 * \date   Created on February 16, 2023
 * \brief  Logic to check if a timeout elapsed.
 *
 * \copyright Copyright 2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOLIB_TIMEOUT_TRIGGER_H_
#define TASKOLIB_TIMEOUT_TRIGGER_H_

#include "taskolib/Timeout.h"
#include "taskolib/time_types.h"

namespace task {

/**
 * Evaluates when the clock is elapsed.
 *
 * \code {.cpp}
 * TimeoutTrigger trigger;
 * trigger.set_timeout(1s);
 * while(true)
 * {
 *     if(trigger.is_elapsed()) break;
 *     // do some stuff
 * }
 * \endcode
 *
 * \note This class is not thread-safe. To make it thread-safe you have to add a mutex to
 * synchronize resources with multiple threads.
 */
class TimeoutTrigger {
private:
    Timeout timeout_{Timeout::infinity()}; ///< Timeout.
    TimePoint start_{}; ///< Starting clock time used to measure the elapsed time.

public:
    /**
     * Reset the timeout start time to the current time. The timeout duration remains
     * unchanged.
     *
     * \return the new created start time for checking on elapsed timeout.
     */
    TimePoint reset() { start_ = Clock::now(); return start_; }

    /**
     * Get the timeout duration.
     *
     * \return constant timeout.
     */
    const Timeout get_timeout() const { return timeout_; }

    /**
     * Set the timeout duration.
     *
     * \param timeout to be measure with the is_elapsed() member function.
     */
    void set_timeout(Timeout timeout) { timeout_ = timeout; }

    /**
     * Get start time.
     *
     * \return start time of the clock.
     * \see reset()
     */
    TimePoint get_start_time() const { return start_; }

    /**
     * Evaluates if a timeout elapsed.
     *
     * \return true when the time elapsed.
     * \return false when still accurate and in the time interval.
     * \see reset()
     */
    bool is_elapsed() const
    {
        if (not isfinite(timeout_))
           return false;
        return Clock::now() - start_ > static_cast<Timeout::Duration>(timeout_);
    }
};

} // namespace task

#endif
