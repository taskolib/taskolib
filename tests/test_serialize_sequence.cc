/**
 * \file   test_serialize_sequence.cc
 * \author Marcus Walla
 * \date   Created on May 6, 2022
 * \brief  Test suite for the free function serialize_sequence().
 *
 * \copyright Copyright 2022-2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <algorithm>
#include <filesystem>
#include <sstream>

#include <gul14/catch.h>

#include "deserialize_sequence.h"
#include "internals_unit_test.h"
#include "serialize_sequence.h"
#include "taskolib/Sequence.h"

using namespace std::literals;
using namespace task;
using namespace task::literals;

TEST_CASE("make_sequence_filename(SequenceName, UniqueId)", "[serialize_sequence]")
{
    REQUIRE(make_sequence_filename(SequenceName{ "A_sequence" }, 0xdeadbeef_uid)
        == "A_sequence[00000000deadbeef]");
    REQUIRE(make_sequence_filename(SequenceName{ "my_seq_name" }, 1_uid)
        == "my_seq_name[0000000000000001]");
    REQUIRE(make_sequence_filename(SequenceName{}, 0x1234_uid) == "[0000000000001234]");
}

TEST_CASE("serialize_sequence: simple step", "[serialize_sequence]")
{
    std::stringstream ss;
    Step step;
    step.set_type(Step::type_while);
    step.set_label("This is a label");

    SECTION("deserialize: minimum type & label")
    {
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(deserialize.get_type() == Step::type_while);
        REQUIRE(deserialize.get_label() == "This is a label");
    }

    SECTION("deserialize: escaped label")
    {
        step.set_label("This is a funny label\n-- label: NOT FUNNY");
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(deserialize.get_label() == "This is a funny label\n-- label: NOT FUNNY");
    }

    SECTION("deserialize: whitespaced label")
    {
        step.set_label(" L a b e l ");
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(deserialize.get_label() == "L a b e l"); // stripped label
    }

    SECTION("deserialize infinite timeout")
    {
        step.set_timeout(Timeout::infinity());
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(isfinite(deserialize.get_timeout()) == false);
    }

    SECTION("deserialize 1s timeout")
    {
        step.set_timeout(1s);
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(deserialize.get_timeout() == 1s);
    }

    SECTION("deserialize last modification time")
    {
        // IMPROVEMENT: very boring but since we store the time with second precision
        // there is a need for this nasty transition ... :/
        auto now = TimePoint::clock::to_time_t(Clock::now());
        auto ts = TimePoint::clock::from_time_t(now);

        step.set_time_of_last_modification(ts);
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(deserialize.get_time_of_last_modification() == ts);
    }

    SECTION("deserialize last execution time")
    {
        // IMPROVEMENT: very boring but since we store the time with second precision
        // there is a need for this nasty transition ... :/
        auto now = TimePoint::clock::to_time_t(Clock::now());
        auto ts = TimePoint::clock::from_time_t(now);

        step.set_time_of_last_execution(ts);
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(deserialize.get_time_of_last_execution() == ts);
    }

    SECTION("deserialize one variable name")
    {
        step.set_used_context_variable_names({"a"});
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(not deserialize.get_used_context_variable_names().empty());
        REQUIRE(deserialize.get_used_context_variable_names().size() == 1);
        REQUIRE(deserialize.get_used_context_variable_names() == VariableNames{"a"});
    }

    SECTION("deserialize more variable name")
    {
        step.set_used_context_variable_names({"a", "b"});
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(not deserialize.get_used_context_variable_names().empty());
        REQUIRE(deserialize.get_used_context_variable_names().size() == 2);
        REQUIRE(deserialize.get_used_context_variable_names() == VariableNames{"a", "b"});
    }

    SECTION("deserialize script")
    {
        step.set_script("-- some nasty comment\n\na = a + 1\n");
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(deserialize.get_script() == "-- some nasty comment\n\na = a + 1\n");
    }

    SECTION("deserialize step")
    {
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(deserialize.get_type() == Step::type_while);
        REQUIRE(deserialize.get_label() == "This is a label");
        REQUIRE(isfinite(deserialize.get_timeout()) == false);
        REQUIRE(deserialize.get_used_context_variable_names().empty());
        REQUIRE(deserialize.get_used_context_variable_names().size() == 0);
        REQUIRE(deserialize.get_used_context_variable_names() == VariableNames({}));
        REQUIRE(deserialize.get_indentation_level() == 0);
        REQUIRE(deserialize.get_time_of_last_execution() == TimePoint{});
        REQUIRE(deserialize.is_disabled() == false);
    }

    SECTION("deserialize disabled flag")
    {
        step.set_disabled(true);
        ss << step;

        Step deserialize;
        ss >> deserialize;

        REQUIRE(deserialize.is_disabled() == true);
    }
}

TEST_CASE("serialize_sequence: deserialize with nasty blanks (1)", "[serialize_sequence]")
{
    std::stringstream ss{
R"(
-- type: action
    -- label: This is a label
)"};

    Step deserialize;
    ss >> deserialize;

    REQUIRE(deserialize.get_type() == Step::type_action);
    REQUIRE(deserialize.get_label() == "This is a label");
}

TEST_CASE("serialize_sequence: deserialize with nasty blanks (2)", "[serialize_sequence]")
{
    std::stringstream ss{"\n\t-- type:  action\t \n   \n--    label:  This is a label  "};

    Step deserialize;
    ss >> deserialize;

    REQUIRE(deserialize.get_type() == Step::type_action);
    REQUIRE(deserialize.get_label() == "This is a label");
}

TEST_CASE("serialize_sequence: deserialize with unknown type", "[serialize_sequence]")
{
    std::stringstream ss{R"(
-- type: abc
-- label: This is a label)"};

    Step deserialize;
    REQUIRE_THROWS_AS(ss >> deserialize, Error);
}

TEST_CASE("serialize_sequence: deserialize with nasty delimiter on variable names",
    "[serialize_sequence]")
{
    std::stringstream ss{
R"(-- type: action
-- label: This is a label
-- use context variable names: [ aaa   ,   bb,cc , d ]
--)"};

    Step deserialize;
    ss >> deserialize;

    VariableNames compare_variable_names{"aaa", "bb", "cc", "d"};

    REQUIRE(deserialize.get_type() == Step::type_action);
    REQUIRE(deserialize.get_label() == "This is a label");
    REQUIRE(not deserialize.get_used_context_variable_names().empty());
    REQUIRE(deserialize.get_used_context_variable_names().size() == 4);
    REQUIRE(deserialize.get_used_context_variable_names() == compare_variable_names);
}

TEST_CASE("serialize_sequence: deserialize with Step properties last",
    "[serialize_sequence]")
{
    std::stringstream ss{R"(
-- type: action
-- label: This is a label

a = a + 1
-- label: This label has no meaning for Step declaration)"};

    Step deserialize;
    ss >> deserialize;

    REQUIRE(deserialize.get_label() == "This is a label");
    REQUIRE(deserialize.get_script() == R"(a = a + 1
-- label: This label has no meaning for Step declaration)");
}

TEST_CASE("serialize_sequence: deserialize with reseting Step", "[serialize_sequence]")
{
    std::stringstream ss{R"(
-- type: action
-- label: This is a label)"};

    Step deserialize;
    deserialize.set_type(Step::type_while);
    deserialize.set_label("This is another label");
    deserialize.set_timeout(1s);

    ss >> deserialize;

    REQUIRE(deserialize.get_type() == Step::type_action);
    REQUIRE(deserialize.get_label() == "This is a label");
    REQUIRE(isfinite(deserialize.get_timeout()) == false);
}

TEST_CASE("serialize_sequence: deserialize with two types", "[serialize_sequence]")
{
    std::stringstream ss{
R"(
-- type: action
-- label: This is a label
-- type: action
)"};

    Step deserialize;
    REQUIRE_THROWS_AS(ss >> deserialize, Error);
}

TEST_CASE("serialize_sequence: deserialize with two labels", "[serialize_sequence]")
{
    std::stringstream ss{
R"(
-- type: action
-- label: This is a label
-- label: This is again a label
)"};

    Step deserialize;
    REQUIRE_THROWS_AS(ss >> deserialize, Error);
}

TEST_CASE("serialize_sequence: deserialize with two time of last modifications",
    "[serialize_sequence]")
{
    std::stringstream ss{
R"(
-- type: action
-- label: This is a label
-- time of last modification: 2022-6-15 19:11:00
-- time of last modification: 2022-6-15 19:11:00
)"};

    Step deserialize;
    REQUIRE_THROWS_AS(ss >> deserialize, Error);
}

TEST_CASE("serialize_sequence: deserialize with two time of last executions",
    "[serialize_sequence]")
{
    std::stringstream ss{
R"(
-- type: action
-- label: This is a label
-- time of last execution: 2022-6-15 19:11:00
-- time of last execution: 2022-6-15 19:11:00
)"};

    Step deserialize;
    REQUIRE_THROWS_AS(ss >> deserialize, Error);
}

TEST_CASE("serialize_sequence: deserialize with two context variables names",
    "[serialize_sequence]")
{
    std::stringstream ss{
R"(
-- type: action
-- label: This is a label
-- use context variable names: [aa]
-- use context variable names: [aa]
)"};

    Step deserialize;
    REQUIRE_THROWS_AS(ss >> deserialize, Error);
}

TEST_CASE("serialize_sequence: deserialize with two timeouts", "[serialize_sequence]")
{
    std::stringstream ss{
R"(
-- type: action
-- label: This is a label
-- timeout: 1000
-- timeout: 1000
)"};

    Step deserialize;
    REQUIRE_THROWS_AS(ss >> deserialize, Error);
}
