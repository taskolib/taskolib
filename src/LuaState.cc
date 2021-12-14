/**
 * \file   LuaState.cc
 * \author Lars Froehlich
 * \date   Created on December 3, 2021
 * \brief  Implementation of the LuaState class.
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

#include <gul14/cat.h>
#include <lua.h>
#include <lauxlib.h>
#include "avtomat/Error.h"
#include "avtomat/LuaState.h"

using gul14::cat;

namespace avto {


LuaState::LuaState()
{
    state_ = luaL_newstate();

    if (state_ == nullptr)
        throw Error("Unable to create new LUA state");
}

LuaState::LuaState(LuaState&& other)
    : state_{ other.state_ }
{
    other.state_ = nullptr;
}

LuaState::~LuaState() noexcept
{
    close();
}

void LuaState::call_function_with_n_arguments(int num_args)
{
    const int stack_pos = get_top();

    // On the stack we should have the function and all arguments, in that order
    if (stack_pos <= num_args || get_type(stack_pos - num_args) != LuaType::function)
        throw Error("Found no callable function on the LUA stack");

    int err = lua_pcall(state_, num_args, LUA_MULTRET, 0);

    if (err != LUA_OK)
        throw Error(cat("Error while executing script: ", pop_string()));
}

void LuaState::close() noexcept
{
    if (state_ == nullptr)
        return;

    lua_close(state_);
    state_ = nullptr;
}

void LuaState::create_table(int num_seq_elements, int num_other_elements)
{
    if (num_seq_elements < 0 || num_other_elements < 0)
    {
        throw Error(cat("Invalid parameters for create_table: num_seq_elements = ",
            num_seq_elements, ", num_other_elements = ", num_other_elements));
    }

    return lua_createtable(state_, num_seq_elements, num_other_elements);
}

LuaType LuaState::get_global(const std::string& global_var_name)
{
    return static_cast<LuaType>(lua_getglobal(state_, global_var_name.c_str()));
}

int LuaState::get_top()
{
    return lua_gettop(state_);
}

LuaType LuaState::get_type(int stack_index)
{
    return static_cast<LuaType>(lua_type(state_, stack_index));
}

void LuaState::load_string(const std::string& script)
{
    int err = luaL_loadstring(state_, script.c_str());
    if (err)
    {
        // If something went wrong, error message is at the top of the stack
        throw Error(cat("Cannot precompile script: ", pop_string()));
    }
}

LuaState& LuaState::operator=(LuaState&& other) noexcept
{
    close();

    state_ = other.state_;
    other.state_ = nullptr;
    return *this;
}

long long LuaState::pop_integer()
{
    int success = 0;

    long long val = lua_tointegerx(state_, -1, &success);

    if (!success)
        throw Error("Cannot pop integer from LUA stack");

    lua_pop(state_, 1);

    return val;
}

double LuaState::pop_number()
{
    int success = 0;

    double val = lua_tonumberx(state_, -1, &success);

    if (!success)
        throw Error("Cannot pop number from LUA stack");

    lua_pop(state_, 1);

    return val;
}

std::string LuaState::pop_string()
{
    size_t len;
    const char* lua_str = lua_tolstring(state_, -1, &len);

    if (lua_str == nullptr)
        throw Error("Unable to pop string from LUA stack");

    std::string str{ lua_str, len };
    lua_pop(state_, 1);

    return str;
}

void LuaState::push_integer(long long value)
{
    lua_pushinteger(state_, value);
}

void LuaState::push_number(double value)
{
    lua_pushnumber(state_, value);
}

void LuaState::push_string(const std::string& str)
{
    lua_pushlstring(state_, str.data(), str.size());
}

void LuaState::push_string(const char* str)
{
    lua_pushstring(state_, str);
}

void LuaState::set_global(const std::string& global_var_name)
{
    lua_setglobal(state_, global_var_name.c_str());
}

void LuaState::set_table(int table_stack_index)
{
    lua_settable(state_, table_stack_index);
}


} // namespace avto
