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

GitRepository::GitRepository(std::filesystem::path file_path)
{
    //init libgit library
    git_libgit2_init();

    
    //init member variables
    repo_path_ = file_path;

    // if repository does not exist
    if (git_repository_open(repo_.getptr(), (repo_path_).c_str())) 
    {
        // initialize everything
        libgit_init(file_path);
    }
    else
    {
        // update status of all files 
        libgit_update();

        // intialize the signature
        int error = git_signature_default(my_signature_.getptr(), repo_.get());
        if (error==GIT_ENOTFOUND) git_signature_new(my_signature_.getptr(), "Taskomat", "taskomat@desy.de", 123456789, 0);
    }
    

}



GitRepository::~GitRepository()
{
    repo_.reset();
    git_libgit2_shutdown();
}

std::string GitRepository::get_last_commit_message()
{
    const git_commit *commit = libgit_get_commit();
    return std::string(git_commit_message(commit));
}

std::filesystem::path GitRepository::get_path() const
{
    return repo_path_;
}

git_repository* GitRepository::get_repo()
{
    return repo_.get();
}

void GitRepository::libgit_update()
{
  GitIndexPtr index;

  char *paths[] = {const_cast<char*>("*")};
  git_strarray array = {paths, 1};

  // update index to check for files
  git_repository_index(index.getptr(), repo_.get());
  git_index_update_all(index.get(), &array, nullptr, nullptr);
  git_index_write(index.get());
}


void GitRepository::libgit_init(std::filesystem::path file_path)
{
    // create repository
    //3rd argument: false so that .git folder is created in given path
    int error = git_repository_init(repo_.getptr(), file_path.c_str(), false);
    if (error) throw task::Error(gul14::cat("Git init failed."));

    // create signature
    error = git_signature_default(my_signature_.getptr(), repo_.get());
    if (error==GIT_ENOTFOUND) git_signature_new(my_signature_.getptr(), "Taskomat", "taskomat@desy.de", 123456789, 0);

    // update files in directory
    libgit_update();

    // make initial commit
    libgit_commit_initial();
}




void GitRepository::libgit_commit_initial()
{
    // prepare gitlib data types
    GitIndexPtr index;
    git_oid tree_id, commit_id;
    GitTreePtr tree;

    // get tree structure for commit
    git_repository_index(index.getptr(), repo_.get());
	  git_index_write_tree(&tree_id, index.get());
    git_tree_lookup(tree.getptr(), repo_.get(), &tree_id);

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


void GitRepository::libgit_commit(const std::string& commit_message)
{

    //get HEAD commit
    const git_commit* parent_commit = libgit_get_commit();

    //define types for commit call
    GitIndexPtr index;
    git_oid tree_id, commit_id;
    GitTreePtr tree;

    // get tree
    git_repository_index(index.getptr(), repo_.get());
	  git_index_write_tree(&tree_id, index.get());
    git_tree_lookup(tree.getptr(), repo_.get(), &tree_id);

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
        &parent_commit
    );

    if (error) throw task::Error(gul14::cat("Cannot commit."));

}


void GitRepository::libgit_add()
{
    //load index of last commit
    GitIndexPtr gindex;
    git_repository_index(gindex.getptr(), repo_.get());

    char *paths[] = {const_cast<char*>("*")};
    git_strarray array = {paths, 1};

    //add all
    int error = git_index_add_all(gindex.get(), &array, GIT_INDEX_ADD_DEFAULT, nullptr, nullptr);
    if (error) throw task::Error(gul14::cat("Cannot stage files."));

    // save addition
    git_index_write(gindex.get());

}


void GitRepository::libgit_remove_sequence(std::filesystem::path seq_directory)
{
    //load index of last commit
    GitIndexPtr gindex;
    git_repository_index(gindex.getptr(), repo_.get());

    //remove files from directory
    int error = git_index_remove_directory(gindex.get(), seq_directory.c_str(), 0);
    if (error) throw task::Error(gul14::cat("Cannot remove sequence files."));
    // remove sequence directory from git
    error = git_index_remove_bypath(gindex.get(), (std::string(seq_directory)+"/").c_str());
    if (error) throw task::Error(gul14::cat("Cannot remove sequence directory."));

    git_index_write(gindex.get());
    //git_index_write_tree(&tree, gindex.get());

    // delete sequence
    std::filesystem::remove_all(repo_path_ / seq_directory);

}



git_commit* GitRepository::libgit_get_commit(int count)
{
    std::string ref = "HEAD^" + std::to_string(count);
    return libgit_get_commit(ref);
}
git_commit* GitRepository::libgit_get_commit(const std::string& ref)
{
    git_commit * commit;
    git_oid oid_parent_commit;

    // resolve HEAD into a SHA1
    int error = git_reference_name_to_id( &oid_parent_commit, repo_.get(), ref.c_str());
    if (error) throw task::Error(gul14::cat("Cannot find HEAD of branch."));

    // find commit object by commit ID
    error = git_commit_lookup( &commit, repo_.get(), &oid_parent_commit );
    if (error) throw task::Error(gul14::cat("Cannot find HEAD of branch."));

    return commit;

}
git_commit* GitRepository::libgit_get_commit()
{
    return libgit_get_commit(std::string{"HEAD"});

}


