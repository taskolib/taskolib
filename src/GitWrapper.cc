#include <git2.h>
#include "taskolib/GitWrapper.h"
#include "taskolib/exceptions.h"
#include "gul14/cat.h"
#include <iostream>
#include <vector>


LibGit::LibGit(std::filesystem::path file_path)
{
    //init libgit library
    git_libgit2_init();

    
    //init member variables
    repo_path_ = file_path;

    // if repository does not exist
    if (git_repository_open(&repo_, (repo_path_).c_str())) 
    {
        // initialize everything
        libgit_init(file_path);
    }
    else
    {
        // intialize only the signature
        int error = git_signature_default(&my_signature_, repo_);
        if (error==GIT_ENOTFOUND) git_signature_new(&my_signature_, "Taskomat", "taskomat@desy.de", 123456789, 0);
    }
    

}



LibGit::~LibGit()
{
    git_repository_free(repo_);
    git_signature_free(my_signature_);
    git_libgit2_shutdown();
}

std::string LibGit::get_last_commit_message() const
{
    const git_commit *commit = libgit_get_commit();
    return std::string(git_commit_message(commit));
}

std::filesystem::path LibGit::get_path() const
{
    return repo_path_;
}

git_repository* LibGit::get_repo()
{
    return repo_;
}


void LibGit::libgit_init(std::filesystem::path file_path)
{
    // create repository
    git_repository_init(&repo_, file_path.c_str(), false);

    // create signature
    int error = git_signature_default(&my_signature_, repo_);
    if (error==GIT_ENOTFOUND) git_signature_new(&my_signature_, "Taskomat", "taskomat@desy.de", 123456789, 0);

    // stage files in directory
    libgit_add();

    // make initial commit
    libgit_commit_initial();
}

void LibGit::libgit_commit_initial()
{
    // prepare gitlib data types
    git_index *index;
	git_oid tree_id, commit_id;
	git_tree *tree;

    // get tree structure for commit
    git_repository_index(&index, repo_);
	git_index_write_tree(&tree_id, index);
    git_tree_lookup(&tree, repo_, &tree_id);

    // commit
    git_commit_create_v(
		&commit_id,
        repo_,
        "HEAD",
        my_signature_,
        my_signature_,
		"UTF-8",
        "Initial commit",
        tree,
        0);

    //cleanup
	git_index_free(index);
    git_tree_free(tree);
}


void LibGit::libgit_commit(std::string commit_message)
{

    //get HEAD commit
    const git_commit* parent_commit = libgit_get_commit();

    //define types for commit call
    git_index *index;
	git_oid tree_id, commit_id;
	git_tree *tree;

    // get tree
    git_repository_index(&index, repo_);
	git_index_write_tree(&tree_id, index);
    git_tree_lookup(&tree, repo_, &tree_id);

    // create commit
    int error = git_commit_create(
        &commit_id,
        repo_,
        "HEAD",
        my_signature_,
        my_signature_,
        "UTF-8",
        commit_message.c_str(),
        tree,
        1,
        &parent_commit
    );

    if (error) throw task::Error(gul14::cat("Cannot commit."));


    //cleanup pointers
    git_index_free(index);
    git_tree_free(tree);
    
    //TODO: muss const pointer befreit werden?
    //git_commit_free(parent_commit);
}


void LibGit::libgit_add()
{
    //load index of last commit
    git_index *gindex;
    git_repository_index(&gindex, repo_);

    //transform filesystem::path into char*
    std::string my_path = repo_path_ / "*";
    char *path = new char[my_path.length()+1];
    strcpy(path, my_path.c_str());

    // init strarray
    char *paths[] = {path};
    git_strarray array = {paths, 1};

    // remove now useless char*
    delete [] path;

    //add all
    git_index_add_all(gindex, &array, 0, NULL, NULL);

    // save addition
    git_index_write(gindex);

    //cleanup pointer
    git_index_free(gindex);


}


void LibGit::libgit_remove_sequence(std::filesystem::path seq_directory)
{
    //load index of last commit
    git_index *gindex;
    git_repository_index(&gindex, repo_);

    // delete sequence
    std::filesystem::remove_all(repo_path_ / seq_directory);

    // remove the sequence from git
    int error = git_index_remove_bypath(gindex, seq_directory.c_str());
    if (error) throw task::Error(gul14::cat("Cannot remove repository."));

    // freeup pointer
    git_index_free(gindex);

}



git_commit* LibGit::libgit_get_commit(int count) const
{
    std::string ref = "HEAD^" + std::to_string(count);
    return libgit_get_commit(ref);
}
git_commit* LibGit::libgit_get_commit(const std::string& ref) const
{
    git_commit * commit;
    git_oid oid_parent_commit;

    // resolve HEAD into a SHA1
    int error = git_reference_name_to_id( &oid_parent_commit, repo_, ref.c_str());
    if (error) throw task::Error(gul14::cat("Cannot find HEAD of branch."));

    // find commit object by commit ID
    error = git_commit_lookup( &commit, repo_, &oid_parent_commit );
    if (error) throw task::Error(gul14::cat("Cannot find HEAD of branch."));

    return commit;

}
git_commit* LibGit::libgit_get_commit() const
{
    return libgit_get_commit(std::string{"HEAD"});

}


std::vector<std::array<std::string, 3>> LibGit::collect_status(git_status_list *status) const
{
  // get number of submodules
  const size_t maxi = git_status_list_entrycount(status);
  std::cout << maxi;

  // declare status holding vector for each submodule
  std::vector<std::array<std::string, 3>> return_array;
  std::array<std::string, 3> elm;


  const git_status_entry *s = NULL;
  const char *old_path = NULL;
  const char *new_path = NULL;

  for (size_t i = 0; i < maxi; ++i)
  {
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
        elm[0] = std::string{old_path} + std::string{" -> "} + std::string{new_path};
      else
        elm[0] = old_path ? std::string{old_path} : std::string{new_path};

      return_array.push_back(elm);

      continue;
  }

	

    // list files which are not staged for next commit
    //################################################
    if (s->status == GIT_STATUS_CURRENT || s->index_to_workdir == NULL)
      continue;
	

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
  }

  delete [] old_path;
  delete [] new_path;

  return return_array;
}

std::vector<std::array<std::string, 3>> LibGit::libgit_status() const
{
    // declare necessary variables
    git_status_list *my_status;
    git_status_options *status_opt;

    // init options with default values (= no args)
    git_status_init_options(status_opt, GIT_STATUS_OPTIONS_VERSION);

    // fill C-type status pointer
    git_status_list_new(&my_status, repo_, status_opt);

    //delete [] status_opt;

    // translate status pointer to redable status information
    std::vector<std::array<std::string, 3>> status_arr = collect_status(my_status);

    git_status_list_free(my_status);
    
    return status_arr;
}