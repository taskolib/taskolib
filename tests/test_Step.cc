/**
 * \file   test_Step.cc
 * \author Lars Froehlich
 * \date   Created on November 26, 2021
 * \brief  Test suite for the Step class.
 *
 * \copyright Copyright 2021 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <type_traits>
#include <gul14/catch.h>
#include "taskomat/Error.h"
#include "taskomat/Step.h"

using namespace std::literals;
using namespace task;

TEST_CASE("Step: Default constructor", "[Step]")
{
    static_assert(std::is_default_constructible<Step>::value,
        "Step is_default_constructible");

    Step s;
}

TEST_CASE("Step: get_indentation_level()", "[Step]")
{
    Step step;
    REQUIRE(step.get_indentation_level() == 0);

    step.set_indentation_level(3);
    REQUIRE(step.get_indentation_level() == 3);
}

TEST_CASE("Step: get_label()", "[Step]")
{
    Step step;
    REQUIRE(step.get_label() == "");
    step.set_label("Do nothing");
    REQUIRE(step.get_label() == "Do nothing");
}

TEST_CASE("Step: get_script()", "[Step]")
{
    Step step;
    REQUIRE(step.get_script() == "");
    step.set_script("a = 42");
    REQUIRE(step.get_script() == "a = 42");
}

TEST_CASE("Step: get_time_of_last_execution()", "[Step]")
{
    Step step;
    REQUIRE(step.get_time_of_last_execution() == Timestamp{});

    auto ts = Clock::now();
    step.set_time_of_last_execution(ts);
    REQUIRE(step.get_time_of_last_execution() == ts);
}

TEST_CASE("Step: get_time_of_last_modification()", "[Step]")
{
    Step step;
    REQUIRE(step.get_time_of_last_modification() == Timestamp{});

    auto ts = Clock::now();
    step.set_time_of_last_modification(ts);
    REQUIRE(step.get_time_of_last_modification() == ts);
}

TEST_CASE("Step: get_timeout()", "[Step]")
{
    Step step;
    REQUIRE(step.get_timeout() == Step::infinite_timeout);

    step.set_timeout(42s);
    REQUIRE(step.get_timeout() == 42'000ms);
}

TEST_CASE("Step: get_type()", "[Step]")
{
    Step step;
    REQUIRE(step.get_type() == Step::type_action);
    step.set_type(Step::type_catch);
    REQUIRE(step.get_type() == Step::type_catch);
    step.set_type(Step::type_if);
    REQUIRE(step.get_type() == Step::type_if);
}

TEST_CASE("Step: get_used_context_variable_names()", "[Step]")
{
    Step step;
    REQUIRE(step.get_used_context_variable_names().empty() == true);

    step.set_used_context_variable_names(VariableNames{ "b52", "a" });
    REQUIRE(step.get_used_context_variable_names() == VariableNames{ "a", "b52" });
}

TEST_CASE("Step: set_indentation_level()", "[Step]")
{
    Step step;

    step.set_indentation_level(3);
    REQUIRE(step.get_indentation_level() == 3);

    step.set_indentation_level(0);
    REQUIRE(step.get_indentation_level() == 0);

    step.set_indentation_level(Step::max_indentation_level);
    REQUIRE(step.get_indentation_level() == Step::max_indentation_level);

    REQUIRE_THROWS_AS(step.set_indentation_level(-1), Error);
    REQUIRE_THROWS_AS(step.set_indentation_level(-42), Error);
    REQUIRE_THROWS_AS(step.set_indentation_level(Step::max_indentation_level + 1), Error);
    REQUIRE_THROWS_AS(step.set_indentation_level(30'000), Error);
}

TEST_CASE("Step: set_label()", "[Step]")
{
    Step step;
    REQUIRE(step.get_time_of_last_modification() == Timestamp{});

    step.set_label("Do nothing");
    REQUIRE(step.get_label() == "Do nothing");
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);

    step.set_label("Do something");
    REQUIRE(step.get_label() == "Do something");
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);
}

TEST_CASE("Step: set_time_of_last_execution()", "[Step]")
{
    Step step;

    auto ts = Clock::now();
    step.set_time_of_last_execution(ts);
    REQUIRE(step.get_time_of_last_execution() == ts);

    step.set_time_of_last_execution(ts - 42s);
    REQUIRE(step.get_time_of_last_execution() == ts - 42s);
}

TEST_CASE("Step: set_time_of_last_modification()", "[Step]")
{
    Step step;

    auto ts = Clock::now();
    step.set_time_of_last_modification(ts);
    REQUIRE(step.get_time_of_last_modification() == ts);

    step.set_time_of_last_modification(ts - 42s);
    REQUIRE(step.get_time_of_last_modification() == ts - 42s);
}

TEST_CASE("Step: set_timeout()", "[Step]")
{
    Step step;

    step.set_timeout(42s);
    REQUIRE(step.get_timeout() == 42s);

    step.set_timeout(-2ms);
    REQUIRE(step.get_timeout() == 0s);

    step.set_timeout(Step::infinite_timeout);
    REQUIRE(step.get_timeout() == Step::infinite_timeout);
}

TEST_CASE("Step: set_type()", "[Step]")
{
    Step step;
    REQUIRE(step.get_time_of_last_modification() == Timestamp{});

    step.set_type(Step::type_while);
    REQUIRE(step.get_type() == Step::type_while);
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);

    step.set_type(Step::type_end);
    REQUIRE(step.get_type() == Step::type_end);
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);
}

TEST_CASE("Step: set_script()", "[Step]")
{
    Step step;
    REQUIRE(step.get_time_of_last_modification() == Timestamp{});

    step.set_script("test");
    REQUIRE(step.get_script() == "test");
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);

    step.set_script("test 2");
    REQUIRE(step.get_script() == "test 2");
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);
}

TEST_CASE("Step: set_used_context_variable_names()", "[Step]")
{
    Step step;

    step.set_used_context_variable_names(VariableNames{ "b52", "a" });
    REQUIRE(step.get_used_context_variable_names() == VariableNames{ "a", "b52" });
}
