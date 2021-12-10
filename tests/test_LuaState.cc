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

#include <limits>
#include <type_traits>
#include <gul14/catch.h>
#include <hlc/util/exceptions.h>
#include <lua.h>
#include "../include/avtomat/Error.h"
#include "../include/avtomat/LuaState.h"

using namespace std::literals;
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

TEST_CASE("LuaState: assign_field()", "[LuaState]")
{
    LuaState state;

    state.create_table();
    state.assign_field(1, 42.0);
    REQUIRE(state.get_top() == 1); // 1 object on stack (just the table)
    REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

    state.assign_field("mykey", "Hello world!", 1);
    REQUIRE(state.get_top() == 1); // 1 object on stack (just the table)
    REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

    state.push_number(1); // index to retrieve
    lua_gettable(state.get(), -2);
    REQUIRE(state.get_top() == 2); // 2 objects on stack (table + result)
    REQUIRE(state.pop_number() == 42.0);
    REQUIRE(state.get_top() == 1);

    state.push_string("mykey"); // index to retrieve
    lua_gettable(state.get(), 1);
    REQUIRE(state.get_top() == 2); // 2 objects on stack (table + result)
    REQUIRE(state.pop_string() == "Hello world!");
    REQUIRE(state.get_top() == 1);
}

TEST_CASE("LuaState: create_table()", "[LuaState]")
{
    LuaState state;

    state.create_table();
    REQUIRE(state.get_top() == 1); // 1 object on stack
    REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

    state.create_table(0, 10);
    REQUIRE(state.get_top() == 2); // 2 objects on stack
    REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

    state.create_table(10, 0);
    REQUIRE(state.get_top() == 3); // 3 objects on stack
    REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

    state.create_table(10, 10);
    REQUIRE(state.get_top() == 4); // 4 objects on stack
    REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

    REQUIRE_THROWS_AS(state.create_table(-1, 0), Error);
    REQUIRE(state.get_top() == 4); // 4 objects on stack

    REQUIRE_THROWS_AS(state.create_table(0, -2), Error);
    REQUIRE(state.get_top() == 4); // 4 objects on stack

    REQUIRE_THROWS_AS(state.create_table(-10, -1), Error);
    REQUIRE(state.get_top() == 4); // 4 objects on stack
}

TEST_CASE("LuaState: get()", "[LuaState]")
{
    LuaState state;
    REQUIRE(state.get() != nullptr);

    auto state2 = std::move(state);
    REQUIRE(state.get() == nullptr);
}

TEST_CASE("LuaState: get_global()", "[LuaState]")
{
    LuaState state;
    REQUIRE(state.get_global("pippo") == LUA_TNIL);

    state.push_number(42.0);
    state.set_global("pippo");

    REQUIRE(state.get_global("pippo") == LUA_TNUMBER);
}

TEST_CASE("LuaState: get_top()", "[LuaState]")
{
    LuaState state;
    REQUIRE(state.get_top() == 0);

    state.push_number(42);
    REQUIRE(state.get_top() == 1);
}

