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
 * Deserialize parameters of \a Step from the input stream. 
 * 
 * @param stream input stream
 * @param step \a Step to be deserialized. 
 * @return passed input stream
 */
std::istream& operator>>(std::istream& stream, Step& step);

/**
 * Deserialize \a Sequence from file path.
 *
 * @param path Path to load \a Sequence and all included \a Step 's
 * @return deserialized \a Sequence
 */
Sequence deserialize_sequence(const std::filesystem::path& path);

} // namespace task

#endif