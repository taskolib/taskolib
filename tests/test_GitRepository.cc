/**
 * \file   test_GitRepository.cc
 * \author Sven-Jannik Woehnert
 * \date   Created on March 22, 2023
 * \brief  Test suite for the GitRepository class.
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


#include <git2.h>
#include <gul14/catch.h>
#include "taskolib/GitRepository.h"
#include "taskolib/serialize_sequence.h"
#include "taskolib/Step.h"
#include "taskolib/Sequence.h"
#include <filesystem>
#include <fstream>

using namespace task;


/**
 *  Creates a directory and store files in it.
 * Filestructure: \n
 * sequences/     \n
 *      $name$/   \n
 *          file0.txt   << $msg$ /n file0  \n
 *          file1.txt   << $msg$ /n file1  \n
 *          ...
 * \param name name of the directory
 * \param nr_files number of files to be created
 * \param msg What to write to the file
 */
void create_testfiles(const std::filesystem::path& name, size_t nr_files, const std::string& msg)
{
    std::filesystem::path p = "sequences" / name;
    std::filesystem::create_directories(p);
    for (size_t i = 0; i < nr_files; i++)
    {
        // msg
        // file i
        std::ofstream f(p/gul14::cat("file", i, ".txt"));
        f << msg << gul14::cat("\nfile", i);
    }  
}


