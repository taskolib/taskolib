/**
 * \file   test_time_types.cc
 * \author Fini Jastrow
 * \date   Created on June 22, 2022
 * \brief  Test suite for the TimePoint class (i.e. alias).
 *
 * \copyright Copyright 2022-2025 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <random>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "taskolib/time_types.h"

TEST_CASE("TimePoint: Dump to stream", "[TimePoint]")
{
    std::stringstream ss{ };
    ss << task::to_string(task::TimePoint{ });
    REQUIRE(ss.str() == "1970-01-01 00:00:00 UTC");
}

TEST_CASE("timegm() random conversions", "[timegm]")
{
    // For 32bit time_t we probably can not go twice-now into the future
    auto not_far_future = std::time(nullptr);
    if (std::numeric_limits<std::time_t>::max() / 2 > not_far_future)
        not_far_future *= 2;
    else
        not_far_future = std::numeric_limits<std::time_t>::max();

    std::random_device r{ };
    std::default_random_engine re{ r() };
    std::uniform_int_distribution<std::time_t> uniform_dist(0, not_far_future);

    for (int i = 10'000; i != 0; --i) {
        auto r = uniform_dist(re);
        INFO(r);
        auto t1 = std::time_t{ r };
        auto in_tm = gmtime(&t1);
        auto t2 = task::timegm(*in_tm);
        INFO(t1 - t2);
        REQUIRE(t1 == t2);
    }
}
