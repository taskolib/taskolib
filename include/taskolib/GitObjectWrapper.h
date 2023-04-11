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


/**
 * Template class wrapper for all libgit2 pointer types starting with git_{a-z}*.
 * Copy methods are excluded to prevent double ownership of said pointer.
 * 
 * \class T: types like git_remote*, git_tree*, git_commit*, ...
*/
template <class T>
class LibGitPointer
{
public:
    /// (default) Default-construct an LibGitPointer. The managed pointer is NULL.
    LibGitPointer(){};
    
    /// (construct by value) Construct a LibGitPointer via a C-pointer from libgit2.
    LibGitPointer(T val): val_{val} {};

    /// (move constructor) Move a C-pointer from an existing LibGitPointer to a new one.
    LibGitPointer(LibGitPointer&& lg): val_{std::exchange(lg.val_, nullptr)} {};

    /// (destructor) Destruct the Object by freeing the C-type pointer with a libgit function.
    ~LibGitPointer(){ free_libgit_ptr(val_); };

    /// (move assignment) swap C-type pointer bewteen objects.
    LibGitPointer& operator=(LibGitPointer&& lg) noexcept
    {
        std::swap(val_, lg.val_);
        return *this;
    }

    /// (copy constructor) Shall not be used to prevent double ownership of pointer.
    LibGitPointer(const LibGitPointer&) = delete;

    /// (copy assignment) Shall not be used to prevent double ownership of pointer.
    LibGitPointer& operator=(const LibGitPointer&) = delete;

    /// function for member variables of GitRepository object
    void reset()
    {
        free_libgit_ptr(val_);
        val_ = nullptr;
    }

    /// getter functions for const and non-const
    T get() {return val_;} ;
    const T get() const {return val_;} ;

private:
    T val_ = nullptr;
};


/**
 * Function wrappers for libgit2 functions which manipulates pointers.
 * (Which take double pointers as parameters)
 * \note for documentation see the associated libgit2 function
 *       name: "git_FUNCTIONNAME"
*/
LibGitPointer<git_repository*> repository_open   (const std::string& repo_path);
LibGitPointer<git_repository*> repository_init   (const std::string& repo_path, bool is_bare);
LibGitPointer<git_index*>      repository_index  (git_repository* repo);
LibGitPointer<git_signature*>  signature_default (git_repository* repo);
LibGitPointer<git_signature*>  signature_new     (const std::string& name, const std::string& email,
                                                  int time, int offset);
LibGitPointer<git_tree*>       tree_lookup       (git_repository* repo, git_oid tree_id);
}