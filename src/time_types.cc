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
#include <sstream>

#include "taskomat/Error.h"
#include "taskomat/time_types.h"

namespace task {

std::string dump_timepoint(TimePoint t) {
    auto in_time_t = Clock::to_time_t(t);
    auto in_tm = std::gmtime(&in_time_t);
    auto str = std::string(25, '\0');
    auto len = std::strftime(str.data(), str.capacity(), "%Y-%m-%d %H:%M:%S UTC", in_tm);
    if (len == 0)
        throw Error{ "Can not format TimePoint" };
    str.resize(len);
    return str;
}

} // namespace task
