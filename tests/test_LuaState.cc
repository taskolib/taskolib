/**
 * \file   test_LuaState.cc
 * \author Lars Froehlich
 * \date   Created on December 3, 2021
 * \brief  Test suite for the LuaState class.
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

#include <type_traits>
#include <gul14/catch.h>
#include <hlc/util/exceptions.h>
#include <lua.h>
#include "../include/avtomat/LuaState.h"

using namespace avto;

TEST_CASE("LuaState: Default constructor", "[LuaState]")
{
    static_assert(std::is_default_constructible<LuaState>::value,
        "LuaState is_default_constructible");

    LuaState state;
}

TEST_CASE("LuaState: Move constructor", "[LuaState]")
{
    LuaState state;

    LuaState state2{ std::move(state) };
    REQUIRE(state.get() == nullptr);
    REQUIRE(state2.get() != nullptr);
}

TEST_CASE("LuaState: get()", "[LuaState]")
{
    LuaState state1;
    REQUIRE(state1.get() != nullptr);
}

TEST_CASE("LuaState: load_string()", "[LuaState]")
{
    LuaState state;

    state.load_string("");
    state.load_string("local a = 2");
    REQUIRE_THROWS_AS(state.load_string("locally a = 2"), hlc::Error); // syntax error
}

TEST_CASE("LuaState: operator=(LuaState&&) (move assignment)", "[LuaState]")
{
    LuaState state1;
    auto* state1_ptr = state1.get();
    REQUIRE(state1_ptr != nullptr);

    LuaState state2;
    auto* state2_ptr = state2.get();
    REQUIRE(state2_ptr != nullptr);

    state2 = std::move(state1);
    REQUIRE(state1.get() == nullptr);
    REQUIRE(state2.get() == state1_ptr);
}

TEST_CASE("LuaState: pop_string()", "[LuaState]")
{
    LuaState state;

    auto initial_stack_pos = lua_gettop(state.get());
    lua_pushstring(state.get(), "Test");

    REQUIRE(state.pop_string() == "Test");
    REQUIRE(lua_gettop(state.get()) == initial_stack_pos);
}
