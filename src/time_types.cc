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
#include <time.h>

#include "taskolib/exceptions.h"
#include "taskolib/time_types.h"

namespace task {

std::string to_string(TimePoint t) {
    auto in_time_t = Clock::to_time_t(t);
    auto in_tm = std::tm{ };
#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
    auto ret = gmtime_s(&in_tm, &in_time_t); // Windows swaps the arguments
#else
    auto ret = gmtime_r(&in_time_t, &in_tm);
#endif
    if (not ret) // ret is std::tm* on Linux or errno_t on Windows
        throw Error{ "Can not format TimePoint" };
    auto str = std::string(25, '\0');
    auto len = std::strftime(str.data(), str.capacity(), "%Y-%m-%d %H:%M:%S UTC", &in_tm);
    if (len == 0)
        throw Error{ "Can not format TimePoint" };
    str.resize(len);
    return str;
}

std::time_t timegm(const std::tm& t) {
        // Does not allow for tm_mon > 11
        // Inspired from http://www.catb.org/esr/time-programming/
        static const int cumulated_days[12] =
                { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
        long year = t.tm_year + 1900;
        std::time_t days = (year - 1970) * 365 + cumulated_days[t.tm_mon];
        if (t.tm_mon < 2) // January and February are normal in leap years regarding cumulated_days
            year--;
        days += (year - 1968) / 4;
        days -= (year - 1900) / 100;
        days += (year - 1600) / 400;
        return (days + t.tm_mday - 1) * 24 * 60 * 60 // days
                + (t.tm_hour - (t.tm_isdst == 1)) * 60 * 60 // hours
                + t.tm_min * 60 // minutes
                + t.tm_sec; // seconds
}

} // namespace task
