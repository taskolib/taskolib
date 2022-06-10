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

#ifndef TASKOMAT_SERIALIZE_H_
#define TASKOMAT_SERIALIZE_H_

#include <filesystem>
#include <iostream>
#include "taskomat/Step.h"
#include "taskomat/Sequence.h"

namespace task {

/**
 * Serialize parameters of \a Step to the output stream. 
 * 
 * @param stream output stream
 * @param step \a Step to serialize. 
 * @return passed output stream
 */
std::ostream& operator<<(std::ostream& stream, const Step& step);

/**
 * Serialize \a Sequence with all of its \a Step 's as files.
 * 
 * After serializing you will find the following structure:
 * 
 * - the sequence label is extracted to a folder name, where underneath all steps are
 *  serialized. If the label has one of the following characters they are escaped to
 *  hexadecimal format: /\\:?*"'<>|$&. Moreover all control characters (<= 32) are 
 *  converted to space character (' ').
 * - underneath the sequence folder you will find the \a Step serialized in file. To
 *  differ the \a Step from the sequence they are enumerated. Each filename starts with 
 *  `step` followed by a consecutive step enumeration number followed
 *  by type. Since you can directly evaluate the step as a Lua script it has the
 *  extension `'.lua'`. Here is one example for the first step that has type 
 *  `action`: `step_1_action.lua`
 * - important \a Step parameters are exported to the beginning of the file as Lua
 *  comments.
 *
 * @param path Path to store \a Sequence
 * @param sequence \a Sequence to be serialized
 */
void serialize_sequence(const std::filesystem::path& path, const Sequence& sequence);

} // namespace task

#endif