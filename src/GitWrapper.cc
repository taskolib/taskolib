#include <git2.h>
#include "taskolib/GitWrapper.h"


LibGit::LibGit(std::filesystem::path file_path)
{
    //init libgit library
    git_libgit2_init();

    //init member variables
    repo_path = file_path.append("/.git"); //repo_path = repo_path.append(); ???
    if (! git_repository_open(&repo, repo_path.c_str())) libgit_init(file_path);
    git_signature_default(&my_signature, repo);

}



LibGit::~LibGit()
{
    git_repository_free(repo);
    git_signature_free(my_signature);
    git_libgit2_shutdown();
}

std::string LibGit::get_last_commit_message() const
{
    // Test if repo got initialized
    git_object *head_commit;
    int error = git_revparse_single(&head_commit, repo, "HEAD^{commit}");
    git_commit *commit = (git_commit*)head_commit;
    return git_commit_message(commit);
}

std::filesystem::path LibGit::get_path() const
{
    return repo_path;
}


void LibGit::libgit_init(std::filesystem::path file_path)
{
    git_repository_init(&repo, file_path.c_str(), false);
    libgit_commit_initial();
}

void LibGit::libgit_commit_initial()
{
    git_index *index;
	git_oid tree_id, commit_id;
	git_tree *tree;

    git_repository_index(&index, repo);
	git_index_write_tree(&tree_id, index);

    git_tree_lookup(&tree, repo, &tree_id);

    git_commit_create_v(
		&commit_id,
        repo,
        "HEAD",
        my_signature,
        my_signature,
		NULL,
        "Initial commit",
        tree,
        0);

    //cleanup
	git_index_free(index);
    git_tree_free(tree);




}


void LibGit::libgit_commit(std::string commit_message)
{
    // prepare writable variables as commit feedbacks (not used yet)
    git_oid commit_id;
    git_tree *tree;

    // find HEAD commit to connect to new commit
    git_commit *parent;
    git_oid parent_id;
    git_reference_name_to_id(&parent_id, repo, "HEAD");
    git_commit_lookup(&parent, repo, &parent_id);

    // create commit
    git_commit_create(
        &commit_id,
        repo,
        "HEAD",
        my_signature,
        my_signature,
        "UTF-8",
        commit_message.c_str(),
        tree,
        1,
        &parent
    );


    //cleanup pointers
    git_commit_free(parent);
    git_tree_free(tree);
}


void LibGit::libgit_add()
{
    //load index of last commit
    git_index *gindex;
    git_repository_index(&gindex, repo);

    //create to collect (unused) path pattern from add_all
    git_strarray array = {0};

    //add all
    git_index_add_all(gindex, &array, 0, NULL, NULL);

    // save addition
    git_index_write(gindex);

    //cleanup pointers
    git_index_free(gindex);


}

