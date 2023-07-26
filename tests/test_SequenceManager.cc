/**
 * \file   test_SequenceManager.cc
 * \author Marcus Walla, Lars Fröhlich
 * \date   Created on July 22, 2022
 * \brief  Test suite for the SequenceManager class.
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
#include <fstream>
#include <utility>

#include <gul14/catch.h>

#include "taskolib/SequenceManager.h"
#include "taskolib/serialize_sequence.h"

using namespace Catch::Matchers;
using namespace task;
using namespace task::literals;

TEST_CASE("SequenceManager: Constructor with path", "[SequenceManager]")
{
    SequenceManager sm{"./another/path/to/sequences"};
    REQUIRE(sm.get_path() == "./another/path/to/sequences");
}

TEST_CASE("SequenceManager: Move constructor", "[SequenceManager]")
{
    SequenceManager s{SequenceManager("unit_test_files")};
    REQUIRE(s.get_path() == "unit_test_files");
}

TEST_CASE("SequenceManager: create_sequence()", "[SequenceManager]")
{
    const char* dir = "unit_test_files/create_sequence_test";

    std::filesystem::create_directories(dir);
    SequenceManager manager{ dir };

    // Create sequence with empty label and random UID
    Sequence seq = manager.create_sequence();
    REQUIRE(seq.empty());
    REQUIRE(seq.get_label().empty());
    REQUIRE(seq.get_name() == SequenceName{ "" });

    // Create sequence with some label and random UID
    seq = manager.create_sequence("23rd & 3rd");
    REQUIRE(seq.empty());
    REQUIRE(seq.get_label() == "23rd & 3rd");
    REQUIRE(seq.get_name() == SequenceName{ "" });

    // Create sequence with non-empty label and selected sequence name
    seq = manager.create_sequence("Hey ho let's go", SequenceName{ "hey_gabba_gabba" });
    REQUIRE(seq.empty());
    REQUIRE(seq.get_label() == "Hey ho let's go");
    REQUIRE(seq.get_name() == SequenceName{ "hey_gabba_gabba" });

    // Create second sequence with same name
    auto seq2 = manager.create_sequence("We want the airwaves",
        SequenceName{ "hey_gabba_gabba" });
    REQUIRE(seq2.empty());
    REQUIRE(seq2.get_label() == "We want the airwaves");
    REQUIRE(seq2.get_name() == SequenceName{ "hey_gabba_gabba" });

    REQUIRE(seq.get_unique_id() != seq2.get_unique_id());

    REQUIRE(manager.list_sequences().size() == 4);
}

TEST_CASE("SequenceManager: list_sequences()", "[SequenceManager]")
{
    // prepare first sequence for test
    Step step_1_01{Step::type_while};
    step_1_01.set_label("while");
    step_1_01.set_script("return i < 10");

    Step step_1_02{Step::type_action};
    step_1_02.set_label("action");
    step_1_02.set_script("i = i + 1");

    Sequence seq_1{ "Test sequence 1", SequenceName{ "test.seq.1" } };
    seq_1.push_back(step_1_01);
    seq_1.push_back(step_1_02);

    // prepare second sequence for test
    Step step_2_01{Step::type_while};
    step_2_01.set_label("while");
    step_2_01.set_script("return i < 10");

    Step step_2_02{Step::type_action};
    step_2_02.set_label("action");
    step_2_02.set_script("i = i + 1");

    Sequence seq_2{ "Test sequence 2", SequenceName{ "test.seq.2" } };
    seq_2.push_back(step_1_01);
    seq_2.push_back(step_1_02);


    const std::string root = "unit_test_files/sequences";
    SequenceManager manager{ root };

    if (std::filesystem::exists(root))
        std::filesystem::remove_all(root);

    manager.store_sequence(seq_1);
    manager.store_sequence(seq_2);

    // create a regular file to test sequence directory loading only
    std::fstream f(root + "/some_weirdo_file[1234567890abcdef]", std::ios::out);
    f.close();

    // create a ".git" repository to test that it is not listed as a sequence
    std::filesystem::create_directory(root + "/.git");

    // create an empty directory with a non-sequence name (should be ignored)
    std::filesystem::create_directory(root + "/some_weirdo_folder[x1234567890abcdef]");

    // create a sequence directory without a unique ID, but with a sequence.lua file
    std::filesystem::create_directory(root + "/A legacy sequence");
    std::fstream f2(root + "/A legacy sequence/sequence.lua", std::ios::out);
    f2.close();


    SequenceManager sm{ root };
    auto sequences = sm.list_sequences();
    std::sort(sequences.begin(), sequences.end(),
        [](const SequenceManager::SequenceOnDisk& a, const SequenceManager::SequenceOnDisk& b)
        {
            return a.path < b.path;
        });

    REQUIRE(sequences.size() == 3);

    REQUIRE_THAT(sequences[0].path, StartsWith(root + "/A_legacy_sequence["));
    REQUIRE_THAT(sequences[0].path, EndsWith("]"));
    REQUIRE(sequences[0].name == SequenceName{ "A_legacy_sequence" });
    REQUIRE(sequences[0].unique_id != 0_uid);

    REQUIRE_THAT(sequences[1].path, StartsWith(root + "/test.seq.1["));
    REQUIRE_THAT(sequences[1].path, EndsWith("]"));
    REQUIRE(sequences[1].name == SequenceName{ "test.seq.1" });
    REQUIRE(sequences[1].unique_id != 0_uid);

    REQUIRE_THAT(sequences[2].path, StartsWith(root + "/test.seq.2["));
    REQUIRE_THAT(sequences[2].path, EndsWith("]"));
    REQUIRE(sequences[2].name == SequenceName{ "test.seq.2" });
    REQUIRE(sequences[2].unique_id != 0_uid);

    // check for loading sequence directories only (and not any arbritrary regular file)
    REQUIRE_THROWS_AS(sm.load_sequence(root + "/some_weirdo_file[1234567890abcdef]"), Error);
}

TEST_CASE("SequenceManager: load_sequence()", "[SequenceManager]")
{
    // prepare some sequence for the test
    Step step_01{Step::type_while};
    step_01.set_label("while");
    step_01.set_script("return i < 10");

    Step step_02{Step::type_action};
    step_02.set_label("action");
    step_02.set_script("i = i + 1");

    Sequence seq{ "Test sequence" };
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

    SequenceManager sm{ "unit_test_files" };
    sm.store_sequence(seq);

    Sequence load = sm.load_sequence(make_sequence_filename(seq));

    REQUIRE(load.get_label() == seq.get_label());
    REQUIRE(load.size() == seq.size());
    auto result = std::equal(load.begin(), load.end(), seq.begin(), check);
    REQUIRE(result);
}

TEST_CASE("SequenceManager: rename_sequence()", "[SequenceManager]")
{
    const char* dir = "unit_test_files/relabel_sequence_test";

    std::filesystem::create_directories(dir);
    SequenceManager manager{ dir };

    Sequence seq1 = manager.create_sequence("First sequence", SequenceName{ "first" });
    Sequence seq2 = manager.create_sequence("Second sequence", SequenceName{ "second" });

    manager.rename_sequence(seq1, SequenceName{ "first_after_rename" });
    REQUIRE(seq1.empty());
    REQUIRE(seq1.get_name() == SequenceName{ "first_after_rename" });

    manager.rename_sequence(SequenceName{ "second" }, seq2.get_unique_id(),
        SequenceName{ "second_after_rename" });

    REQUIRE_THROWS_AS(manager.rename_sequence(SequenceName{ "inexistent" }, 0_uid,
        SequenceName{ "Bla" }), Error);

    auto sequences = manager.list_sequences();
    std::sort(sequences.begin(), sequences.end(),
        [](const SequenceManager::SequenceOnDisk& a, const SequenceManager::SequenceOnDisk& b)
        {
            return a.path < b.path;
        });

    REQUIRE(sequences.size() == 2);
    REQUIRE(sequences[0].name.string() == "first_after_rename");
    REQUIRE(sequences[0].unique_id == seq1.get_unique_id());
    REQUIRE(sequences[1].name.string() == "second_after_rename");
    REQUIRE(sequences[1].unique_id == seq2.get_unique_id());
}
