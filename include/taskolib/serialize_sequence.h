/**
 * \file   serialize_sequence.h
 * \author Marcus Walla
 * \date   Created on May 6, 2022
 * \brief  Serialize Sequence and Steps on storage hardware.
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

#ifndef TASKOLIB_SERIALIZE_H_
#define TASKOLIB_SERIALIZE_H_

#include <filesystem>
#include <iostream>
#include "taskolib/Step.h"
#include "taskolib/Sequence.h"

namespace task {

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
 * Store Step to the file system. It pushes the following Step properties to the created
 * file stream:
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
 * \param folder for the Step
 * \param step step to serialize
 */
void store_step(const std::filesystem::path& folder, const Step& step);

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

/**
 * Stores Sequence with all of its Step 's as files.
 *
 * After storing you will find the following structure:
 *
 * - the sequence label is extracted to a folder name, where underneath all steps are
 *  serialized. If the label has one of the following characters they are escaped to
 *  hexadecimal format: /\\:?*"'<>|$&. Moreover all control characters (<= 32) are
 *  converted to space character (' ').
 *
 * - underneath the sequence folder you will find the Step 's serialized in files. To
 *  differ the Step they are enumerated. Each filename starts with `step` followed by a
 *  consecutive step enumeration number followed by type. Since you can directly evaluate
 *  the step as a Lua script it has the extension `'.lua'`. The step numbering is filled
 *  with '0' to allow alphanumerical sorting.
 *
 *  Here is one example for the first step that has type `action`: `step_001_action.lua`
 *
 *  \note Remember that the libary only uses three digit for numbering. There is no
 *  guarantee for serializing more then 1000 Step 's in alphabetical order.
 *
 * - important Step parameters are exported to the beginning of the file as Lua
 *  comments. See ::store_step(const std::filesystem::path&, const Step&) for more
 *  information.
 *
 * \param folder to store Sequence
 * \param sequence to be serialized
 */
void store_sequence(const std::filesystem::path& folder, const Sequence& sequence);

} // namespace task

#endif
