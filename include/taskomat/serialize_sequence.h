/**
 * \file   serialize_sequence.h
 * \author Marcus Walla
 * \date   Created on May 6, 2022
 * \brief  Serialize Sequence and Steps on storage hardware.
 *
 * \copyright Copyright 2021-2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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
#include "taskomat.h"

namespace task {

/**
 * Serialize parameters of \a Step to the output stream. 
 * 
 * @param stream output stream
 * @param step \a Step to serialize. 
 * @return std::ostream& output stream
 */
std::ostream& operator<<(std::ostream& stream, const Step& step);

/**
 * Serialize \a Sequence with all of its \a Step 's as files.
 * 
 * After serializing you will find the following structure:
 * 
 * - a folder, starting with \code sequence that carries all \a Step 's serialized as
 *  files. To differ between sequences the label is attached to the sequence, replacing
 *  space (' ') with underline ('_'). More then one spaces is reduced to one underline
 *  character.
 * - underneath the sequence folder you will find the \a Step serialized in file. To
 *  differ the \a Step from the sequence they are enumerated. Each filename starts with 
 *  \code step followed by a consecutive step enumeration number followed
 *  by type. Since you can directly evaluate the step as a Lua script it has the
 *  extension \code '.lua'. Here is one example for the first step that has type 
 *  \code action: \code step_1_action.lua
 * - important \a Step parameters are exported to the beginning of the file as Lua
 *  comments.
 *
 * @param path Path to store \a Sequence
 * @param sequence \a Sequence to be serialized
 */
void serialize_sequence(const std::filesystem::path& path, const Sequence& sequence);

} // namespace task

#endif