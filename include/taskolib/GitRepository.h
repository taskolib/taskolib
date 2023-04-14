/**
 * \file   GitRepository.h
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

namespace task {

/**
 * Struct to express the git status for one file
 */
struct FileStatus
{
    std::string path_name; ///relative path to file. If the path changed this value will have the shape "OLD_NAME -> NEW_NAME".
    std::string handling;  ///Handling status of file [unchanged, unstaged, staged, untracked, ignored]
    std::string changes;   ///Change status of file [new file, deleted, renamed, typechanged, modified, unchanged, ignored, untracked]
};

/**
 * A class to wrap used methods from C-Library libgit2.
 * 
 * This class is necessary to handle C-pointers in a way that
 * C++ programmers using this class do not have to think about them.
 * 
 * This class does not reflect the full libgit2 library.
 * git functions which are not implemented in this class will be
 * implemented in case of necessity
 */
class GitRepository
{
public:

    /**
     * Constructor which specifies the root dir of the git repository.
     * \param file_path: path to git directory
     */
    explicit GitRepository(const std::filesystem::path& file_path);

    /// returns member variable, which is the root dir of the git repository.
    std::filesystem::path get_path() const;

    /// returns a C-type pointer to the current loaded repository.
    git_repository* get_repo();

    /// stage all new and changed files and folders in the repository directory.
    void add();

    /**
     * Stage specific files listed in filepaths.
     * \param filepaths: list of files. Either relative to repository root or absolute.
     * \return list of indices. An index from the filepaths vector is returned if
     *          the staging of the file failed. Returns an empty vector if all
     *          files were staged successfully.
     */
   std::vector <int> add_files(const std::vector<std::filesystem::path>& filepaths);

    /**
     * return the commit message of the HEAD commit.
     * \note this function is solely for Unittest purposes
     * \return message of last commit (=HEAD)
     */
    std::string get_last_commit_message();

    /**
     * Commit staged changes to the master branch of the git repository.
     * \param commit_message: customized message for the commit
     */
    void commit(const std::string& commit_message);

    /**
     * Deletes seq_repository and all files within.
     * \param directory: directory holding sequence
     * \attention directory is a relative path within repo_path_
     */
    void remove_directory(std::filesystem::path directory);

    /**
     * returns current git status.
     * \return vector of file status for each file.
     */ 
    std::vector<FileStatus> status();

    /// Destructor
    ~GitRepository();

private:
    /// Pointer which holds all infos of the active repository.
    LibGitPointer<git_repository> repo_;

    /// path to the repository.
    std::filesystem::path repo_path_;

    /// signature used in commits.
    LibGitPointer<git_signature> my_signature_;

    /**
     * initialize a new git repository and commit all files in its path.
     * \note This is a private member function because git repository init 
     *       should be done by an LibGit Object Initialization
     */
    void init(std::filesystem::path file_path);

    /**
     * Make the first commit. Function is called by init.
     * \note slightly differs from commit because in this case,
     *       there is no previous commit to dock on.
    */
    void commit_initial();

    /**
     * Get a specific commit.
     * \param None: Get the Head commit
     * \param ref: Use Hex string identifier to find a commit
     * \param count: Jump back NUMBER of commits behind HEAD
     * 
     * \return C-type commit object
    */
    LibGitPointer<git_commit> get_commit(int count);
    LibGitPointer<git_commit> get_commit(const std::string& ref);
    LibGitPointer<git_commit> get_commit();

    /**
     * Translate all status information for each submodule into String.
     * 
     * \param status: C-type status of all submodules from libgit
     * 
     * \return A vector of dynamic length which contains a status struct 
    */
    std::vector<FileStatus> collect_status(LibGitPointer<git_status_list>& status) const;

    /** Update the tracked files in the repository.
     * This member function stages all changes of already tracked
     * files in the repository.
    */
    void update();

    /**
     * check if the file from the status entry is not staged and collect the status in filestats.
     * \note this function is solely called by collect_status. It has no other purpose.
     * \param filestats a structure which will be filled with the status of the file if the file is unstaged
     * \param s status of the file. A struct which contains flags about every possible state
     * \return true if file is unstaged (and filestats initialized), else false
     */
    static bool check_filestatus_for_unstaged(FileStatus& filestats, const git_status_entry* s);

    /**
     * check if the file from the status entry is not unstaged and collect the status in filestats.
     * \note this function is solely called by collect_status. It has no other purpose.
     * \param filestats a structure which will be filled with the status of the file if the file is staged
     * \param s status of the file. A struct which contains flags about every possible state
     * \return true if file is staged (and filestats initialized), else false
     */
    static bool check_filestatus_for_staged(FileStatus& filestats, const git_status_entry* s);

};

} // namespace task