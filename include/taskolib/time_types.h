/**
 * \file   time_types.h
 * \author Lars Fröhlich
 * \date   Created on April 8, 2022
 * \brief  Declaration of time-related types.
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

#ifndef TASKOLIB_TIME_TYPES_H_
#define TASKOLIB_TIME_TYPES_H_

#include <chrono>
#include <string>

namespace task {

using Clock = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<Clock>;

std::string to_string(task::TimePoint t);
std::time_t timegm(const std::tm& t);

} // namespace task

#endif