std::vector<std::array<std::string, 3>> GitRepository::collect_status(git_status_list *status) const
{
  // get number of submodules
  const size_t nr_entries = git_status_list_entrycount(status);

  // declare status holding vector for each submodule
  std::vector<std::array<std::string, 3>> return_array; //TODO: struct statt array
  std::array<std::string, 3> elm;

  for (size_t i = 0; i < nr_entries; ++i)
  {
    const git_status_entry *s = nullptr;
    const char *old_path = nullptr;
    const char *new_path = nullptr;

    std::string istatus = "";
    std::string wstatus = "";

    s = git_status_byindex(status, i);

    // list files which exists but are untouched since last commit
    //############################################################
    if (s->status == GIT_STATUS_CURRENT)
    {
      elm[1] = "unchanged";
      elm[2] = "unchanged";

      old_path = s->head_to_index->old_file.path;
      new_path = s->head_to_index->new_file.path;

      elm[0] = old_path ? std::string{old_path} : std::string{new_path};

      return_array.push_back(elm);

      continue;
    }


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
      elm[1] = "unstaged";
      elm[2] = std::string{wstatus};


      old_path = s->index_to_workdir->old_file.path;
      new_path = s->index_to_workdir->new_file.path;

      if (old_path && new_path && strcmp(old_path, new_path))
        elm[0] = std::string{old_path} + std::string{" -> "} + std::string{new_path};
      else
        elm[0] = old_path ? std::string{old_path} : std::string{new_path};

      return_array.push_back(elm);

      continue;
    }


    // list files which are staged for next commit
    //############################################

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
      
      
      elm[1] = "staged";
      elm[2] = std::string{istatus};

      old_path = s->head_to_index->old_file.path;
      new_path = s->head_to_index->new_file.path;

      if (old_path && new_path && strcmp(old_path, new_path))
        elm[0] = std::string{old_path} + std::string{" -> "} + std::string{new_path}; //TODO: use gul14
      else
        elm[0] = old_path ? std::string{old_path} : std::string{new_path};

      return_array.push_back(elm);

      
      continue;
  }

	

    // list files which are not staged for next commit
    //################################################
    //if (s->status == GIT_STATUS_CURRENT || s->index_to_workdir == nullptr)
      //continue;
	

    

    // list untracked files
    //######################
    if (s->status == GIT_STATUS_WT_NEW)
    {

      elm[1] = "untracked";
      elm[2] = "untracked";
      elm[0] = std::string(s->index_to_workdir->old_file.path);

      return_array.push_back(elm);

      continue;
    }


  // list ignored files
  //####################
    if (s->status == GIT_STATUS_IGNORED) {

      elm[1] = "ignored";
      elm[2] = "ignored";
      elm[0] = std::string(s->index_to_workdir->old_file.path);

      return_array.push_back(elm);

      continue;
    }

    delete [] old_path;
    delete [] new_path;
  }



  return return_array;
}

std::vector<std::array<std::string, 3>> GitRepository::libgit_status()
{
    // declare necessary variables
    git_status_list *my_status;
    git_status_options status_opt;

    // update files
    libgit_update();

    // init options with default values (= no args)
    int error = git_status_init_options(&status_opt, GIT_STATUS_OPT_INCLUDE_UNTRACKED);// & GIT_STATUS_OPT_INCLUDE_IGNORED);
    if (error) throw task::Error(gul14::cat("Cannot init status options."));

    // fill C-type status pointer
    error = git_status_list_new(&my_status, repo_.get(), &status_opt);
    if (error) throw task::Error(gul14::cat("Cannot init status."));

    // translate status pointer to redable status information
    std::vector<std::array<std::string, 3>> status_arr = collect_status(my_status);

    git_status_list_free(my_status);
    
    return status_arr;
}


std::vector <int> GitRepository::libgit_add_files(std::vector<std::filesystem::path> filepaths)
{
      //load index of last commit
    GitIndexPtr gindex;
    git_repository_index(gindex.getptr(), repo_.get());

    size_t v_len = filepaths.size();

    //iterate and add every file bypath
    std::vector <int> error_list;
    for (size_t i = 0; i < v_len; i++)
    {
      char *filepath = nullptr;
      strcpy(filepath, filepaths[i].c_str());
      int error = git_index_add_bypath(gindex.get(), filepath);
      if (error) error_list.push_back(i);
    }

    // save addition
    git_index_write(gindex.get());

    return error_list;
}

} //namespace task