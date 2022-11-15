/**
 * \file   test_lua_details.cc
 * \author Lars Froehlich
 * \date   Created on October 28, 2022
 * \brief  Test suite for Lua-related internal functions.
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

#include <limits>

#include <gul14/catch.h>

#include "../src/lua_details.h"

using namespace std::chrono;
using namespace std::literals;
using namespace task;

TEST_CASE("get_ms_since_epoch()", "[lua_details]")
{
    const auto now = Clock::now();

    REQUIRE(get_ms_since_epoch(TimePoint{}, 100ms) == 100);
    REQUIRE(get_ms_since_epoch(now, 100ms)
            == round<milliseconds>((now + 100ms).time_since_epoch()).count());

    if constexpr (milliseconds::max().count() >= std::numeric_limits<long long>::max())
    {
        REQUIRE(get_ms_since_epoch(now, milliseconds::max())
                == std::numeric_limits<long long>::max());
    }
}
