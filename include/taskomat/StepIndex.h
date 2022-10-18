/**
 * \file   StepIndex.h
 * \author Lars Froehlich
 * \date   Created on October 18, 2022
 * \brief  Declaration of the StepIndex type.
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

#ifndef TASKOMAT_STEPINDEX_H_
#define TASKOMAT_STEPINDEX_H_

namespace task {

/// A type for storing the index of a Step in a Sequence.
using StepIndex = std::uint16_t;

} // namespace task

#endif
