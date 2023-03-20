#include <git2.h>
#include <string>
#include <filesystem>


class LibGit
{
public:
    LibGit(std::filesystem::path file_path);
    std::filesystem::path get_path() const;
    void libgit_init(std::filesystem::path file_path);
    void libgit_add();
    std::string get_last_commit_message() const;
    void libgit_commit(std::string commit_message);
    ~LibGit();

private:
    git_repository *repo;
    std::filesystem::path repo_path;
    git_signature *my_signature;
    void libgit_commit_initial();

};