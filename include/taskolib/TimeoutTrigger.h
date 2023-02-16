/**
 * \file   TimeoutTrigger.h
 * \author Marcus Walla
 * \date   Created on February 16, 2023
 * \brief  A sequence of Steps.
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
 * Evaluates when a timeout is elapsed.
 *
 * \code {.cpp}
 * TimeoutTrigger trigger;
 * trigger.reset(100ms);
 * while(true)
 * {
 *     if(trigger.is_elapsed()) break;
 *     gul14::sleep(10ms);
 * }
 * \endcode
 */
class TimeoutTrigger {
private:
    Timeout timeout_{Timeout::infinity()}; ///< Timeout.
    TimePoint start_{Clock::now()}; ///< Starting clock time used to measure the elapsed time.

public:
    /**
     * When invoked resets the start time of the internal clock and sets the timeout that
     * is used in the is_elapsed() member function.
     *
     * \param timeout to be measure with the is_elapsed() member function.
     */
    void reset(Timeout timeout) { timeout_ = timeout; start_ = Clock::now(); }

    /**
     * Get the timeout object.
     *
     * \return constant timeout.
     */
    const Timeout get_timeout() const { return timeout_; }

    /**
     * Evaluates if the previous set timeout elapsed.
     *
     * \return true when the time elapsed.
     * \return false when still accurate and in the time interval.
     * \see reset(Timeout) for setting timeout.
     */
    bool is_elapsed() const
    {
        auto timeout_in_ms = static_cast<Timeout::Duration>(timeout_).count();
        return (Clock::now() - start_).count() > timeout_in_ms;
    }
};

} // namespace task

#endif
