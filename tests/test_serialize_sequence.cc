/**
 * \file   test_serialize_sequence.cc
 * \author Marcus Walla
 * \date   Created on May 6, 2022
 * \brief  Test suite for the free function serialize_sequence().
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

#include <gul14/gul.h>
#include <gul14/catch.h>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <system_error>
#include "taskomat/serialize_sequence.h"
#include "taskomat/deserialize_sequence.h"

using namespace task;
using namespace std::chrono_literals;

TEST_CASE("serialize_sequence: simple step", "[serialize_sequence]")
{
    std::stringstream ss;
    Step step{};

    SECTION("deserialize type")
    {
        step.set_type(Step::type_while);
        ss << step;

        Step deserialize{};
        ss >> deserialize;

        REQUIRE(deserialize.get_type() == Step::type_while);
    }
    
    SECTION("deserialize label")
    {
        step.set_label("This is a funny label");
        ss << step;

        Step deserialize{};
        ss >> deserialize;

        REQUIRE(deserialize.get_label() == "This is a funny label");
    }
    
    SECTION("deserialize infinite timeout")
    {
        step.set_timeout(Step::infinite_timeout);
        ss << step;

        Step deserialize{};
        ss >> deserialize;

        REQUIRE(deserialize.get_timeout() == Step::infinite_timeout);
    }
    
    SECTION("deserialize 1s timeout")
    {
        step.set_timeout(1s);
        ss << step;

        Step deserialize{};
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

        Step deserialize{};
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

        Step deserialize{};
        ss >> deserialize;

        REQUIRE(deserialize.get_time_of_last_execution() == ts);
    }
    
    SECTION("deserialize one variable name")
    {
        step.set_used_context_variable_names({"a"});
        ss << step;

        Step deserialize{};
        ss >> deserialize;

        REQUIRE(not deserialize.get_used_context_variable_names().empty());
        REQUIRE(deserialize.get_used_context_variable_names().size() == 1);
        REQUIRE(deserialize.get_used_context_variable_names() == VariableNames{"a"});
    }
    
    SECTION("deserialize more variable name")
    {
        step.set_used_context_variable_names({"a", "b"});
        ss << step;

        Step deserialize{};
        ss >> deserialize;

        REQUIRE(not deserialize.get_used_context_variable_names().empty());
        REQUIRE(deserialize.get_used_context_variable_names().size() == 2);
        REQUIRE(deserialize.get_used_context_variable_names() == VariableNames{"a", "b"});
    }
    
    SECTION("deserialize script")
    {
        step.set_script("-- some nasty comment\n\na = a + 1\n");
        ss << step;

        Step deserialize{};
        ss >> deserialize;

        REQUIRE(deserialize.get_script() == "-- some nasty comment\n\na = a + 1\n");
    }

    SECTION("deserialize step")
    {
        ss << step;

        Step deserialize{};
        ss >> deserialize;

        REQUIRE(deserialize.get_type() == Step::type_action);
        REQUIRE(deserialize.get_timeout() == Step::infinite_timeout);
        REQUIRE(deserialize.get_used_context_variable_names().empty());
        REQUIRE(deserialize.get_used_context_variable_names().size() == 0);
        REQUIRE(deserialize.get_used_context_variable_names() == VariableNames({}));
        REQUIRE(deserialize.get_indentation_level() == 0);
        REQUIRE(deserialize.get_label() == "");
        REQUIRE(deserialize.get_time_of_last_execution() == TimePoint{});
    }
}

TEST_CASE("serialize_sequence: indentation level & type of sequence",
    "[serialize_sequence]")
{
    Step step01{Step::type_while};
    Step step02{Step::type_action};
    Step step03{Step::type_end};

    Sequence sequence{"This is a sequence"};
    sequence.push_back(step01);
    sequence.push_back(step02);
    sequence.push_back(step03);

    REQUIRE_NOTHROW(serialize_sequence("/tmp/taskomat/test_fs_sequence", sequence));

    Sequence deserialize_seq = deserialize_sequence(
        "/tmp/taskomat/test_fs_sequence/This is a sequence");

    REQUIRE(not deserialize_seq.empty());
    REQUIRE(deserialize_seq.size() == 3);
    REQUIRE(deserialize_seq[0].get_indentation_level() == 0);
    REQUIRE(deserialize_seq[0].get_type() == Step::type_while);
    REQUIRE(deserialize_seq[1].get_indentation_level() == 1);
    REQUIRE(deserialize_seq[1].get_type() == Step::type_action);
    REQUIRE(deserialize_seq[2].get_indentation_level() == 0);
    REQUIRE(deserialize_seq[2].get_type() == Step::type_end);

    // remove the temp folder
    std::error_code e;
    std::filesystem::remove_all("/tmp/taskomat/test_fs_sequence", e);
    if (e)
        WARN("removing test folder fails: test_fs_sequence");
}