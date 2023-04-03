#include <git2.h>
#include <utility>

/**
 * A wrapper to handle the raw C pointer of git_repository
**/
class GitRepositoryPtr

{
public:
    // empty
    //GitRepositoryPtr(){}

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

    git_repository** getptr() {return &repo_;};

    void reset()
    {
        git_repository_free(repo_);
        repo_ = nullptr;
    }


    //getter
    git_repository* get() {return repo_;} ;
    const git_repository* get() const {return repo_;} ;


private:
    git_repository* repo_;    
};


class GitSignaturePtr

{
public:

    // empty
    //GitSignaturePtr(){}

    // normal
    GitSignaturePtr(git_signature* signature): signature_{signature} {};

    // copy constructor
    GitSignaturePtr(const GitSignaturePtr& gl): GitSignaturePtr{gl.signature_} {};

    // move constructor
    GitSignaturePtr(GitSignaturePtr&& gl): signature_{std::exchange(gl.signature_, nullptr)} {};

    // destructor
    ~GitSignaturePtr(){ git_signature_free(signature_); };

    // copy assignment
    GitSignaturePtr& operator=(const GitSignaturePtr& gl) {return *this = GitSignaturePtr{gl};};

    // move assignment
    GitSignaturePtr& operator=(GitSignaturePtr&& gl) noexcept
    {
        std::swap(signature_, gl.signature_);
        return *this;
    }

    git_signature** getptr() {return &signature_;};


    //getter
    git_signature* get() {return signature_;} ;


private:
    git_signature* signature_;    
};


class GitIndexPtr

{
public:

    // empty
    GitIndexPtr(){}

    // normal
    GitIndexPtr(git_index* gindex): gindex_{gindex} {};

    // copy constructor
    GitIndexPtr(const GitIndexPtr& gl): GitIndexPtr{gl.gindex_} {};

    // move constructor
    GitIndexPtr(GitIndexPtr&& gl): gindex_{std::exchange(gl.gindex_, nullptr)} {};

    // destructor
    ~GitIndexPtr(){ git_index_free(gindex_); };

    // copy assignment
    GitIndexPtr& operator=(const GitIndexPtr& gl) {return *this = GitIndexPtr{gl};};

    // move assignment
    GitIndexPtr& operator=(GitIndexPtr&& gl) noexcept
    {
        std::swap(gindex_, gl.gindex_);
        return *this;
    }

    git_index** getptr() {return &gindex_;};


    //getter
    git_index* get() {return gindex_;} ;
    const git_index* get() const {return gindex_;} ;


private:
    git_index* gindex_{nullptr};    
};


class GitTreePtr

{
public:

    // empty
    GitTreePtr(){};
    
    // normal
    GitTreePtr(git_tree* tree): tree_{tree} {};

    // copy constructor
    GitTreePtr(const GitTreePtr& gl): GitTreePtr{gl.tree_} {};

    // move constructor
    GitTreePtr(GitTreePtr&& gl): tree_{std::exchange(gl.tree_, nullptr)} {};

    // destructor
    ~GitTreePtr(){ git_tree_free(tree_); };

    // copy assignment
    GitTreePtr& operator=(const GitTreePtr& gl) {return *this = GitTreePtr{gl};};

    // move assignment
    GitTreePtr& operator=(GitTreePtr&& gl) noexcept
    {
        std::swap(tree_, gl.tree_);
        return *this;
    }

    git_tree** getptr() {return &tree_;};


    //getter
    git_tree* get() {return tree_;} ;
    const git_tree* get() const {return tree_;} ;


private:
    git_tree* tree_{nullptr};    
};