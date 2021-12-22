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

    // No library functions may be loaded
    REQUIRE(state.get_global("print") == LuaType::nil);
    REQUIRE(state.get_global("ipairs") == LuaType::nil);
}

TEST_CASE("LuaState: Constructor with LuaLibraries::all", "[LuaState]")
{
    LuaState state(LuaLibraries::all);

    REQUIRE(state.get_global("print") == LuaType::function);
    REQUIRE(state.get_global("ipairs") == LuaType::function);
}

TEST_CASE("LuaState: Constructor with LuaLibraries::safe_subset", "[LuaState]")
{
    LuaState state(LuaLibraries::safe_subset);

    REQUIRE(state.get_global("print") == LuaType::nil);
    REQUIRE(state.get_global("ipairs") == LuaType::function);
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
    REQUIRE(state.get_type() == LuaType::table);

    state.assign_field("mykey", "Hello world!", 1);
    REQUIRE(state.get_top() == 1); // 1 object on stack (just the table)
    REQUIRE(state.get_type() == LuaType::table);

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

TEST_CASE("LuaState: call_function()", "[LuaState]")
{
    LuaState state;

    SECTION("Calling a valid function with 2 results")
    {
        state.load_string(R"(
            function sum_and_count(...)
                result = 0
                local arg = {...}
                for i = 1, #arg do
                    result = result + arg[i]
                end
                return result, #arg
            end
        )");

        REQUIRE(state.get_top() == 1); // 1 object on stack (the chunk)
        REQUIRE(state.get_type() == LuaType::function);
        REQUIRE(state.call_function() == 0); // execute chunk to get function definition
        REQUIRE(state.get_top() == 0);

        state.get_global("sum_and_count");
        REQUIRE(state.get_top() == 1); // 1 object on stack (the function)
        REQUIRE(state.get_type() == LuaType::function);

        SECTION("Call without parameters")
        {
            REQUIRE(state.call_function() == 2);
            REQUIRE(state.get_top() == 2); // just the 2 return values are on the stack
            REQUIRE(state.pop_number() == 0.0); // sum 0.0
            REQUIRE(state.pop_integer() == 0); // 0 input arguments given
        }

        SECTION("Call with 1 parameter")
        {
            REQUIRE(state.call_function(42.0) == 2);
            REQUIRE(state.get_top() == 2); // 2 return values
            REQUIRE(state.pop_integer() == 1); // # input arguments
            REQUIRE(state.pop_number() == 42.0); // sum
        }

        SECTION("Call with 2 parameters")
        {
            REQUIRE(state.call_function(42.0, -13.0) == 2);
            REQUIRE(state.get_top() == 2); // 2 return values
            REQUIRE(state.pop_integer() == 2); // # input arguments
            REQUIRE(state.pop_number() == 29.0); // sum
        }

        SECTION("Call with 3 parameters")
        {
            state.call_function(42.0, -13.0, -25LL);
            REQUIRE(state.get_top() == 2); // 2 return values
            REQUIRE(state.pop_integer() == 3); // # input arguments
            REQUIRE(state.pop_number() == 4.0); // sum
        }
    }

    SECTION("Cannot call a function on an empty stack")
    {
        REQUIRE(state.get_top() == 0);
        REQUIRE_THROWS_AS(state.call_function(), Error);
    }

    SECTION("Cannot call a non-function")
    {
        state.push_number(42);
        REQUIRE_THROWS_AS(state.call_function(), Error);
    }
}

TEST_CASE("LuaState: create_table()", "[LuaState]")
{
    LuaState state;

    state.create_table();
    REQUIRE(state.get_top() == 1); // 1 object on stack
    REQUIRE(state.get_type() == LuaType::table);

    state.create_table(0, 10);
    REQUIRE(state.get_top() == 2); // 2 objects on stack
    REQUIRE(state.get_type() == LuaType::table);

    state.create_table(10, 0);
    REQUIRE(state.get_top() == 3); // 3 objects on stack
    REQUIRE(state.get_type() == LuaType::table);

    state.create_table(10, 10);
    REQUIRE(state.get_top() == 4); // 4 objects on stack
    REQUIRE(state.get_type() == LuaType::table);

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

TEST_CASE("LuaState: get_type()", "[LuaState]")
{
    LuaState state;

    state.push_integer(42);
    REQUIRE(state.get_type() == LuaType::number);

    state.push_string("Test");
    REQUIRE(state.get_type() == LuaType::string);
    REQUIRE(state.get_type(-2) == LuaType::number);

    state.push_number(-1.5);
    REQUIRE(state.get_type() == LuaType::number);
    REQUIRE(state.get_type(-2) == LuaType::string);
    REQUIRE(state.get_type(-3) == LuaType::number);

    REQUIRE(state.get_type(1) == LuaType::number);
    REQUIRE(state.get_type(2) == LuaType::string);
    REQUIRE(state.get_type(3) == LuaType::number);
}

TEST_CASE("LuaState: get_global()", "[LuaState]")
{
    LuaState state;
    REQUIRE(state.get_global("pippo") == LuaType::nil);

    state.push_number(42.0);
    state.set_global("pippo");

    REQUIRE(state.get_global("pippo") == LuaType::number);
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

TEST_CASE("LuaState: open_libraries()", "[LuaState]")
{
    LuaState state;

    SECTION("LuaLibraries::none")
    {
        state.open_libraries(LuaLibraries::none);
        REQUIRE(state.get_global("ipairs") == LuaType::nil);
        REQUIRE(state.get_global("print") == LuaType::nil);
        REQUIRE(state.get_global("io") == LuaType::nil);
        REQUIRE(state.get_global("os") == LuaType::nil);
        REQUIRE(state.get_global("string") == LuaType::nil);
    }

    SECTION("LuaLibraries::all")
    {
        state.open_libraries(LuaLibraries::all);
        REQUIRE(state.get_global("ipairs") == LuaType::function);
        REQUIRE(state.get_global("print") == LuaType::function);
        REQUIRE(state.get_global("io") == LuaType::table);
        REQUIRE(state.get_global("os") == LuaType::table);
        REQUIRE(state.get_global("string") == LuaType::table);
    }

    SECTION("LuaLibraries::safe_subset")
    {
        state.open_libraries(LuaLibraries::safe_subset);
        REQUIRE(state.get_global("ipairs") == LuaType::function);
        REQUIRE(state.get_global("print") == LuaType::nil);
        REQUIRE(state.get_global("io") == LuaType::nil);
        REQUIRE(state.get_global("os") == LuaType::nil);
        REQUIRE(state.get_global("string") == LuaType::table);
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

TEST_CASE("LuaState: pop()", "[LuaState]")
{
    LuaState state;

    state.push_number(42);
    REQUIRE(state.get_top() == 1);

    state.pop();
    REQUIRE(state.get_top() == 0);

    state.push_number(42);
    state.push_string("Hello");
    REQUIRE(state.get_top() == 2);
    state.pop(2);
    REQUIRE(state.get_top() == 0);
}

TEST_CASE("LuaState: push()", "[LuaState]")
{
    LuaState state;

    state.push(char{ 42 });
    REQUIRE(state.get_type() == LuaType::number);
    REQUIRE(state.pop_integer() == 42);

    state.push(42);
    REQUIRE(state.get_type() == LuaType::number);
    REQUIRE(state.pop_integer() == 42);

    state.push(123'456'789ULL);
    REQUIRE(state.get_type() == LuaType::number);
    REQUIRE(state.pop_integer() == 123'456'789);

    state.push(-43.5);
    REQUIRE(state.get_type() == LuaType::number);
    REQUIRE(state.pop_number() == -43.5);

    state.push(-43.5L);
    REQUIRE(state.get_type() == LuaType::number);
    REQUIRE(state.pop_number() == -43.5);

    state.push(nullptr);
    REQUIRE(state.get_type() == LuaType::nil);
    state.pop();

    state.push("Hello world!");
    REQUIRE(state.get_type() == LuaType::string);
    REQUIRE(state.pop_string() == "Hello world!");

    state.push("Hello\0world!"s);
    REQUIRE(state.get_type() == LuaType::string);
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
        REQUIRE(state.get_type() == LuaType::nil);
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

    REQUIRE(state.get_global("pippo") == LuaType::number);
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
        REQUIRE(state.get_type() == LuaType::table);

        state.push_number(1); // index to retrieve
        lua_gettable(state.get(), -2);
        REQUIRE(state.get_top() == 2); // 2 objects on stack (table + result)
        REQUIRE(state.pop_number() == 42.0);
    }
}


TEST_CASE("LuaType: Constants match LUA definitions")
{
    REQUIRE(static_cast<int>(LuaType::none) == LUA_TNONE);
    REQUIRE(static_cast<int>(LuaType::nil) == LUA_TNIL);
    REQUIRE(static_cast<int>(LuaType::boolean) == LUA_TBOOLEAN);
    REQUIRE(static_cast<int>(LuaType::light_user_data) == LUA_TLIGHTUSERDATA);
    REQUIRE(static_cast<int>(LuaType::number) == LUA_TNUMBER);
    REQUIRE(static_cast<int>(LuaType::string) == LUA_TSTRING);
    REQUIRE(static_cast<int>(LuaType::table) == LUA_TTABLE);
    REQUIRE(static_cast<int>(LuaType::function) == LUA_TFUNCTION);
    REQUIRE(static_cast<int>(LuaType::user_data) == LUA_TUSERDATA);
    REQUIRE(static_cast<int>(LuaType::thread) == LUA_TTHREAD);
}