TEST_CASE("LuaState: load_string()", "[LuaState]")
{
    LuaState state;

    SECTION("Valid LUA strings")
    {
        state.load_string("");
        state.load_string("local a = 2");
    }

    SECTION("Syntax error")
    {
        REQUIRE_THROWS_AS(state.load_string("locally a = 2"), Error);
    }
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

TEST_CASE("LuaState: pop_integer()", "[LuaState]")
{
    LuaState state;

    SECTION("Number can be retrieved and stack position is adjusted")
    {
        state.push_integer(std::numeric_limits<long long>::lowest());
        REQUIRE(state.get_top() == 1);
        REQUIRE(state.pop_integer() == std::numeric_limits<long long>::lowest());
        REQUIRE(state.get_top() == 0);
    }

    SECTION("Exception thrown if value on stack cannot be converted into integer")
    {
        lua_pushnil(state.get());
        REQUIRE(state.get_top() == 1);
        REQUIRE_THROWS_AS(state.pop_integer(), Error);
        REQUIRE(state.get_top() == 1);
    }
}

TEST_CASE("LuaState: pop_number()", "[LuaState]")
{
    LuaState state;

    SECTION("Number can be retrieved and stack position is adjusted")
    {
        state.push_number(42.0);
        REQUIRE(state.get_top() == 1);
        REQUIRE(state.pop_number() == 42.0);
        REQUIRE(state.get_top() == 0);
    }

    SECTION("Exception thrown if value on stack cannot be converted into number")
    {
        lua_pushnil(state.get());
        REQUIRE(state.get_top() == 1);
        REQUIRE_THROWS_AS(state.pop_number(), Error);
        REQUIRE(state.get_top() == 1);
    }
}

TEST_CASE("LuaState: pop_string()", "[LuaState]")
{
    LuaState state;

    SECTION("String can be retrieved and stack position is adjusted")
    {
        lua_pushstring(state.get(), "Test");
        REQUIRE(state.get_top() == 1);
        REQUIRE(state.pop_string() == "Test");
        REQUIRE(state.get_top() == 0);
    }

    SECTION("String with embedded zero byte can be retrieved")
    {
        state.push_string("test1\0test2"s);
        REQUIRE(state.get_top() == 1);
        REQUIRE(state.pop_string() == "test1\0test2"s);
        REQUIRE(state.get_top() == 0);
    }

    SECTION("Exception thrown if value on stack cannot be converted into string")
    {
        lua_pushnil(state.get());
        REQUIRE(state.get_top() == 1);
        REQUIRE_THROWS_AS(state.pop_string(), Error);
        REQUIRE(state.get_top() == 1);
    }
}

TEST_CASE("LuaState: push()", "[LuaState]")
{
    LuaState state;

    state.push(char{ 42 });
    REQUIRE(lua_type(state.get(), -1) == LUA_TNUMBER);
    REQUIRE(state.pop_integer() == 42);

    state.push(42);
    REQUIRE(lua_type(state.get(), -1) == LUA_TNUMBER);
    REQUIRE(state.pop_integer() == 42);

    state.push(123'456'789ULL);
    REQUIRE(lua_type(state.get(), -1) == LUA_TNUMBER);
    REQUIRE(state.pop_integer() == 123'456'789);

    state.push(-43.5);
    REQUIRE(lua_type(state.get(), -1) == LUA_TNUMBER);
    REQUIRE(state.pop_number() == -43.5);

    state.push(-43.5L);
    REQUIRE(lua_type(state.get(), -1) == LUA_TNUMBER);
    REQUIRE(state.pop_number() == -43.5);

    state.push(nullptr);
    REQUIRE(lua_type(state.get(), -1) == LUA_TNIL);
    lua_pop(state.get(), 1);

    state.push("Hello world!");
    REQUIRE(lua_type(state.get(), -1) == LUA_TSTRING);
    REQUIRE(state.pop_string() == "Hello world!");

    state.push("Hello\0world!"s);
    REQUIRE(lua_type(state.get(), -1) == LUA_TSTRING);
    REQUIRE(state.pop_string() == "Hello\0world!"s);
}

TEST_CASE("LuaState: push_integer()", "[LuaState]")
{
    LuaState state;

    state.push_integer(42);
    REQUIRE(state.get_top() == 1);
    REQUIRE(state.pop_integer() == 42);

    state.push_integer(std::numeric_limits<long long>::max());
    REQUIRE(state.get_top() == 1);
    REQUIRE(state.pop_integer() == std::numeric_limits<long long>::max());

    state.push_integer(std::numeric_limits<long long>::lowest());
    REQUIRE(state.get_top() == 1);
    REQUIRE(state.pop_integer() == std::numeric_limits<long long>::lowest());
}

TEST_CASE("LuaState: push_number()", "[LuaState]")
{
    LuaState state;

    state.push_number(42.0);
    REQUIRE(state.get_top() == 1);
    REQUIRE(state.pop_number() == 42.0);

    state.push_number(std::numeric_limits<double>::max());
    REQUIRE(state.get_top() == 1);
    REQUIRE(state.pop_number() == std::numeric_limits<double>::max());

    state.push_number(std::numeric_limits<double>::lowest());
    REQUIRE(state.get_top() == 1);
    REQUIRE(state.pop_number() == std::numeric_limits<double>::lowest());
}

TEST_CASE("LuaState: push_string()", "[LuaState]")
{
    LuaState state;

    SECTION("Null-terminated string")
    {
        state.push_string("Hello World");
        REQUIRE(state.get_top() == 1);
        REQUIRE(state.pop_string() == "Hello World");
    }

    SECTION("nullptr")
    {
        state.push_string(nullptr);
        REQUIRE(state.get_top() == 1);
        REQUIRE(lua_type(state.get(), -1) == LUA_TNIL);
    }

    SECTION("String with embedded zero bytes")
    {
        state.push_string("test1\0test2"s);
        REQUIRE(state.get_top() == 1);
        REQUIRE(state.pop_string() == "test1\0test2"s);
    }
}

TEST_CASE("LuaState: set_global()", "[LuaState]")
{
    LuaState state;

    state.push_number(42.0);
    state.set_global("pippo");

    REQUIRE(state.get_global("pippo") == LUA_TNUMBER);
}

TEST_CASE("LuaState: set_table()", "[LuaState]")
{
    LuaState state;

    SECTION("Store value 42 at table index 1")
    {
        state.create_table();
        state.push_number(1);
        state.push_number(42.0);
        state.set_table(-3);
        REQUIRE(state.get_top() == 1); // 1 object on stack (just the table)
        REQUIRE(lua_type(state.get(), -1) == LUA_TTABLE);

        state.push_number(1); // index to retrieve
        lua_gettable(state.get(), -2);
        REQUIRE(state.get_top() == 2); // 2 objects on stack (table + result)
        REQUIRE(state.pop_number() == 42.0);
    }
}
