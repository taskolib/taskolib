#include <git2.h>
#include "taskolib/GitWrapper.h"
//#include "taskolib/GitObjectWrapper.h"
#include "taskolib/exceptions.h"
#include "gul14/cat.h"
#include <iostream>


LibGit::LibGit(std::filesystem::path file_path)
{
    //init libgit library
    git_libgit2_init();

    
    //init member variables
    repo_path_ = file_path;
    if (git_repository_open(&repo_, (repo_path_).c_str())) 
    {
        libgit_init(file_path);
    }
    else
    {
        int error = git_signature_default(&my_signature_, repo_);
        if (error==GIT_ENOTFOUND) git_signature_new(&my_signature_, "Taskomat", "jannik.woehnert@desy.de", 123456789, 0);
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
    // Test if repo_ got initialized
    //git_object *head_commit;
    //int error = git_revparse_single(&head_commit, repo_, "HEAD");
    //if (error) throw task::Error(gul14::cat("Cannot find HEAD of branch."));

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
    git_index *index;
	git_oid tree_id, commit_id;
	git_tree *tree;

    git_repository_index(&index, repo_);
	git_index_write_tree(&tree_id, index);

    git_tree_lookup(&tree, repo_, &tree_id);

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

    libgit_add();
    libgit_commit("remove " + seq_directory.string());

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
    git_oid oid_parent_commit;  /* the SHA1 for last commit */

    /* resolve HEAD into a SHA1 */
    int error = git_reference_name_to_id( &oid_parent_commit, repo_, ref.c_str());
    if (error) throw task::Error(gul14::cat("Cannot find HEAD of branch."));

    error = git_commit_lookup( &commit, repo_, &oid_parent_commit );
    if (error) throw task::Error(gul14::cat("Cannot find HEAD of branch."));

    return commit;

}
git_commit* LibGit::libgit_get_commit() const
{
    return libgit_get_commit(std::string{"HEAD"});

}
