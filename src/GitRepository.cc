/**
 * \file   GitRepository.cc
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
#include "taskolib/GitRepository.h"
#include "taskolib/exceptions.h"
#include "gul14/cat.h"
#include <iostream>
#include <vector>

namespace task {


GitRepository::GitRepository(const std::filesystem::path& file_path)
{
    //init libgit library
    git_libgit2_init();
 
    //init member variables
    repo_path_ = file_path;

    // if repository does not exist
    repo_=repository_open(repo_path_);
    if (repo_.get() == nullptr) 
    {
        // initialize everything
        init(file_path);
    }
    else
    {
        // intialize the signature
        my_signature_ = signature_default(repo_.get());
        if (my_signature_.get() == nullptr) my_signature_ = signature_new("Taskomat", "taskomat@desy.de", 123456789, 0);
    }
}


GitRepository::~GitRepository()
{
    repo_.reset();
    my_signature_.reset();
    git_libgit2_shutdown();
}

std::string GitRepository::get_last_commit_message()
{
    const LibGitPointer<git_commit> commit = get_commit();
    return std::string(git_commit_message(commit.get()));
}

std::filesystem::path GitRepository::get_path() const
{
    return repo_path_;
}

git_repository* GitRepository::get_repo()
{
    return repo_.get();
}

void GitRepository::update()
{
    LibGitPointer<git_index> index{repository_index(repo_.get())};

    char *paths[] = {const_cast<char*>("*")};
    git_strarray array = {paths, 1};

    // update index to check for files
    git_index_update_all(index.get(), &array, nullptr, nullptr);
    git_index_write(index.get());
}


void GitRepository::init(std::filesystem::path file_path)
{
    // create repository
    //3rd argument: false so that .git folder is created in given path
    repo_ = repository_init(file_path.c_str(), false);
    if (repo_.get()==nullptr) throw task::Error("Git init failed.");

    // create signature
    my_signature_ = signature_default(repo_.get());
    if (my_signature_.get()==nullptr) signature_new("Taskomat", "taskomat@desy.de", 123456789, 0);

    // update files in directory
    update();

    // make initial commit
    commit_initial();
}


void GitRepository::commit_initial()
{
    // prepare gitlib data types
    LibGitPointer<git_index> index{repository_index(repo_.get())};
    git_oid tree_id, commit_id;
    
    // write tree
    repository_index(repo_.get());
	  git_index_write_tree(&tree_id, index.get());

    // get tree structure for commit
    LibGitPointer<git_tree> tree{tree_lookup(repo_.get(), tree_id)};

    // commit
    git_commit_create_v(
      &commit_id,
      repo_.get(),
      "HEAD",
      my_signature_.get(),
      my_signature_.get(),
      "UTF-8",
      "Initial commit",
      tree.get(),
      0
      );

}


void GitRepository::commit(const std::string& commit_message)
{
    //get HEAD commit
    LibGitPointer<git_commit> parent_commit = get_commit();
    const git_commit* raw_commit = parent_commit.get();

    //define types for commit call and get index
    LibGitPointer<git_index> index{repository_index(repo_.get())};
    git_oid tree_id, commit_id;
    
    // get tree
	git_index_write_tree(&tree_id, index.get());
    LibGitPointer<git_tree> tree{tree_lookup(repo_.get(), tree_id)};

    // create commit
    int error = git_commit_create(
        &commit_id,
        repo_.get(),
        "HEAD",
        my_signature_.get(),
        my_signature_.get(),
        "UTF-8",
        commit_message.c_str(),
        tree.get(),
        1,
        &raw_commit
    );

    if (error) throw task::Error("Cannot commit.");
}


void GitRepository::add()
{
    //load index of last commit
    LibGitPointer<git_index> gindex{repository_index(repo_.get())};

    char *paths[] = {const_cast<char*>("*")};
    git_strarray array = {paths, 1};

    //add all
    int error = git_index_add_all(gindex.get(), &array, GIT_INDEX_ADD_DEFAULT, nullptr, nullptr);
    if (error) throw task::Error("Cannot stage files.");

    // save addition
    git_index_write(gindex.get());
}


void GitRepository::remove_directory(std::filesystem::path seq_directory)
{
    //load index of last commit
    LibGitPointer<git_index> gindex{repository_index(repo_.get())};

    //remove files from directory
    int error = git_index_remove_directory(gindex.get(), seq_directory.c_str(), 0);
    if (error) throw task::Error("Cannot remove sequence files.");
    // remove sequence directory from git
    error = git_index_remove_bypath(gindex.get(), (std::string(seq_directory)+"/").c_str());
    if (error) throw task::Error("Cannot remove sequence directory.");

    git_index_write(gindex.get());
    //git_index_write_tree(&tree, gindex.get());

    // delete sequence
    std::filesystem::remove_all(repo_path_ / seq_directory);
}


LibGitPointer<git_commit> GitRepository::get_commit(int count)
{
    std::string ref = "HEAD^" + std::to_string(count);
    return get_commit(ref);
}
LibGitPointer<git_commit> GitRepository::get_commit(const std::string& ref)
{
    git_commit * commit;
    git_oid oid_parent_commit;

    // resolve HEAD into a SHA1
    int error = git_reference_name_to_id( &oid_parent_commit, repo_.get(), ref.c_str());
    if (error) throw task::Error("Cannot find HEAD of branch.");

    // find commit object by commit ID
    error = git_commit_lookup( &commit, repo_.get(), &oid_parent_commit );
    if (error) throw task::Error("Cannot find HEAD of branch.");

    return commit;
}
LibGitPointer<git_commit> GitRepository::get_commit()
{
    return get_commit(std::string{"HEAD"});
}

bool GitRepository::check_filestatus_for_unstaged(FileStatus& filestats, const git_status_entry* s)
{
    std::string wstatus = "";

    if (s->status & GIT_STATUS_WT_MODIFIED)
        wstatus = "modified";
    if (s->status & GIT_STATUS_WT_DELETED)
        wstatus = "deleted";
    if (s->status & GIT_STATUS_WT_RENAMED)
        wstatus = "renamed";
    if (s->status & GIT_STATUS_WT_TYPECHANGE)
        wstatus = "typechange";

    if (wstatus != "")
    {
        filestats.handling = "unstaged";
        filestats.changes = std::string{wstatus};


        const char *old_path = s->index_to_workdir->old_file.path;
        const char *new_path = s->index_to_workdir->new_file.path;

        if (old_path && new_path && strcmp(old_path, new_path))
            filestats.path_name = gul14::cat(old_path, " -> ", new_path);
        else
            filestats.path_name = old_path ? std::string{old_path} : std::string{new_path};
        
        return true;
    }
    return false;
}


bool GitRepository::check_filestatus_for_staged(FileStatus& filestats, const git_status_entry* s)
{
    std::string istatus = "";

    if (s->status & GIT_STATUS_INDEX_NEW)
        istatus = "new file";
    if (s->status & GIT_STATUS_INDEX_MODIFIED)
        istatus = "modified";
    if (s->status & GIT_STATUS_INDEX_DELETED)
        istatus = "deleted";
    if (s->status & GIT_STATUS_INDEX_RENAMED)
        istatus = "renamed";
    if (s->status & GIT_STATUS_INDEX_TYPECHANGE)
        istatus = "typechange";

    if (istatus != "")
    {
        
        filestats.handling = "staged";
        filestats.changes = std::string{istatus};

        const char *old_path = s->head_to_index->old_file.path;
        const char *new_path = s->head_to_index->new_file.path;

        if (old_path && new_path && strcmp(old_path, new_path))
            filestats.path_name = gul14::cat(old_path, " -> ", new_path);
        else
            filestats.path_name = old_path ? std::string{old_path} : std::string{new_path};
        
        return true;
    }
    return false;
}


std::vector<FileStatus> GitRepository::collect_status(LibGitPointer<git_status_list>& status) const
{
    // get number of submodules
    const size_t nr_entries = git_status_list_entrycount(status.get());

    // declare status holding vector for each submodule
    std::vector<FileStatus> return_array;
    FileStatus filestats;

    for (size_t i = 0; i < nr_entries; ++i)
    {
        const git_status_entry *s = nullptr;
        const char *old_path = nullptr;
        const char *new_path = nullptr;

        std::string istatus = "";
        std::string wstatus = "";

        s = git_status_byindex(status.get(), i);

        // list files which exists but are untouched since last commit
        //############################################################
        if (s->status == GIT_STATUS_CURRENT)
        {
            filestats.handling = "unchanged";
            filestats.changes = "unchanged";

            old_path = s->head_to_index->old_file.path;
            new_path = s->head_to_index->new_file.path;

            filestats.path_name = old_path ? std::string{old_path} : std::string{new_path};

            return_array.push_back(filestats);

            continue;
        }

        // list files which were touched but stil are unchanged
        //#####################################################
        
        if (check_filestatus_for_unstaged(filestats, s))
        {
            return_array.push_back(filestats);
            continue;
        }

        // list files which are staged for next commit
        //############################################

        
        if (check_filestatus_for_staged(filestats, s))
        {
            return_array.push_back(filestats);
            continue;
        }

        // list untracked files
        //######################
        if (s->status == GIT_STATUS_WT_NEW)
        {

            filestats.handling = "untracked";
            filestats.changes = "untracked";
            filestats.path_name = std::string(s->index_to_workdir->old_file.path);

            return_array.push_back(filestats);

            continue;
        }

        // list ignored files
        //####################
        if (s->status == GIT_STATUS_IGNORED) {

            filestats.handling = "ignored";
            filestats.changes = "ignored";
            filestats.path_name = std::string(s->index_to_workdir->old_file.path);

            return_array.push_back(filestats);

            continue;
        }

        delete [] old_path;
        delete [] new_path;
    }
    return return_array;
}

std::vector<FileStatus> GitRepository::status()
{
    // declare status options
    git_status_options status_opt = GIT_STATUS_OPTIONS_INIT;
    status_opt.flags =  GIT_STATUS_OPT_INCLUDE_UNTRACKED |          // untracked files
                        GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |     // untracked directories
                        GIT_STATUS_OPT_INCLUDE_UNMODIFIED |         // unmodified files
                        GIT_STATUS_OPT_INCLUDE_IGNORED;             // ignored files

    // fill C-type status pointer
    LibGitPointer<git_status_list> my_status{status_list_new(repo_.get(), status_opt)};
    if (my_status.get() == nullptr) throw task::Error("Cannot init status.");

    // translate status pointer to redable status information
    std::vector<FileStatus> status_arr = collect_status(my_status);
    
    return status_arr;
}


std::vector<int> GitRepository::add_files(const std::vector<std::filesystem::path>& filepaths)
{
    //load index of last commit
    LibGitPointer<git_index> gindex{repository_index(repo_.get())};

    size_t v_len = filepaths.size();

    //iterate and add every file bypath
    std::vector <int> error_list;
    for (size_t i = 0; i < v_len; i++)
    {
        int error = git_index_add_bypath(gindex.get(), filepaths[i].c_str());
        if (error) error_list.push_back(i);
    }

    // save addition
    git_index_write(gindex.get());

    return error_list;
}

} //namespace task