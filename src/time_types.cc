/**
 * \file   time_types.cc
 * \author Fini Jastrow
 * \date   Created on June 22, 2022
 * \brief  Conversion for TimePoint
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

#include <ctime>
#include <iomanip>

#include "taskomat/time_types.h"

std::ostream& operator<<(std::ostream& stream, task::TimePoint t) {
    auto in_time_t = task::Clock::to_time_t(t);
    stream << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%d %H:%M:%S UTC");
    return stream;
}