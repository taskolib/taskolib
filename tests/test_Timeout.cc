/**
 * \file   test_Timeout.cc
 * \author Lars Fröhlich
 * \date   Created on November 28, 2022
 * \brief  Test suite for the Timeout class.
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

#include <cmath>
#include <stdexcept>
#include <sstream>

#include <gul14/catch.h>

#include "taskolib/Timeout.h"

using namespace std::literals;
using namespace task;
using namespace Catch::Matchers;

TEST_CASE("Timeout: Default constructor", "[Timeout]")
{
    static_assert(std::is_default_constructible<Timeout>::value,
        "Timeout is_default_constructible");

    Timeout t;
    REQUIRE(!isfinite(t));
}

TEST_CASE("Timeout: Construction from chrono duration", "[Timeout]")
{
    REQUIRE_THROWS_AS(Timeout{ -1ms }, Error);
    REQUIRE_THROWS_AS(Timeout{ -10h }, Error);
    REQUIRE(isfinite(Timeout{ 0s }));
    REQUIRE(isfinite(Timeout{ 1s }));
    REQUIRE(isfinite(Timeout{ 24h }));
    REQUIRE(!isfinite(Timeout{ std::chrono::duration<double>{ 1.0e100 } }));
}

TEST_CASE("Timeout: Construction from floating-point value", "[Timeout]")
{
    REQUIRE_THROWS_AS(Timeout{ -10.0 }, Error);
    REQUIRE_THROWS_AS(Timeout{ -INFINITY }, Error);
    REQUIRE_THROWS_AS(Timeout{ NAN }, Error);
    REQUIRE(isfinite(Timeout{ 0.0f }));
    REQUIRE(isfinite(Timeout{ 1.0 }));
    REQUIRE(isfinite(Timeout{ 24.0f * 60.0f * 60.0f }));
    REQUIRE(!isfinite(Timeout{ 1e100 }));
    REQUIRE(!isfinite(Timeout{ INFINITY }));
}

TEST_CASE("Timeout: Implicit cast to floating point", "[Timeout]")
{
    REQUIRE(static_cast<double>(Timeout{ 1s }) == 1.0);
    REQUIRE(static_cast<float>(Timeout{ 500ms }) == 0.5f);

    float timeout_float = Timeout{};
    REQUIRE(timeout_float == std::numeric_limits<float>::infinity());

    double timeout_double = Timeout::infinity();
    REQUIRE(timeout_double == std::numeric_limits<float>::infinity());
}

TEST_CASE("Timeout: Explicit cast to chrono durations", "[Timeout]")
{
    REQUIRE(static_cast<std::chrono::milliseconds>(Timeout{ 1s }) == 1000ms);
    REQUIRE(static_cast<std::chrono::duration<double>>(Timeout{ 500ms }) == 0.5s);
    REQUIRE(static_cast<std::chrono::seconds>(Timeout::infinity()) == std::chrono::seconds::max());
    REQUIRE(static_cast<std::chrono::nanoseconds>(Timeout::infinity()) == std::chrono::nanoseconds::max());
}

TEST_CASE("Timeout: Comparison operators", "[Timeout]")
{
    SECTION("t0 < t1, both finite")
    {
        auto t0 = Timeout{ 1s };
        auto t1 = Timeout{ 2s };

        REQUIRE((t0 == t1) == false);
        REQUIRE((t1 == t0) == false);
        REQUIRE((t0 != t1) == true);
        REQUIRE((t1 != t0) == true);
        REQUIRE((t0 < t1) == true);
        REQUIRE((t1 < t0) == false);
        REQUIRE((t0 > t1) == false);
        REQUIRE((t1 > t0) == true);
        REQUIRE((t0 <= t1) == true);
        REQUIRE((t1 <= t0) == false);
        REQUIRE((t1 >= t0) == true);
        REQUIRE((t0 >= t1) == false);
    }

    SECTION("t0 == t1, both finite")
    {
        auto t0 = Timeout{ 1s };
        auto t1 = Timeout{ 1s };

        REQUIRE((t0 == t1) == true);
        REQUIRE((t1 == t0) == true);
        REQUIRE((t0 != t1) == false);
        REQUIRE((t1 != t0) == false);
        REQUIRE((t0 < t1) == false);
        REQUIRE((t1 < t0) == false);
        REQUIRE((t0 > t1) == false);
        REQUIRE((t1 > t0) == false);
        REQUIRE((t0 <= t1) == true);
        REQUIRE((t1 <= t0) == true);
        REQUIRE((t1 >= t0) == true);
        REQUIRE((t0 >= t1) == true);
    }

    SECTION("t0 finite, t1 infinite")
    {
        auto t0 = Timeout{ 1s };
        auto t1 = Timeout::infinity();

        REQUIRE((t0 == t1) == false);
        REQUIRE((t1 == t0) == false);
        REQUIRE((t0 != t1) == true);
        REQUIRE((t1 != t0) == true);
        REQUIRE((t0 < t1) == true);
        REQUIRE((t1 < t0) == false);
        REQUIRE((t0 > t1) == false);
        REQUIRE((t1 > t0) == true);
        REQUIRE((t0 <= t1) == true);
        REQUIRE((t1 <= t0) == false);
        REQUIRE((t1 >= t0) == true);
        REQUIRE((t0 >= t1) == false);
    }
}

TEST_CASE("Timeout: Dump to stream", "[Timeout]")
{
    auto t = Timeout{ 0s };
    std::stringstream ss{ };
    ss << t;
    REQUIRE(ss.str() == "0");

    t = Timeout{ 10s };
    ss.str("");
    ss << t;
    REQUIRE(ss.str() == "10000");

    t = Timeout::infinity();
    ss.str("");
    ss << t;
    REQUIRE(ss.str() == "infinite");
}
