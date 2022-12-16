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

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <gul14/catch.h>
#include <gul14/gul.h>
#include <sstream>
#include <system_error>
#include <vector>

#include "../src/internals.h" // SEQUENCE_LUA_FILENAME
#include "taskolib/deserialize_sequence.h"
#include "taskolib/serialize_sequence.h"

using namespace task;
using namespace std::literals;

static const auto temp_dir = "unit_test"s;

// Remove the previous created temp folder
// This is executed before main() is called
static auto prepare_filesystem = []() { return std::filesystem::remove_all(temp_dir); }();

// Helper that returns all Lua step entries of a directory (i.e. files or subdirs)
std::vector<std::string> collect_lua_filenames(const std::filesystem::path& path)
{
    std::vector<std::string> result;
    for (const auto& entry: std::filesystem::directory_iterator{ path })
        if (entry.path().extension() == ".lua")
            result.push_back(entry.path().filename().string());
    return result;
}

// Helper that returns all entries of a directory (i.e. files or subdirs)
std::vector<std::string> collect_filenames(const std::filesystem::path& path)
{
    std::vector<std::string> result;
    for (const auto& entry: std::filesystem::directory_iterator{ path })
        result.push_back(entry.path().filename().string());
    return result;
}

TEST_CASE("serialize_sequence: test filename format", "[serialize_sequence]")
{
    REQUIRE_THROWS_AS(deserialize_sequence(temp_dir + "/sequence"), Error);

    Step step01{Step::type_action};
    step01.set_label("action");
    Step step02{Step::type_if};
    step02.set_label("if");
    Step step03{Step::type_action};
    step03.set_label("action");
    Step step04{Step::type_elseif};
    step04.set_label("elseif");
    Step step05{Step::type_action};
    step05.set_label("action");
    Step step06{Step::type_end};
    step06.set_label("end");
    Step step07{Step::type_while};
    step07.set_label("while");
    Step step08{Step::type_action};
    step08.set_label("action");
    Step step09{Step::type_end};
    step09.set_label("end");
    Step step10{Step::type_action};
    step10.set_label("action");

    Sequence sequence{"sequence"};
    sequence.push_back(step01);
    sequence.push_back(step02);
    sequence.push_back(step03);
    sequence.push_back(step04);
    sequence.push_back(step05);
    sequence.push_back(step06);
    sequence.push_back(step07);
    sequence.push_back(step08);
    sequence.push_back(step09);
    sequence.push_back(step10);

    REQUIRE_NOTHROW(serialize_sequence(temp_dir, sequence));

    std::vector<std::string> expect{
        task::sequence_lua_filename,
        "step_01_action.lua",
        "step_02_if.lua",
        "step_03_action.lua",
        "step_04_elseif.lua",
        "step_05_action.lua",
        "step_06_end.lua",
        "step_07_while.lua",
        "step_08_action.lua",
        "step_09_end.lua",
        "step_10_action.lua"
    };

    std::vector<std::string> actual = collect_lua_filenames(temp_dir + "/sequence");
    std::sort(actual.begin(), actual.end());

    REQUIRE(expect.size() == actual.size());
    REQUIRE(expect == actual);
}

TEST_CASE("serialize_sequence: loading nonexisting file", "[serialize_sequence]")
{
    // empty path
    REQUIRE_THROWS_AS(deserialize_sequence(""), Error);

    std::filesystem::create_directory(temp_dir);

    // folder 'sequence' does not exist
    REQUIRE_THROWS_AS(deserialize_sequence(temp_dir + "/sequence2"), Error);

    std::filesystem::create_directory(temp_dir + "/sequence2");
    // No steps found
    REQUIRE_THROWS_AS(deserialize_sequence(temp_dir + "/sequence2"), Error);

}

TEST_CASE("serialize_sequence: indentation level & type", "[serialize_sequence]")
{
    Step step01{Step::type_while};
    step01.set_label("while condition");
    Step step02{Step::type_action};
    step02.set_label("action");
    Step step03{Step::type_end};
    step03.set_label("while end");

    Sequence sequence{"This is a sequence"};
    sequence.push_back(step01);
    sequence.push_back(step02);
    sequence.push_back(step03);

    REQUIRE_NOTHROW(serialize_sequence(temp_dir, sequence));

    Sequence deserialize_seq = deserialize_sequence(temp_dir + "/This is a sequence");

    REQUIRE(not deserialize_seq.empty());
    REQUIRE(deserialize_seq.size() == 3);
    REQUIRE(deserialize_seq[0].get_indentation_level() == 0);
    REQUIRE(deserialize_seq[0].get_type() == Step::type_while);
    REQUIRE(deserialize_seq[1].get_indentation_level() == 1);
    REQUIRE(deserialize_seq[1].get_type() == Step::type_action);
    REQUIRE(deserialize_seq[2].get_indentation_level() == 0);
    REQUIRE(deserialize_seq[2].get_type() == Step::type_end);
}

