/**
 * \file   internals.h
 * \author Lars Froehlich, Marcus Walla
 * \date   Created on August 30, 2022
 * \brief  Declaration of internal constants and functions.
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

#ifndef TASKOLIB_INTERNALS_H_
#define TASKOLIB_INTERNALS_H_

#include <string>
#include <gul14/string_view.h>

namespace task {

/**
 * A marker string (the word "ABORT" surrounded by Unicode stop signs) whose presence
 * anywhere in an error message signals that the execution of a script should be stopped.
 */
extern const gul14::string_view abort_marker;

enum class ErrorCause { terminated_by_script, aborted, uncaught_error };

/**
 * Remove abort markers from the given error message and determine the cause of the error.
 *
 * All abort markers are removed from the given error message. If no marker was present,
 * the message describes a "normal" error and ErrorCause::uncaught_error is returned.
 *
 * If the message contains at least two abort markers and there is no message enclosed
 * between the first two of them, this is considered an explicit termination request by
 * the running script. The error message is set to a corresponding explanation ("Script
 * called terminate_sequence()") and ErrorCause::terminated_by_script is returned.
 *
 * If the error message contains at least one abort marker and an error message, it is
 * considered an abort request (either by the user or by timeouts) and ErrorCause::aborted
 * is returned. If there are at least two abort markers, the error message is set to the
 * text enclosed by the first two of them.
 */
ErrorCause remove_abort_markers_from_error_message(std::string& msg);

} // namespace task

#endif
