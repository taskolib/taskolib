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

struct lua_State; // Forward declaration of struct lua_State from the LUA headers


namespace avto {


/**
 * This class encapsulates a state of the LUA virtual machine.
 * It offers only a small subset of the functionalities of LUA.
 *
 * \note
 * LuaState is a move-only class. Moved-from objects can be deleted safely, but other
 * operations on them invoke undefined behavior.
 */
class LuaState
{
public:
    /**
     * Default constructor.
     *
     * \exception hlc::Error is thrown if a new LUA state cannot be created. This is
     *            usually an indication for an out-of-memory condition.
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
     * Return a pointer to the LUA state.
     * This pointer can be used to call functions from the LUA C library directly.
     * Calling lua_close() on it leads to undefined behavior.
     */
    lua_State* get() const noexcept { return state_; }

    /// Copy assignment is deleted.
    LuaState& operator=(const LuaState& other) = delete;

    /**
     * Move assignment.
     * The existing LUA state is closed and the assigned one is taken over. The moved-from
     * state can be deleted safely but other operations on it invoke undefined behavior.
     */
    LuaState& operator=(LuaState&& other) noexcept;

private:
    lua_State* state_ = nullptr;

    // Close the LUA state if one is open and set the state pointer to nullptr.
    void close() noexcept;
};


} // namespace avto


#endif