TEST_CASE("serialize_sequence: default constructed Step", "[serialize_sequence]")
{
    Step step{ };

    Sequence sequence{ "BlueAsBlood" };
    sequence.push_back(step);

    REQUIRE_NOTHROW(serialize_sequence(temp_dir, sequence));
    REQUIRE_NOTHROW(deserialize_sequence(temp_dir + "/BlueAsBlood"));
}

TEST_CASE("serialize_sequence: sequence name escaping", "[serialize_sequence]")
{
    auto before = collect_filenames(temp_dir);

    Sequence sequence{ "A/\"sequence\"$<again>" };
    sequence.push_back(Step{ });
    REQUIRE_NOTHROW(serialize_sequence(temp_dir, sequence));

    auto after = collect_filenames(temp_dir);
    // Man do I hate C++, I just want to subtract one array from another
    // after = after - before;
    after.erase(std::remove_if(after.begin(), after.end(),
        [&before](std::string const& e) -> bool {
            for (auto const& b : before)
                if (e == b)
                    return true;
            return false;
        }),
        after.end());

    REQUIRE(after.size() == 1);
    REQUIRE(after[0] == "A$2f$22sequence$22$24$3cagain$3e"); // This is strictly speaking not required

    Sequence deserialize_seq = deserialize_sequence(temp_dir + "/" + after[0]);
    REQUIRE(sequence.get_label() == deserialize_seq.get_label());
}

TEST_CASE("serialize_sequence: sequence name escaping 2", "[serialize_sequence]")
{
    auto before = collect_filenames(temp_dir);

    // Unfortunately this is legal:
    // (Maybe labels of Step and Sequence should not allow control characters?)
    Sequence sequence{ "A\bbell" };

    sequence.push_back(Step{ });
    REQUIRE_NOTHROW(serialize_sequence(temp_dir, sequence));

    auto after = collect_filenames(temp_dir);
    after.erase(std::remove_if(after.begin(), after.end(),
        [&before](std::string const& e) -> bool {
            for (auto const& b : before)
                if (e == b)
                    return true;
            return false;
        }),
        after.end());

    REQUIRE(after.size() == 1);
    REQUIRE(after[0] == "A bell"); // This is strictly speaking not required

    Sequence deserialize_seq = deserialize_sequence(temp_dir + "/" + after[0]);
    // REQUIRE(sequence.get_label() == deserialize_seq.get_label());
    // Compare all chars but not the control char which is at index 1:
    REQUIRE(sequence.get_label().substr(0,1) == deserialize_seq.get_label().substr(0,1));
    REQUIRE(sequence.get_label().substr(2) == deserialize_seq.get_label().substr(2));
    // Control char shall be encoded as blank
    REQUIRE(deserialize_seq.get_label().substr(1, 1) == " ");
}

TEST_CASE("serialize_sequence: : simple step setup", "[serialize_sequence]")
{
    Step step_action{Step::type_action};
    step_action.set_script("a = preface .. 'Bob'");

    Sequence seq{ "test_sequence_with_simple_step_setup" };
    seq.push_back(step_action);
    seq.set_step_setup_script("preface = 'Alice calls '");

    serialize_sequence(temp_dir, seq);

    Sequence seq_deserialized = deserialize_sequence(temp_dir + "/test_sequence_with_simple_step_setup");

    REQUIRE(seq_deserialized.get_step_setup_script() == "preface = 'Alice calls '");
}

TEST_CASE("serialize_sequence: : complex step setup", "[serialize_sequence]")
{
    auto step_setup_script =
R"(
a = 'Bob'
        function test(name) return name .. ' with some funky stuff!'     end
b = test('Alice') \b\b  c = 4)";

    Step step_action{Step::type_action};
    step_action.set_script("a = preface .. 'Bob'");

    Sequence seq{ "test_sequence_with_complex_step_setup" };
    seq.push_back(step_action);
    seq.set_step_setup_script(step_setup_script);

    serialize_sequence(temp_dir, seq);

    Sequence seq_deserialized = deserialize_sequence(temp_dir + "/test_sequence_with_complex_step_setup");

    REQUIRE(seq_deserialized.get_step_setup_script() == step_setup_script);
}
