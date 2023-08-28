/**
 * \file   internals.h
 * \author Lars Fröhlich, Marcus Walla
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
#include <utility>

#include <gul14/string_view.h>

namespace task {

/// An enum detailing the possible causes of the termination of a sequence.
enum class ErrorCause { terminated_by_script, aborted, uncaught_error };

/// Define the Lua sequence filename for storing and loading Lua script.
const char sequence_lua_filename[] = "sequence.lua";

/**
 * A marker string (the word "ABORT" surrounded by Unicode stop signs) whose presence
 * anywhere in an error message signals that the execution of a script should be stopped.
 */
extern const gul14::string_view abort_marker;

/**
 * Throw an exception if the string contains control characters.
 *
 * \exception Error is thrown if the string contains any control characters.
 */
void check_for_control_characters(gul14::string_view str);

/**
 * Remove abort markers from the given error message, beautify it, and determine the cause
 * of the error.
 *
 * If at least two abort markers are present, the message is truncated to the text between
 * the first two of these markers. Otherwise, all markers are simply removed.
 *
 * The cause of the error is determined as follows:
 * - If no marker was present, the message describes a "normal" error and
 *   ErrorCause::uncaught_error is returned.
 * - If the message contains one or more abort markers, it describes an abort request for
 *   the sequence. There are two different types of abort requests:
 *   - If there is an error message, this is considered an abort request (either by the
 *     user or by timeouts) and ErrorCause::aborted is returned.
 *   - If the error message is empty, this is considered an explicit termination request
 *     by the running script. The error message is set to a corresponding explanation
 *     ("Script called terminate_sequence()") and ErrorCause::terminated_by_script is
 *     returned.
 *
 * If the message contains a Lua stack trace, the function highlights its start by
 * including a UTF-8 bullet point symbol to visually separate the main message text from
 * the stack trace.
 *
 * \returns a pair consisting of the (possibly) modified error message and of an
 *          ErrorCause.
 */
std::pair<std::string, ErrorCause> remove_abort_markers(gul14::string_view error_message);

} // namespace task

#endif
