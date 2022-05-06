/**
 * \file   Serialize.h
 * \author Marcus Walla
 * \date   Created on May 6, 2022
 * \brief  Serialize Sequence and Steps on media storage hardware.
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

#include <string>
#include "taskomat.h"

namespace task {

/**
 * Serialize \a Sequence and \a Context to a file.
 *
 * @param path Path to store the \a Sequence and \a Context
 * @param sequence \a Sequence to be serialized
 * @param conext \a Context to be serialzed
 */
void serialize_sequence(const std::string& path, const Sequence& sequence
    , const Context& context);

} // namespace task

#endif