TEST_CASE("GitRepository Wrapper Test all", "[GitWrapper]")
{
    /**
     * Create files in a directory and then initialize the git repository within.
     * The initial commit must be empty as no staging has been done.
     * Check for the initialization of member variables
     * Check if a repository is created (HEAD exists)
     * Check if a comit was created (initial commit)
     * 
     */
    SECTION("Construct GitRepository object")
    {
        create_testfiles("unit_test_1", 2, "Construct");

        // Create Git Library
        GitRepository gl{"sequences"};

        REQUIRE(not gl.get_path().empty());
        REQUIRE(gl.get_path() == "sequences");
        git_reference *temp;
        REQUIRE(git_repository_head(&temp, gl.get_repo()) == 0);
        
        
        // Test if repo_ got initialized
        REQUIRE(gl.get_last_commit_message() == "Initial commit");
    }


    /**
     * Check the general stag function "add".
     * 1) Load existing repository (in contrast to initialize in first section)
     * 2) Create files after repository loading (in contrast to first section)
     * 3) Check if files appear as untracked
     * 4) Stage all files. Now the status should show 4 staged new files
     */
    SECTION("Stage files")
    {
        // Create Git Library
        GitRepository gl{"sequences"};

        create_testfiles("unit_test_2", 2, "Stage");

        std::vector<FileStatus> stats = gl.status();
        REQUIRE(stats.size() != 0);
        for (size_t i =0; i < stats.size(); i++)
        {
            FileStatus elm = stats.at(i);
            if (elm.path_name.rfind("unit_test_1", 0) == 0 || elm.path_name.rfind("unit_test_2", 0) == 0)
            {
                REQUIRE(elm.handling == "untracked");
                REQUIRE(elm.changes == "untracked");
            }
        }

        gl.add();

        stats = gl.status();

        // new submodules from unit_test_2 should be in stage mode
        REQUIRE(stats.size() != 0);
        for (size_t i =0; i < stats.size(); i++)
        {
            FileStatus elm = stats.at(i);
            if (elm.path_name.rfind("unit_test_1", 0) == 0 || elm.path_name.rfind("unit_test_2", 0) == 0)
            {
                REQUIRE(elm.handling == "staged");
                REQUIRE(elm.changes == "new file");
            }
        }
    }

    /**
     * Commit the previous staged files.
     * 1) Load repository. 4 files should still be staged
     * 2) Last commit should be the initial commit
     * 3) commit staged files. files should now be in status "unchanged"
     * 4) Check if commit message was set successful
     */
    SECTION("Commit")
    {

        // Create Git Library
        GitRepository gl{"sequences"};

        auto stats = gl.status();

        // new submodules from unit_test_2 should be still in stage mode
        REQUIRE(stats.size() != 0);
        for (size_t i =0; i < stats.size(); i++)
        {
            FileStatus elm = stats.at(i);
            if (elm.path_name.rfind("unit_test_1", 0) == 0 || elm.path_name.rfind("unit_test_2", 0) == 0)
            {
                REQUIRE(elm.handling == "staged");
                REQUIRE(elm.changes == "new file");
            }
        }
        
        // Check if repo_ can be found again
        REQUIRE(gl.get_last_commit_message() == "Initial commit");

        gl.commit("Add second sequence");

        stats = gl.status();
        REQUIRE(stats.size() != 0);
              for (size_t i =0; i < stats.size(); i++)
        {
            FileStatus elm = stats.at(i);
            if (elm.path_name.rfind("unit_test_1", 0) == 0 || elm.path_name.rfind("unit_test_2", 0) == 0)
            {
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");
            }
        }

        REQUIRE(gl.get_last_commit_message() == "Add second sequence");
    }


    /**
     * Change two files, but only stage one of them.
     * 1) Manipulate both files of unit_test_1
     * 2) check if status of them is modified, but unstaged
     * 3) stage file1 of unit_test_1
     * 4) file1 should be staged and file0 still be unstaged
    */
    SECTION("Add by path")
    {
        GitRepository gl{"sequences"};

        create_testfiles("unit_test_1", 2, "Add by path");

        std::vector<FileStatus> stats = gl.status();
        REQUIRE(stats.size() != 0);
        for (size_t i =0; i < stats.size(); i++)
        {
            FileStatus elm = stats.at(i);
            if (elm.path_name.rfind("unit_test_1/file", 0) == 0)
            {
                REQUIRE(elm.handling == "unstaged");
                REQUIRE(elm.changes == "modified");
            }
            else if (elm.path_name.rfind("unit_test_2/file", 0) == 0)
            {
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");
            }
        }

        auto ret = gl.add_files({"unit_test_1/file1.txt"});

        // No errors should have occur 
        REQUIRE(ret.size() == 0);

        stats = gl.status();
        REQUIRE(stats.size() != 0);
        for (size_t i =0; i < stats.size(); i++)
        {
            FileStatus elm = stats.at(i);
            if (elm.path_name.rfind("unit_test_1/file0", 0) == 0)
            {
                REQUIRE(elm.handling == "unstaged");
                REQUIRE(elm.changes == "modified");
            }
            else if (elm.path_name.rfind("unit_test_1/file1", 0) == 0)
            {
                REQUIRE(elm.handling == "staged");
                REQUIRE(elm.changes == "modified");
            }
            else if (elm.path_name.rfind("unit_test_2/file", 0) == 0)
            {
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");
            }
        }
    }

    /**
     * remove a directory and check if the repository status notices
     * 1) Delete unit_test_2
     * 2) Files should be automatically staged for deletion
     * 3) Commit removal
     * 4) check if files are gone from status
     * 5) check if files are gone from filesystem
     */
    SECTION("Delete Directory")
    {
        // Create Git Library
        GitRepository gl{"sequences"};
        
        std::filesystem::path mypath = "unit_test_2";

        gl.remove_directory(mypath);

        std::vector<FileStatus> stats = gl.status();

        // every file in unit_test_2 should have the tag deleted
        REQUIRE(stats.size() != 0);
        for (size_t i =0; i < stats.size(); i++)
        {
            
            FileStatus elm = stats.at(i);
            if (elm.path_name.rfind("unit_test_2", 0) == 0)
            {
                REQUIRE(elm.handling == "staged");
                REQUIRE(elm.changes == "deleted");
            }
        }

        gl.commit("remove sequence");

        stats = gl.status();

        // check if files are removed from repository status
        REQUIRE(stats.size() != 0);
        for (size_t i =0; i < stats.size(); i++)
        {
            FileStatus elm = stats.at(i);
            REQUIRE (elm.path_name.rfind("unit_test_2/file", 0) != 0);
        }

        // check if path got removed
        REQUIRE(not std::filesystem::exists("sequences" / mypath));

        std::filesystem::remove_all("sequences");
    }
}