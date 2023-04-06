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


namespace task {

// free C-type pointer (overload)
void free_lg_ptr(git_tree* tree);
void free_lg_ptr(git_signature* signature);
void free_lg_ptr(git_index* index);
void free_lg_ptr(git_repository* repo);


/**
 * Template class wrapper for all libgit2 pointer types starting with git_{a-z} *
 * Copy methods are excluded to prevejnt double ownership of said pointer.
 * 
*/
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

    // function for member variables of GitRepository object
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