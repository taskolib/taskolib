#include <git2.h>
#include <string>
#include <filesystem>
#include "taskolib/GitObjectWrapper.h"


class LibGit
{
public:
    explicit LibGit(std::filesystem::path file_path);
    std::filesystem::path get_path() const;
    git_repository* get_repo();
    void libgit_init(std::filesystem::path file_path);
    void libgit_add();
    std::string get_last_commit_message() const;
    void libgit_commit(std::string commit_message);
    void libgit_remove_sequence(std::filesystem::path seq_directory);
    ~LibGit();

private:
    git_repository *repo_{ nullptr };
    std::filesystem::path repo_path_;
    git_signature *my_signature_{ nullptr };
    void libgit_commit_initial();
    git_commit* libgit_get_commit(int count) const;
    git_commit* libgit_get_commit(const std::string& ref) const;
    git_commit* libgit_get_commit() const;

};