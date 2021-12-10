/**
 * \file   LuaState.h
 * \author Lars Froehlich
 * \date   Created on December 3, 2021
 * \brief  Declaration of the LuaState class.
 *
 * \copyright Copyright 2021 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef AVTOMAT_LUASTATE_H_
#define AVTOMAT_LUASTATE_H_

#include <string>

struct lua_State; // Forward declaration of struct lua_State from the LUA headers


namespace avto {


/**
 * This class encapsulates a state of the LUA virtual machine.
 * It offers only a small subset of the functionalities of LUA.
 *
 * \note
 * LuaState is a move-only class. Moved-from objects represent a "closed" LUA state. They
 * can be deleted safely, but other operations on them may invoke undefined behavior.
 */
class LuaState
{
public:
    /**
     * Default constructor.
     *
     * \exception Error is thrown if a new LUA state cannot be created. This is usually an
     *            indication for an out-of-memory condition.
     */
    LuaState();

    /// Deleted copy constructor.
    LuaState(const LuaState& other) = delete;

    /**
     * Move constructor.
     * The new object takes over the LUA state from the original one. The moved-from
     * object can be deleted safely but other operations on it invoke undefined behavior.
     */
    LuaState(LuaState&& other);

    /// Destructor: Close the LUA state.
    ~LuaState() noexcept;

    /**
     * Close the LUA state.
     * After closing, most member functions will throw an exception when called. The
     * object can be safely deleted, however. The function may be called on an already
     * closed object.
     */
    void close() noexcept;

    /**
     * Create an empty table and push it onto the LUA stack.
     *
     * Although LUA's memory management is fully automatic, it is sometimes useful to
     * preallocate some space in the table for performance reasons. This can be done with
     * the following function parameters:
     *
     * \param num_seq_elements   A hint for how many sequential ("array") elements should
     *                           be preallocated.
     * \param num_other_elements A hint for how many other elements should be
     *                           preallocated.
     *
     * \exception hlc::Error is thrown if the table cannot be created (e.g. if the hints
     *            for the number of elements are negative).
     */
    void create_table(int num_seq_elements = 0, int num_other_elements = 0);

    /**
     * Return a pointer to the LUA state.
     * This pointer is null if the LUA state has been closed. Otherwise, it can be used to
     * call functions from the LUA C library directly. Calling lua_close() on it leads to
     * undefined behavior.
     */
    lua_State* get() const noexcept { return state_; }

    /**
     * Retrieve the global variable with the specified name, push its value onto the LUA
     * stack and return its type.
     */
    int get_global(const std::string& global_var_name);

    /**
     * Load a LUA script from a string without running it.
     * The script is precompiled into a chunk and its syntax is checked.
     * \exception hlc::Error is thrown if a syntax error is found, if there is
     *            insufficient memory, or if the LUA state is closed.
     */
    void load_string(const std::string& script);

    /// Copy assignment is deleted.
    LuaState& operator=(const LuaState& other) = delete;

    /**
     * Move assignment.
     * The existing LUA state is closed and the assigned one is taken over. The moved-from
     * state can be deleted safely but other operations on it may invoke undefined
     * behavior.
     */
    LuaState& operator=(LuaState&& other) noexcept;

    /// Push a number onto the LUA stack.
    void push_number(double value);

    /**
     * Pop a number from the LUA stack and return it.
     * \exception hlc::Error is thrown if there is no value to pop from the stack or if
     *            it cannot be converted to a number.
     */
    double pop_number();

    /**
     * Pop a string from the LUA stack and return it.
     * \exception hlc::Error is thrown if there is no value to pop from the stack or if
     *            it cannot be converted to a string.
     */
    std::string pop_string();

    /**
     * Pops a value from the LUA stack and assigns it to the global variable with the
     * specified name.
     */
    void set_global(const std::string& global_var_name);

private:
    lua_State* state_ = nullptr;

    // Throw hlc::Error if this LUA state has already been closed (or moved from).
    void throw_if_closed();
};


} // namespace avto


#endif
