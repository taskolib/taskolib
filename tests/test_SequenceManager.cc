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
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include <gul14/catch.h>
#include <gul14/substring_checks.h>
#include <libgit4cpp/GitRepository.h>

#include "internals.h"
#include "serialize_sequence.h"
#include "taskolib/SequenceManager.h"


using namespace Catch::Matchers;
using namespace std::literals;
using namespace task;
using namespace task::literals;

namespace {

static const std::filesystem::path temp_dir{ "unit_test_files" };

// Helper that returns all entries of a directory (i.e. files or subdirs)
std::vector<std::string> collect_filenames(const std::filesystem::path& path)
{
    std::vector<std::string> result;
    for (const auto& entry: std::filesystem::directory_iterator{ path })
        result.push_back(entry.path().filename().string());

    std::sort(result.begin(), result.end());
    return result;
}

// Helper that returns all ".lua" entries of a directory (i.e. files or subdirs)
std::vector<std::string> collect_lua_filenames(const std::filesystem::path& path)
{
    std::vector<std::string> result;
    for (const auto& entry: std::filesystem::directory_iterator{ path })
        if (entry.path().extension() == ".lua")
            result.push_back(entry.path().filename().string());

    std::sort(result.begin(), result.end());
    return result;
}

} // anonymous namespace


TEST_CASE("SequenceManager: Constructor with path", "[SequenceManager]")
{
    std::filesystem::remove_all("./another/path/to/sequences");
    SequenceManager sm{"./another/path/to/sequences"};
    REQUIRE(sm.get_path() == "./another/path/to/sequences");
}

TEST_CASE("SequenceManager: Move constructor", "[SequenceManager]")
{
    SequenceManager s{SequenceManager("unit_test_files")};
    REQUIRE(s.get_path() == "unit_test_files");
}

TEST_CASE("SequenceManager: copy_sequence()", "[SequenceManager]")
{
    const char* dir = "unit_test_files/copy_sequence_test";

    if (std::filesystem::exists(dir))
        std::filesystem::remove_all(dir);

    std::filesystem::create_directories(dir);
    SequenceManager manager{ dir };

    // Create a sequence and store it
    Sequence seq{ "First sequence", SequenceName{ "first" } };
    Step step{ Step::type_action };
    step.set_script("answer = 42");
    seq.push_back(step);
    manager.store_sequence(seq);

    // Make a copy
    Sequence copy = manager.copy_sequence(seq.get_unique_id(), SequenceName("copy"));

    // Compare original and copy
    REQUIRE(copy.get_label() == "First sequence");
    REQUIRE(copy.get_name() == SequenceName{ "copy" });
    REQUIRE(copy.get_unique_id() != seq.get_unique_id());
    REQUIRE(copy.size() == seq.size());
    REQUIRE(copy[0].get_type() == seq[0].get_type());
    REQUIRE(copy[0].get_script() == seq[0].get_script());

    // Examine base folder
    auto list = manager.list_sequences();
    REQUIRE(list.size() == 2);
    REQUIRE(std::find_if(list.begin(), list.end(),
        [](const SequenceManager::SequenceOnDisk& s)
        {
            return s.name == SequenceName{ "first" };
        }) != list.end());
    REQUIRE(std::find_if(list.begin(), list.end(),
        [](const SequenceManager::SequenceOnDisk& s)
        {
            return s.name == SequenceName{ "copy" };
        }) != list.end());
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
    const std::string root = "unit_test_files/sequences";
    if (std::filesystem::exists(root))
        std::filesystem::remove_all(root);

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

    SequenceManager manager{ root };

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

    REQUIRE_THAT(sequences[0].path, StartsWith("A_legacy_sequence["));
    REQUIRE_THAT(sequences[0].path, EndsWith("]"));
    REQUIRE(sequences[0].name == SequenceName{ "A_legacy_sequence" });
    REQUIRE(sequences[0].unique_id != 0_uid);

    REQUIRE_THAT(sequences[1].path, StartsWith("test.seq.1["));
    REQUIRE_THAT(sequences[1].path, EndsWith("]"));
    REQUIRE(sequences[1].name == SequenceName{ "test.seq.1" });
    REQUIRE(sequences[1].unique_id != 0_uid);

    REQUIRE_THAT(sequences[2].path, StartsWith("test.seq.2["));
    REQUIRE_THAT(sequences[2].path, EndsWith("]"));
    REQUIRE(sequences[2].name == SequenceName{ "test.seq.2" });
    REQUIRE(sequences[2].unique_id != 0_uid);
}

