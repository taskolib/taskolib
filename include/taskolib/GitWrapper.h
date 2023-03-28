/**
 * \file   GitWrapper.h
 * \author Sven-Jannik Wöhnert
 * \date   Created on March 20, 2023
 * \brief  Wrapper for C-Package libgit2
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
#include <string>
#include <filesystem>
#include "taskolib/GitObjectWrapper.h"
#include <vector>

/**
 * A class to wrap used methods from C-Library libgit2
 * 
 * This class is necessary to handle C-pointers in a way that
 * C++ programmers using this class do not have to think about them.
 * 
 * This class does not reflect the full libgit2 library.
 * git functions which are not implemented in this class will be
 * implemented in case of necessity
*/
class LibGit
{
public:

    /**
     * Constructor which specifies the root dir of the git repository
     * \param file_path: path to "sequences"-folder (or customized directory)
     *
     */
    explicit LibGit(std::filesystem::path file_path);

    /// returns member variable, which is the root dir of the git repository
    std::filesystem::path get_path() const;

    /// returns a C-type pointer to the current loaded repository
    git_repository* get_repo();

    /// stage all new and changed files in the repository environment
    void libgit_add();

    /**
     * return the commit message of ther HEAD commit
     * \note this function is solely for Unittest purposes
     * \return message of last commit (=HEAD)
    */
    std::string get_last_commit_message() const;

    /**
     * Commit staged changes to the master branch of the git repository
     * \param commit_message: customized message for the commit
    */
    void libgit_commit(std::string commit_message);

    /**
     * Deletes seq_repository and all files within
     * \param seq_directory: directory holding sequence
     * 
     * \attention seq_directory is a relative path with repo_path_ as root
     *            if the sequence to be deleted is in "sequences/"
     *            then set seq_directory = SEQUENCE_NAME
     * 
    */
    void libgit_remove_sequence(std::filesystem::path seq_directory);

    /// returns current git
    std::vector<std::array<std::string, 3>> libgit_status() const;

    /// Destructor
    ~LibGit();

private:
    /// Pointer which holds all infos of the active repository
    git_repository *repo_{ nullptr };

    /// path to the repository (for taskomat .../sequences/)
    std::filesystem::path repo_path_;

    /// signature used in commits
    git_signature *my_signature_{ nullptr };

    /**
     * initialize a new git repository and commit all files in its path
     * 
     * \note This is a private member function because git repository init 
     *       should be done by an LibGit Object Initialization
    */
    void libgit_init(std::filesystem::path file_path);

    /**
     * Make the first commit. Function is called by libgit_init
     * Function slightly differs from libgit_commit because in this case,
     * there is no previous commit to dock on.
     * 
    */
    void libgit_commit_initial();

    /**
     * Get a specific commit
     * \param None: Get the Head commit
     * \param ref: Use Hex string identifier to find a commit
     * \param count: Jump back NUMBER of commits behind HEAD
     * 
     * \return C-type commit object
    */
    git_commit* libgit_get_commit(int count) const;
    git_commit* libgit_get_commit(const std::string& ref) const;
    git_commit* libgit_get_commit() const;

    /**
     * Translate all status information for each submodule into String
     * 
     * \param status: C-type status of all submodules from libgit
     * 
     * \return A vector of dynamic length which contains an array with the following values:
     *  :array[0] - path name. If the path changed this value will have the shape
     *              "OLD_NAME -> NEW_NAME"
     *  :array[1] - Handling status [unchanged, unstaged, staged, untracked, ignored]
     *  :array[2] - Change status [new file, deleted, renamed, typechanged, modified, unchanged, ignored, untracked]
    */
    std::vector<std::array<std::string, 3>> collect_status(git_status_list *status) const;

};