/**
 * \file   test_SequenceManager.cc
 * \author Marcus Walla
 * \date   Created on July 22, 2022
 * \brief  Test suite for the SequenceManager class.
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

#include <gul14/catch.h>
#include <utility>
#include <algorithm>
#include <fstream>
#include "taskolib/SequenceManager.h"
#include "taskolib/serialize_sequence.h"

using namespace task;

TEST_CASE("Construct SequenceManager with path", "[SequenceManager]")
{
    SequenceManager sm{"./another/path/to/sequences"};

    SECTION("Another path to sequences")
    {
        REQUIRE(not sm.get_path().empty());
        REQUIRE(sm.get_path() == "./another/path/to/sequences");
    }

    std::filesystem::remove_all("another");
}

TEST_CASE("Move SequenceManager constructor", "[SequenceManager]")
{
    SequenceManager s{SequenceManager("unit_test")};
    REQUIRE(not s.get_path().empty());
    REQUIRE(s.get_path() == "unit_test");

    std::filesystem::remove_all("unit_test");
}

TEST_CASE("Get sequence names", "[SequenceManager]")
{
    // prepare first sequence for test
    Step step_1_01{Step::type_while};
    step_1_01.set_label("while");
    step_1_01.set_script("return i < 10");

    Step step_1_02{Step::type_action};
    step_1_02.set_label("action");
    step_1_02.set_script("i = i + 1");

    Sequence seq_1{"test.seq.1"};
    seq_1.push_back(step_1_01);
    seq_1.push_back(step_1_02);

    // prepare second sequence for test
    Step step_2_01{Step::type_while};
    step_2_01.set_label("while");
    step_2_01.set_script("return i < 10");

    Step step_2_02{Step::type_action};
    step_2_02.set_label("action");
    step_2_02.set_script("i = i + 1");

    Sequence seq_2{"test.seq.2"};
    seq_2.push_back(step_1_01);
    seq_2.push_back(step_1_02);

    store_sequence("unit_test_2", seq_1);
    store_sequence("unit_test_2", seq_2);

    // create regular file to test sequence directory loading only.
    std::fstream f("unit_test_2/some_text_file.txt", std::ios::out);
    f.close();

    SequenceManager sm{"unit_test_2"};
    auto sequence_paths = sm.get_sequence_names();

    REQUIRE(sequence_paths.size() == 2);
    REQUIRE(sequence_paths[0] == "unit_test_2/test.seq.1");
    REQUIRE(sequence_paths[1] == "unit_test_2/test.seq.2");

    // check for loading sequence directories only (and not any arbritray regular file)
    REQUIRE_THROWS_AS(sm.load_sequence("unit_test_2/some_text_file.txt"), Error);

    std::filesystem::remove_all("unit_test_2");
}

TEST_CASE("Load sequence", "[SequenceManager]")
{
    const std::string sequence_name{"test.seq"};

    // prepare some sequence for the test
    Step step_01{Step::type_while};
    step_01.set_label("while");
    step_01.set_script("return i < 10");

    Step step_02{Step::type_action};
    step_02.set_label("action");
    step_02.set_script("i = i + 1");

    Sequence seq{sequence_name};
    seq.push_back(step_01);
    seq.push_back(step_02);

    auto check = [](const Step& lhs, const Step& rhs)
    {
        bool r = lhs.get_type() == rhs.get_type()
            and lhs.get_indentation_level() == rhs.get_indentation_level()
            and lhs.get_label() == rhs.get_label()
            and lhs.get_script() == rhs.get_script();
        if (not r)
            WARN(gul14::cat("Step '", lhs.get_label(), "' is unequal."));
        return r;
    };

    SECTION("Simple path")
    {
        // store sequence to the default path '.'
        store_sequence(".", seq);

        SequenceManager sm{"."};
        Sequence load = sm.load_sequence(sequence_name);

        REQUIRE(load.get_label() == seq.get_label());
        REQUIRE(load.size() == seq.size());
        auto result = std::equal(load.begin(), load.end(), seq.begin(), check);
        REQUIRE(result);
    }

    SECTION("Complex path")
    {
        // store sequence to the default path 'unit_test'
        store_sequence("unit_test", seq);

        SequenceManager sm{"unit_test"};
        Sequence load = sm.load_sequence(sequence_name);

        REQUIRE(load.get_label() == seq.get_label());
        REQUIRE(load.size() == seq.size());
        auto result = std::equal(load.begin(), load.end(), seq.begin(), check);
        REQUIRE(result);

        std::filesystem::remove_all("unit_test");
        std::filesystem::remove_all("test.seq");
    }

}


TEST_CASE("Remove Repository", "[SequenceManager]")
{
    // prepare first sequence for test
    Step step_1_01{Step::type_while};
    step_1_01.set_label("while");
    step_1_01.set_script("return i < 10");

    Step step_1_02{Step::type_action};
    step_1_02.set_label("action");
    step_1_02.set_script("i = i + 1");

    Sequence seq_1{"unit_test_3"};
    seq_1.push_back(step_1_01);
    seq_1.push_back(step_1_02);

    // prepare second sequence for test
    Step step_2_01{Step::type_while};
    step_2_01.set_label("while");
    step_2_01.set_script("return i < 10");

    Step step_2_02{Step::type_action};
    step_2_02.set_label("action");
    step_2_02.set_script("i = i + 1");

    Sequence seq_2{"unit_test_4"};
    seq_2.push_back(step_1_01);
    seq_2.push_back(step_1_02);

    store_sequence("sequences", seq_1);
    store_sequence("sequences", seq_2);

    //SequenceManager sm{"sequences"};

    // TODO: fix remove_all_sequences_and_repository

    // remove everything and come back to git with one initial commit
    //sm.remove_all_sequences_and_repository();

    std::filesystem::remove_all("sequences");

}