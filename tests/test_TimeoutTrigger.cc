/**
 * \file   test_TimeoutTrigger.cc
 * \author Marcus Walla
 * \date   Created on February 21, 2023
 * \brief  Test suite for the TriggerTimeout class.
 *
 * \copyright Copyright 2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <gul14/catch.h>
#include <gul14/time_util.h>

#include "taskolib/TimeoutTrigger.h"

using namespace std::literals;
using namespace task;

TEST_CASE("TimeoutTrigger: Default constructor", "[TimeoutTrigger]")
{
    static_assert(std::is_default_constructible<TimeoutTrigger>::value,
        "TimeoutTrigger is_default_constructible");
    static_assert(std::is_trivially_destructible<TimeoutTrigger>::value,
        "TimeoutTrigger is_trivially_destructible");

    TimeoutTrigger timeout_trigger;
    REQUIRE(timeout_trigger.get_start_time() == task::TimePoint{});
}

TEST_CASE("TimeoutTrigger: Default copy", "[TimeoutTrigger]")
{
    static_assert(std::is_copy_constructible<TimeoutTrigger>::value,
        "TimeoutTrigger is_copy_constructible");
    static_assert(std::is_copy_assignable<TimeoutTrigger>::value,
        "TimeoutTrigger is_copy_assignable");

    TimeoutTrigger timeout_trigger;

    SECTION("copy constructable")
    {
        TimeoutTrigger timeout_trigger_copy = timeout_trigger;
        timeout_trigger_copy.reset();

        REQUIRE(timeout_trigger.get_start_time() == task::TimePoint{});
        REQUIRE(timeout_trigger_copy.get_start_time() != task::TimePoint{});
    }

    SECTION("copy assignment operator")
    {
        timeout_trigger.set_timeout(321ms);

        TimeoutTrigger timeout_trigger_assignment;
        timeout_trigger_assignment.set_timeout(123ms);

        timeout_trigger_assignment = timeout_trigger;

        REQUIRE(timeout_trigger_assignment.get_timeout() == 321ms);
    }
}

TEST_CASE("TimeoutTrigger: get/set timeout & start time", "[TimeoutTrigger]")
{
    TimeoutTrigger timeout_trigger;

    REQUIRE(timeout_trigger.get_timeout() == Timeout::infinity());

    timeout_trigger.set_timeout(200ms);
    REQUIRE(timeout_trigger.get_timeout() == 200ms);

    auto start_time = timeout_trigger.reset();
    REQUIRE(timeout_trigger.get_start_time() == start_time);
}

TEST_CASE("TimeoutTrigger: check elapsed timeout", "[TimeoutTrigger]")
{
    TimeoutTrigger timeout_trigger;

    timeout_trigger.set_timeout(200ms);

    gul14::sleep(10ms);
    REQUIRE(timeout_trigger.is_elapsed() == true);
}
