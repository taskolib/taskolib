/**
 * \file   deserialize_sequence.h
 * \author Marcus Walla
 * \date   Created on May 24, 2022
 * \brief  Deserialize Sequence and Steps from storage hardware.
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

#ifndef TASKOLIB_DESERIALIZE_SEQUENCE_H_
#define TASKOLIB_DESERIALIZE_SEQUENCE_H_

#include <filesystem>
#include <iostream>

#include <gul14/optional.h>

#include "taskolib/Sequence.h"
#include "taskolib/SequenceName.h"
#include "taskolib/Step.h"
#include "taskolib/UniqueId.h"

namespace task {

struct SequenceInfo
{
    std::string label;
    gul14::optional<SequenceName> name;
    gul14::optional<UniqueId> unique_id;

    friend bool operator==(const SequenceInfo& a, const SequenceInfo& b) noexcept
    {
        return a.unique_id == b.unique_id && a.label == b.label;
    }

    friend bool operator!=(const SequenceInfo& a, const SequenceInfo& b) noexcept
    {
        return !(a == b);
    }
};

/**
 * Determine the label, name, and unique ID of a sequence from a filename, if possible.
 *
 * New versions of Taskolib store sequences in a folder following the scheme "name[uid]",
 * whereas old versions just used an escaped form of the label as the folder name. This
 * function looks for the angle brackets to figure out which of the two formats is used.
 *
 * This is the reverse function to make_sequence_filename().
 */
SequenceInfo get_sequence_info_from_filename(gul14::string_view filename);

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
 * Read a Step from a file and return it.
 *
 * \param lua_file  The filename from which to read the step. The file must be a Lua
 *                  script and should have the extension 'lua'.
 *
 * It will throw an Error exception if an I/O error occurs on the external filename object
 * or the file does not exist.
 *
 * Lua comments in the header of the file are used to read metadata. To load a Step, it
 * must have at least the following properties:
 * \code
 * -- type: action \a or if \a or ...
 * -- label: < \a label \a description >
 * \endcode
 *
 * The following properties are optional:
 * \code
 * -- use context variable names: [ \a variable1, ... ]
 * -- time of last modification: %Y-%m-%d %H:%M:%S
 * -- time of last execution: %Y-%m-%d %H:%M:%S
 * -- timeout: [infinity|< \a timeout \a in \a milliseconds >]
 * \endcode
 *
 * If one of the optional parameters is not set the following is provided as default:
 * - context variable names is an empty list
 * - time of last modification is set to a time stamp when loaded
 * - time of last execution is set to January 1st 1970
 * - timeout is set to 0s
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
 * \note for time interpretation see <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>
 * \note if <tt>>time of last modification</tt> is not provided in the file it is set to
 * the time on loading the step.
 * \note the collection of context variable names can also be an empty list, ie. \c [] .
 *
 * \returns the deserialized Step object.
 */
Step load_step(const std::filesystem::path& lua_file);

/**
 * Load sequence parameters like the step setup script and the sequence timeout.
 *
 * \param folder of the Sequence.
 * \param sequence to store the loaded step setup script.
 * \see Sequence for step setup script.
 */
void load_sequence_parameters(const std::filesystem::path& folder, Sequence& sequence);

} // namespace task

#endif
