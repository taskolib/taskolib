/**
 * \file   serialize_sequence.h
 * \author Marcus Walla
 * \date   Created on May 6, 2022
 * \brief  Serialize Sequence and Steps on storage hardware.
 *
 * \copyright Copyright 2022-2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOLIB_SERIALIZE_SEQUENCE_H_
#define TASKOLIB_SERIALIZE_SEQUENCE_H_

#include <filesystem>
#include <iostream>

#include <gul14/string_view.h>

#include "taskolib/Sequence.h"
#include "taskolib/Step.h"
#include "taskolib/UniqueId.h"

namespace task {

/**
 * Return a filename for the given combination of machine-friendly sequence name and
 * unique ID.
 *
 * It is guaranteed that the filename does not contain filesystem-unsafe characters like
 * / or \.
 */
std::string make_sequence_filename(SequenceName sequence_name, UniqueId unique_id);

/**
 * Serialize parameters of Step to the output stream.
 *
 * No checking of any stream failure is done and should be performed by the caller.
 *
 * \param stream to serialize the Step
 * \param step to serialize
 * \return passed output stream
 */
std::ostream& operator<<(std::ostream& stream, const Step& step);

/**
 * Store a Step in a file.
 *
 * This function saves the script of this step in a Lua file. Metadata like the step type
 * or the label are stored as comments in the header of the file:
 *
 * \code
 * -- type: {action, if, elseif, else, while, try, catch, end}
 * -- label: < \a label \a description >
 * -- use context variable names: [ \a variable1, ... ]
 * -- time of last modification: %Y-%m-%d %H:%M:%S
 * -- time of last execution: %Y-%m-%d %H:%M:%S
 * -- timeout: [infinity|< \a time \a in \a milliseconds >]
 * < \a Lua \a script >
 * \endcode
 *
 * The \a Lua \a script can be omitted for type \a try, \a catch, and \a end as it has no
 * meaning for execution. See Step::execution for more information. The list of context
 * variable names can also be empty.
 *
 * Here is one example of a stored Step `step_001_action.lua`:
 *
 * \code
 * -- type: action
 * -- label: Increment a
 * -- use context variable names: [a]
 * -- time of last modification: 2022-06-13 16:30:32
 * -- time of last execution: 2022-06-13 16:55:21
 * -- timeout: infinity
 * a = a + 1
 * \endcode
 *
 * The label is explicitly escaped on storing and unescaped on loading.
 *
 * \param lua_file  filename under which the step should be stored
 * \param step  the Step object that should be serialized
 */
void store_step(const std::filesystem::path& lua_file, const Step& step);

/**
 * Serialize parameters of Sequence to the output stream.
 *
 * No checking of any stream failure is done and should be performed by the caller.
 *
 * \param stream to serialize the Step
 * \param sequence to serialize
 * \return passed output stream
 */
std::ostream& operator<<(std::ostream& stream, const Sequence& sequence);

} // namespace task

#endif
