/**
 * \file   test_GitWrapper.cc
 * \author Sven-Jannik Woehnert
 * \date   Created on March 22, 2023
 * \brief  Test suite for the GitWrapper class.
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


#include <git2.h>
#include <gul14/catch.h>
#include "taskolib/GitWrapper.h"
#include "taskolib/serialize_sequence.h"
#include "taskolib/Step.h"
#include "taskolib/Sequence.h"
#include <filesystem>

using namespace task;



TEST_CASE("Construct LibGit object", "[GitWrapper]")
{
    // make sure directory is empty
    std::filesystem::remove_all("sequences");


    // prepare first sequence for test
    Step step_1_01{Step::type_while};
    step_1_01.set_label("while");
    step_1_01.set_script("return i < 10");

    Step step_1_02{Step::type_action};
    step_1_02.set_label("action");
    step_1_02.set_script("i = i + 1");

    Sequence seq_1{"unit_test_1"};
    seq_1.push_back(step_1_01);
    seq_1.push_back(step_1_02);

    store_sequence("sequences", seq_1);


    // Create Git Library
    LibGit gl{"sequences"};
    

    REQUIRE(not gl.get_path().empty());
    REQUIRE(gl.get_path() == "sequences");
    git_reference *temp;
    REQUIRE(git_repository_head(&temp, gl.get_repo()) == 0);
    
    
    // Test if repo_ got initialized
    REQUIRE(gl.get_last_commit_message() == "Initial commit");

}

TEST_CASE("Stage files", "[GitWrapper]")
{
    // Create Git Library
    LibGit gl{"sequences"};

    // prepare another sequence for test
    Step step_2_01{Step::type_while};
    step_2_01.set_label("while");
    step_2_01.set_script("return i < 10");

    Step step_2_02{Step::type_action};
    step_2_02.set_label("action");
    step_2_02.set_script("i = i + 1");

    Sequence seq_2{"unit_test_2"};
    seq_2.push_back(step_2_01);
    seq_2.push_back(step_2_02);

    store_sequence("sequences", seq_2);

    std::vector<std::array<std::string, 3>> stats = gl.libgit_status();

    for (size_t i =0; i < stats.size(); i++)
    {
        std::array<std::string, 3> elm = stats.at(i);
        if (elm[0] == "unit_test_2/step_1_while.lua")
        {
            REQUIRE(elm[1] == "unstaged");
            REQUIRE(elm[2] == "new file");
        }
    }


    gl.libgit_add();

    stats = gl.libgit_status();

    for (size_t i =0; i < stats.size(); i++)
    {
        std::array<std::string, 3> elm = stats.at(i);
        if (elm[0] == "unit_test_2/step_2_action.lua")
        {
            REQUIRE(elm[1] == "staged");
            REQUIRE(elm[2] == "new file");
        }
    }

}


TEST_CASE("Commit", "[GitWrapper]")
{
    // Create Git Library
    LibGit gl{"sequences"};
    
    
    // Check if repo_ can be found again
    REQUIRE(gl.get_last_commit_message() == "Initial commit");

    gl.libgit_add(); // add unit_test_2
    gl.libgit_commit("Add second sequence");

    REQUIRE(gl.get_last_commit_message() == "Add second sequence");

    auto stats = gl.libgit_status();

    for (size_t i =0; i < stats.size(); i++)
    {
        std::array<std::string, 3> elm = stats.at(i);
        REQUIRE(elm[1] != "staged");

    }

}


TEST_CASE("Delete Sequence", "[GitWrapper]")
{

    // Create Git Library
    LibGit gl{"sequences"};
    
    std::filesystem::path mypath = "unit_test_2";

    gl.libgit_remove_sequence(mypath);

    auto stats = gl.libgit_status();

    for (size_t i =0; i < stats.size(); i++)
    {
        std::array<std::string, 3> elm = stats.at(i);
        if (elm[0] == "unit_test_2/step_2_action.lua")
        {
            REQUIRE(elm[1] != "unstaged");
            REQUIRE(elm[2] != "deleted");
        }

    }

    gl.libgit_add();

    stats = gl.libgit_status();

    for (size_t i =0; i < stats.size(); i++)
    {
        std::array<std::string, 3> elm = stats.at(i);
        if (elm[0] == "unit_test_2/step_2_action.lua")
        {
            REQUIRE(elm[1] != "staged");
            REQUIRE(elm[2] != "deleted");
        }

    }

    // check if path got removed
    REQUIRE(not std::filesystem::exists("sequences" / mypath));

}