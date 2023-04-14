/**
 * \file   GitObjectWrapper.h
 * \author Sven-Jannik Wöhnert
 * \date   Created on March 20, 2023
 * \brief  Wrapper for data types defined in libgit2
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
#include <utility>
#include <typeinfo>
#include <string>


namespace task {

// free C-type pointer (overload)
void free_libgit_ptr(git_tree* tree);
void free_libgit_ptr(git_signature* signature);
void free_libgit_ptr(git_index* index);
void free_libgit_ptr(git_repository* repo);
void free_libgit_ptr(git_remote* remote);
void free_libgit_ptr(git_commit* commit);
void free_libgit_ptr(git_status_list* status);


/**
 * Template class wrapper for all libgit2 pointer types starting with git_{a-z}*.
 * Copy methods are excluded to prevent double ownership of said pointer.
 * 
 * \tparam T: A dereferencing from a pointer type like git_remote*, git_tree*, git_commit*, ...
*/
template <class T>
class LibGitPointer
{
public:
    /// (default) Default-construct an LibGitPointer. The managed pointer is NULL.
    LibGitPointer(){};
    
    /// (construct by value) Construct a LibGitPointer via a C-pointer from libgit2.
    LibGitPointer(T *val): val_{val} {};

    /// (move constructor) Move a C-pointer from an existing LibGitPointer to a new one.
    LibGitPointer(LibGitPointer&& lg): val_{std::exchange(lg.val_, nullptr)} {};

    /// (destructor) Destruct the Object by freeing the C-type pointer with a libgit function.
    ~LibGitPointer(){ free_libgit_ptr(val_); };

    /// (move assignment) move value from other object to this object
    LibGitPointer& operator=(LibGitPointer&& lg) noexcept
    {
        val_ = std::move(lg.val_);
        lg.val_ = nullptr;
        return *this;
    }

    /// (copy constructor) Shall not be used to prevent double ownership of pointer.
    LibGitPointer(const LibGitPointer&) = delete;

    /// (copy assignment) Shall not be used to prevent double ownership of pointer.
    LibGitPointer& operator=(const LibGitPointer&) = delete;

    /**
     * function to free member variables of GitRepository object before deconstruction of their owner.
     * necessary because they have to be freed before the libgit2 environment deconstruction, see:
     * git_libgit2_shutdow()
     */
    void reset()
    {
        free_libgit_ptr(val_);
        val_ = nullptr;
    }

    /// Return a non-owning pointer to the managed object (or nullptr if no object is managed).
    T* get() {return val_;} ;
    /// Return a constant non-owning pointer to the managed object (or nullptr if no object is managed).
    const T* get() const {return val_;};

private:
    T *val_ = nullptr;
};


/**
 * Function wrappers for libgit2 functions which manipulates pointers.
 * (Which take double pointers as parameters)
 * \note for documentation see the associated libgit2 function
 *       name: "git_FUNCTIONNAME"
 * \defgroup lgptrfunc Group of wrapped libgit2 functions
 * \{
 */

/**
 * Return an existing repository.
 * \param repo_path absolute or relative path from executable
 * \return Wrapper of a git_repository*
*/
LibGitPointer<git_repository> repository_open   (const std::string& repo_path);


/**
 * Return a fresh initialized repository.
 * \param repo_path absolute or relative path from executable
 * \return Wrapper of a git_repository*
*/
LibGitPointer<git_repository> repository_init   (const std::string& repo_path, bool is_bare);

/**
 * Return the current index of a repository.
 * \param repo C-type repository
 * \return Wrapper of a git_index*
*/
LibGitPointer<git_index>      repository_index  (git_repository* repo);

/**
 * Generate a signature from system values (already collected by the C-type repository).
 * \param repo_path absolute or relative path from executable
 * \return Wrapper of a git_signature*
*/
LibGitPointer<git_signature>  signature_default (git_repository* repo);

/**
 * Generate a signature from given parameters.
 * \param name User name who creates the commit
 * \param email E-mail of the user
 * \param time timestamp
 * \param offset timezone adjustment for the timezone
 * \return Wrapper of a git_signature*
*/
LibGitPointer<git_signature>  signature_new     (const std::string& name, const std::string& email,
                                                  int time, int offset);


/**
 * Collect the current tree of the git repository.
 * \param repo C-type repository
 * \param tree_id identity number of the active tree
 * \return Wrapper of a git_signature*
*/
LibGitPointer<git_tree>       tree_lookup       (git_repository* repo, git_oid tree_id);

/**
 * Create a new status list which conatins status_list_entry* elements
 * \param repo C-type repository
 * \param status_opt struct of status options
*/
LibGitPointer<git_status_list> status_list_new  (git_repository* repo, const git_status_options status_opt);
/** \}*/
}