TEST_CASE("SequenceManager: load_sequence() - Nonexistent unique ID", "[SequenceManager]")
{
    const char* dir = "unit_test_files/empty_sequence_folder";

    if (std::filesystem::exists(dir))
        std::filesystem::remove_all(dir);

    std::filesystem::create_directories(dir);

    SequenceManager manager{ dir };

    REQUIRE_THROWS_AS(manager.load_sequence(UniqueId{}), Error);
}

TEST_CASE("SequenceManager: remove_sequence()", "[SequenceManager]")
{
    const char* dir = "unit_test_files/remove_sequence_test";

    if (std::filesystem::exists(dir))
        std::filesystem::remove_all(dir);

    std::filesystem::create_directories(dir);
    SequenceManager manager{ dir };

    Sequence seq1 = manager.create_sequence("First sequence", SequenceName{ "first" });
    seq1.push_back(Step{ Step::type_action });
    manager.store_sequence(seq1);

    Sequence seq2 = manager.create_sequence("Second sequence", SequenceName{ "second" });

    manager.remove_sequence(seq1.get_unique_id());

    auto sequences = manager.list_sequences();
    REQUIRE(sequences.size() == 1);
    REQUIRE(sequences[0].name == SequenceName{ "second" });
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

    manager.rename_sequence(seq2.get_unique_id(), SequenceName{ "second_after_rename" });

    REQUIRE_THROWS_AS(manager.rename_sequence(0_uid, SequenceName{ "Bla" }), Error);

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

TEST_CASE("SequenceManager: store_sequence() - Filename format", "[SequenceManager]")
{
    SequenceManager manager{ temp_dir };

    const SequenceName seq_name{ "Test_sequence" };
    const UniqueId seq_uid{ 0xfeeddeafdeadbeef };
    const auto seq_folder = make_sequence_filename(seq_name, seq_uid);

    if (std::filesystem::exists(temp_dir / seq_folder))
        std::filesystem::remove_all(temp_dir / seq_folder);

    Sequence sequence{ "", seq_name, seq_uid };
    sequence.push_back(Step{ Step::type_action });
    sequence.push_back(Step{ Step::type_if });
    sequence.push_back(Step{ Step::type_action });
    sequence.push_back(Step{ Step::type_elseif });
    sequence.push_back(Step{ Step::type_action });
    sequence.push_back(Step{ Step::type_end });
    sequence.push_back(Step{ Step::type_while });
    sequence.push_back(Step{ Step::type_action });
    sequence.push_back(Step{ Step::type_end });
    sequence.push_back(Step{ Step::type_action });

    REQUIRE_NOTHROW(manager.store_sequence(sequence));

    std::vector<std::string> expect{
        sequence_lua_filename,
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

    std::vector<std::string> actual = collect_lua_filenames(temp_dir / seq_folder);
    std::sort(actual.begin(), actual.end());

    REQUIRE(expect.size() == actual.size());
    REQUIRE(expect == actual);
}

TEST_CASE("SequenceManager: store_sequence() & load_sequence() - Steps",
    "[SequenceManager]")
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

    Sequence load = sm.load_sequence(seq.get_unique_id());

    REQUIRE(load.get_label() == seq.get_label());
    REQUIRE(load.size() == seq.size());
    auto result = std::equal(load.begin(), load.end(), seq.begin(), check);
    REQUIRE(result);
}

TEST_CASE("SequenceManager: store_sequence() & load_sequence() - Label, name, UID",
    "[SequenceManager]")
{
    SequenceManager manager{ temp_dir };

    const auto label = "A/\"sequence\"$[<again>]"s;
    const SequenceName name{ "gabba-gabba_he.Y" };

    Sequence sequence{ label, name };
    sequence.push_back(Step{});

    const auto seq_folder = make_sequence_filename(sequence);

    if (std::filesystem::exists(temp_dir / seq_folder))
        std::filesystem::remove_all(temp_dir / seq_folder);

    auto before = collect_filenames(temp_dir);

    REQUIRE_NOTHROW(manager.store_sequence(sequence));

    auto after = collect_filenames(temp_dir);

    std::vector<std::string> new_filenames;
    std::set_difference(after.begin(), after.end(),
                        before.begin(), before.end(),
                        std::back_inserter(new_filenames));

    REQUIRE(new_filenames.size() == 1);
    REQUIRE(new_filenames[0] == seq_folder);

    Sequence deserialize_seq = manager.load_sequence(sequence.get_unique_id());
    REQUIRE(sequence.get_label() == deserialize_seq.get_label());
    REQUIRE(sequence.get_name() == deserialize_seq.get_name());
    REQUIRE(sequence.get_unique_id() == deserialize_seq.get_unique_id());
}

TEST_CASE("SequenceManager: store_sequence() & load_sequence() - Step setup",
    "[SequenceManager]")
{
    SequenceManager manager{ temp_dir };

    const auto step_setup_script =
        R"(
        a = 'Bob'
        function test(name)
            return name .. ' with some funky stuff!'
        end
        b = test('Alice') \t\b\b  c = 4)";

    Sequence seq{ "seq_with_complex_step_setup" };
    seq.push_back(Step{ Step::type_action });
    seq.set_step_setup_script(step_setup_script);

    manager.store_sequence(seq);

    Sequence seq_deserialized = manager.load_sequence(seq.get_unique_id());

    REQUIRE(seq_deserialized.get_step_setup_script() == step_setup_script);
}

