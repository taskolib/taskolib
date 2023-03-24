#include <git2.h>
#include <utility>

/**
 * A wrapper to handle the raw C pointer of git_repository
**/
class GitRepositoryPtr

{
public:
    // normal
    GitRepositoryPtr(git_repository* repo): repo_{repo} {};

    // copy constructor
    GitRepositoryPtr(const GitRepositoryPtr& gl): GitRepositoryPtr{gl.repo_} {};

    // move constructor
    GitRepositoryPtr(GitRepositoryPtr&& gl): repo_{std::exchange(gl.repo_, nullptr)} {};

    // destructor
    ~GitRepositoryPtr(){ git_repository_free(repo_); };

    // copy assignment
    GitRepositoryPtr& operator=(const GitRepositoryPtr& gl) {return *this = GitRepositoryPtr{gl};};

    // move assignment
    GitRepositoryPtr& operator=(GitRepositoryPtr&& gl) noexcept
    {
        std::swap(repo_, gl.repo_);
        return *this;
    }

    git_repository& operator*() {return *repo_;};


    //getter
    git_repository* get() const {return repo_;} ;


private:
    git_repository* repo_;    
};