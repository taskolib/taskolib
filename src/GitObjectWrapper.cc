#include <git2.h>
#include "taskolib/GitObjectWrapper.h"

namespace task {

// free C-type pointer (overload)
void free_lg_ptr(git_tree* tree)
{
    git_tree_free(tree);
}
void free_lg_ptr(git_signature* signature)
{
    git_signature_free(signature);
}
void free_lg_ptr(git_index* index)
{
    git_index_free(index);
}
void free_lg_ptr(git_repository* repo)
{
    git_repository_free(repo);
}

}