TEST_CASE("SequenceManager: store_sequence() & load_sequence() - Indentation level & type",
    "[SequenceManager]")
{
    SequenceManager manager{ temp_dir };

    Sequence sequence{ "this_is_a_sequence" };
    sequence.push_back(Step{ Step::type_while });
    sequence.push_back(Step{ Step::type_action });
    sequence.push_back(Step{ Step::type_end });

    REQUIRE_NOTHROW(manager.store_sequence(sequence));

    Sequence deserialize_seq = manager.load_sequence(sequence.get_unique_id());

    REQUIRE(not deserialize_seq.empty());
    REQUIRE(deserialize_seq.size() == 3);
    REQUIRE(deserialize_seq[0].get_indentation_level() == 0);
    REQUIRE(deserialize_seq[0].get_type() == Step::type_while);
    REQUIRE(deserialize_seq[1].get_indentation_level() == 1);
    REQUIRE(deserialize_seq[1].get_type() == Step::type_action);
    REQUIRE(deserialize_seq[2].get_indentation_level() == 0);
    REQUIRE(deserialize_seq[2].get_type() == Step::type_end);
}

TEST_CASE("SequenceManager: store_sequence() & load_sequence() - Maintainers, timeout",
    "[SequenceManager]")
{
    SequenceManager manager{ temp_dir };

    Sequence seq{ "Test sequence with maintainers" };

    // remove previously stored sequence
    if (std::filesystem::exists(temp_dir / make_sequence_filename(seq)))
        std::filesystem::remove_all(temp_dir / make_sequence_filename(seq));

    seq.set_maintainers("John Doe john.doe@universe.org; Bob Smith boby@milkyway.edu");
    seq.set_timeout(task::Timeout{1min});

    manager.store_sequence(seq);

    Sequence seq_deserialized = manager.load_sequence(seq.get_unique_id());

    REQUIRE("John Doe john.doe@universe.org; Bob Smith boby@milkyway.edu"
        == seq_deserialized.get_maintainers());
    REQUIRE(task::Timeout{1min} == seq_deserialized.get_timeout());
    REQUIRE("Test sequence with maintainers" == seq_deserialized.get_label());
}

