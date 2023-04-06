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



/**
 * A wrapper to handle the raw C pointer of git_repository
**/

/*
class GitRepositoryPtr

{
public:
    // empty
    GitRepositoryPtr(){}

    // normal
    GitRepositoryPtr(git_repository* repo): repo_{repo} {};

    // move constructor
    GitRepositoryPtr(GitRepositoryPtr&& gl): repo_{std::exchange(gl.repo_, nullptr)} {};

    // destructor
    ~GitRepositoryPtr(){ git_repository_free(repo_); };

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
    git_repository* repo_ = nullptr;    
};


class GitSignaturePtr

{
public:

    // empty
    GitSignaturePtr(){}

    // normal
    GitSignaturePtr(git_signature* signature): signature_{signature} {};

    // move constructor
    GitSignaturePtr(GitSignaturePtr&& gl): signature_{std::exchange(gl.signature_, nullptr)} {};

    // destructor
    ~GitSignaturePtr(){ git_signature_free(signature_); };

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
    git_signature* signature_ = nullptr;    
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
    git_index* gindex_ = nullptr;    
};


class GitTreePtr

{
public:

    // empty
    GitTreePtr(){};
    
    // normal
    GitTreePtr(git_tree* tree): tree_{tree} {};

    // move constructor
    GitTreePtr(GitTreePtr&& gl): tree_{std::exchange(gl.tree_, nullptr)} {};

    // destructor
    ~GitTreePtr(){ git_tree_free(tree_); };

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
    git_tree* tree_ = nullptr;    
};

} //namespace task

*/

namespace task {

// free C-type pointer (overload)
void free_lg_ptr(git_tree* tree);
void free_lg_ptr(git_signature* signature);
void free_lg_ptr(git_index* index);
void free_lg_ptr(git_repository* repo);



template <class T>
class LgObject
{
public:
    // empty
    LgObject(){};
    
    // normal
    LgObject(T val): val_{val} {};

    // move constructor
    LgObject(LgObject&& lg): val_{std::exchange(lg.val_, nullptr)} {};

    // destructor
    ~LgObject(){ free_lg_ptr(val_); };

    // move assignment
    LgObject& operator=(LgObject&& lg) noexcept
    {
        std::swap(val_, lg.val_);
        return *this;
    }

    void reset()
    {
        free_lg_ptr(val_);
        val_ = nullptr;
    }

    // get pointer
    T* getptr() {return &val_;};

    //getter
    T get() {return val_;} ;
    const T get() const {return val_;} ;

private:
    T val_ = nullptr;
};


}