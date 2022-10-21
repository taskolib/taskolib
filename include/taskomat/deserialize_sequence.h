/**
 * \file   deserialize_sequence.h
 * \author Marcus Walla
 * \date   Created on May 24, 2022
 * \brief  Deserialize Sequence and Steps from storage hardware.
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

#ifndef TASKOMAT_DESERIALIZE_H_
#define TASKOMAT_DESERIALIZE_H_

#include <filesystem>
#include <iostream>
#include "taskomat/Step.h"
#include "taskomat/Sequence.h"

namespace task {

/**
 * Deserialize parameters of Step from the input stream.
 *
 * No checking of any stream failure is done and should be performed by the caller.
 *
 * \param stream input stream
 * \param step Step to be deserialized.
 * \return passed input stream
 */
std::istream& operator>>(std::istream& stream, Step& step);

/**
 * Extracts and creates from \a path a Step and returns it. The file path must be a Lua
 * script and should have the extension 'lua'.
 *
 * It will throw an Error exception if an I/O error occurs on the external file path or
 * the file does not exist.
 *
 * To deserialize a Step it must consist with following minimum properties:
 * \code
 * -- type: action \a or if \a or ...
 * -- label: < \a label \a description >
 * \endcode
 *
 * Optional are the following properties:
 * \code
 * -- use context variable names: [ \a variable1, ... ]
 * -- time of last modification: %Y-%m-%d %H:%M:%S
 * -- time of last execution: %Y-%m-%d %H:%M:%S
 * -- timeout: [infinity|< \a timeout \a in \a milliseconds >]
 * \endcode
 *
 * Here is one example of a stored Step \a step_001_while.lua :
 * \code
 * -- type: while
 * -- label: Is increment lower then 10?
 * -- use context variable names: [incr]
 * -- time of last modification: 2022-06-13 16:30:32
 * -- time of last execution: 2022-06-13 16:55:21
 * -- timeout: 60000
 * return incr < 10
 * \endcode
 *
 * The label is explicitly escaped on storing and unescaped on loading.
 *
 * \note \c '--' is a Lua comment and interpreted with special keywords to fill the Step
 * properties.
 * \note for time interpretation see <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>
 * \note if <tt>>time of last modification</tt> is not provided in the file it is set to
 * the time on loading the step.
 * \note the collection of context variable names can also be an empty list, ie. \c [] .
 *
 * \param path  the path to a file from which the step should be loaded
 * \returns the deserialized Step object.
 */
Step deserialize_step(const std::filesystem::path& path);

/**
 * Deserialize Sequence from file path.
 *
 * \param path  a directory from which the sequence should be loaded
 * \returns the deserialized Sequence object.
 */
Sequence deserialize_sequence(const std::filesystem::path& path);

} // namespace task

#endif