TEST_CASE("SequenceManager: store_sequence() & load_sequence() - Empty sequence",
    "[SequenceManager]")
{
    SequenceManager manager{ temp_dir };

    std::string uid{ "empty_seq_with_step_setup" };
    Sequence seq{ uid };

    SECTION("Deserialize empty sequence (part 1)")
    {
        manager.store_sequence(seq);
        REQUIRE_NOTHROW(manager.load_sequence(seq.get_unique_id()));
    }

    SECTION("Deserialize empty sequence (part 2)")
    {
        // Remove previously stored sequence
        if (std::filesystem::exists(temp_dir / make_sequence_filename(seq)))
            std::filesystem::remove_all(temp_dir / make_sequence_filename(seq));

        manager.store_sequence(seq);

        Sequence seq_deserialized = manager.load_sequence(seq.get_unique_id());
        REQUIRE(seq_deserialized.empty());
    }
}


TEST_CASE("SequenceManager: git repository", "[SequenceManager]")
{
    std::filesystem::path git_dir {"sequences_git"};

    SECTION("Create and store sequence")
    {
        // clean surrounding
        std::filesystem::remove_all(git_dir);

        // init manager
        SequenceManager manager{ git_dir};

        // create sequence
        Sequence seq{ "git Sequence 1",  SequenceName{ "git_sequence_1" }, UniqueId{ 0xabcdef123456 } };
        Step step_01{Step::type_while};
        step_01.set_label("while");
        step_01.set_script("return i < 10");
        seq.push_back(step_01);

        // store sequence and make git commit
        manager.store_sequence(seq);

        // unable to reach repo debug functions without a new function in sequenceManager
        // Therefore a standalone git handler has to be initialized
        git::GitRepository repo{git_dir};

        // check if commit was done
        const std::string expected_msg = "change sequence:\n- Add sequence.lua from new sequence 'git_sequence_1[0000abcdef123456]'\n- Add step_1_while.lua from new sequence 'git_sequence_1[0000abcdef123456]'";
        const auto last_msg = repo.get_last_commit_message();
        REQUIRE(expected_msg == last_msg);


        // use raw repository to access status
        const auto stats = repo.status();
        REQUIRE(stats.size() != 0);
        bool seq_exists = false;
        for(const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "git_sequence_1"))
            {
                seq_exists = true;

                // if staging did not work: 
                //      elm.handling == "untracked"
                //      elm.changes == "untracked"
                // if commit did not work
                //      elm.handling == "staged"
                //      elm.changes == "new file"
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");
            }
        }
        // check if created sequence is at least indexed by git
        REQUIRE(seq_exists);
    }

    SECTION("change and store sequence (step_setup)")
    {
        SequenceManager manager{ git_dir};

        // load sequence an change step setup
        auto seq = manager.load_sequence(UniqueId{ 0xabcdef123456 });
        const auto step_setup_script =
        R"(
        a = 'Bob'
        function test(name)
            return name .. ' with some funky stuff!'
        end
        b = test('Alice') \t\b\b  c = 4)";
        seq.set_step_setup_script(step_setup_script);

        // store sequence
        manager.store_sequence(seq);

        git::GitRepository repo{git_dir};

        const std::string expected_msg = "change sequence:\n- modify 'git_sequence_1[0000abcdef123456]/sequence.lua'";
        const auto last_msg = repo.get_last_commit_message();
        REQUIRE(expected_msg == last_msg);

        // use raw repository to access status
        const auto stats = repo.status();
        REQUIRE(stats.size() != 0);
        bool seq_exists = false;
        for(const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "git_sequence_1[0000abcdef123456]/sequence.lua"))
            {
                seq_exists = true;

                // if staging did not work: 
                //      elm.handling == "unstaged"
                //      elm.changes == "modified"
                // if commit did not work
                //      elm.handling == "staged"
                //      elm.changes == "modified"
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");
            }
        }
        // check if created sequence is at least indexed by git
        REQUIRE(seq_exists);
    }

    SECTION("copy sequence")
    {
        SequenceManager manager{ git_dir};

        // find sequence unique ID
        UniqueId uID{0};
        for (auto elm: manager.list_sequences())
        {
            if (elm.name == SequenceName{"git_sequence_1"}) uID = elm.unique_id;
        }
        REQUIRE(uID == UniqueId{0xabcdef123456});

        manager.copy_sequence(UniqueId{0xabcdef123456}, SequenceName{"git_sequence_2"});

        git::GitRepository repo{git_dir};

        // find sequence full name
        std::filesystem::path seq_name{""};
        for (auto elm: manager.list_sequences())
        {
            if (elm.name == SequenceName{"git_sequence_2"}) seq_name = elm.path;
        }
        REQUIRE(seq_name != "");

        // check commit message
        const std::string expected_msg = gul14::cat("copy sequence:\n- Add sequence.lua from new sequence '",
                                                     seq_name.string(), "'\n- Add step_1_while.lua from new sequence '",
                                                     seq_name.string(), "'");
        const auto last_msg = repo.get_last_commit_message();
        REQUIRE(expected_msg == last_msg);

        // use raw repository to access status
        const auto stats = repo.status();
        REQUIRE(stats.size() != 0);
        bool seq_exists = false;
        for(const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "git_sequence_2"))
            {
                seq_exists = true;

                // if staging did not work: 
                //      elm.handling == "untracked"
                //      elm.changes == "untracked"
                // if commit did not work
                //      elm.handling == "staged"
                //      elm.changes == "new file"
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");

            }
        }
        // check if created sequence is at least indexed by git
        REQUIRE(seq_exists);
    }

    SECTION("rename sequence")
    {
        SequenceManager manager{ git_dir};

        // find sequence unique ID and full name
        UniqueId uID{0};
        std::filesystem::path seq2{""};
        for (auto elm: manager.list_sequences())
        {
            if (elm.name == SequenceName{"git_sequence_2"})
            {
                uID = elm.unique_id;
                seq2 = elm.path;
            }
        }
        REQUIRE(uID != UniqueId{0});
        REQUIRE(seq2 != "");


        manager.rename_sequence(uID, SequenceName{"git_sequence_3"});

        // find sequence paths
        std::filesystem::path seq3{""};
        for (auto elm: manager.list_sequences())
        {
            
            if (elm.name == SequenceName{"git_sequence_3"}) seq3 = elm.path;
        }
        REQUIRE(seq3 != "");

        git::GitRepository repo{git_dir};

        const std::string expected_msg = gul14::cat("Rename git_sequence_2 to git_sequence_3:\n",
                                                     "- delete '", seq2.string(), "/sequence.lua'\n",
                                                     "- delete '", seq2.string(), "/step_1_while.lua'\n"
                                                     "- Add sequence.lua from new sequence '", seq3.string(), "'\n"
                                                     "- Add step_1_while.lua from new sequence '", seq3.string(), "'");
        const auto last_msg = repo.get_last_commit_message();
        REQUIRE(expected_msg == last_msg);

        // use raw repository to access status
        const auto stats = repo.status();
        REQUIRE(stats.size() != 0);
        bool seq_exists = false;
        for(const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "git_sequence_3"))
            {
                seq_exists = true;

                // if staging did not work: 
                //      elm.handling == "untracked"
                //      elm.changes == "untracked"
                // if commit did not work
                //      elm.handling == "staged"
                //      elm.changes == "new file"
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");

            }
        }
        // check if created sequence is at least indexed by git
        REQUIRE(seq_exists);

    }

    SECTION("remove sequence")
    {
        SequenceManager manager{ git_dir};

        manager.remove_sequence(UniqueId{ 0xabcdef123456 });

        // create object here so that manager.store_sequence throws error
        // if init repository did not work
        git::GitRepository repo{git_dir};

        const std::string expected_msg = gul14::cat("remove sequence:\n",
                                                     "- delete 'git_sequence_1[0000abcdef123456]/sequence.lua'\n",
                                                     "- delete 'git_sequence_1[0000abcdef123456]/step_1_while.lua'");
        const auto last_msg = repo.get_last_commit_message();
        REQUIRE(expected_msg == last_msg);

        // use raw repository to access status
        const auto stats = repo.status();
        bool seq_exists = false;
        for(const auto& elm: stats)
        {
            // if staging did not work: 
            //      elm.handling == "unstaged"
            //      elm.changes == "deleted"
            // if commit did not work
            //      elm.handling == "staged"
            //      elm.changes == "deleted"
            REQUIRE(! gul14::starts_with(elm.path_name, "git_sequence_1"));
        }
        // check if sequence got removed from index
        REQUIRE(! seq_exists);
    }

}
