/**
 * \file   test_execute_lua_script.cc
 * \author Lars Fröhlich
 * \date   Created on October 28, 2022
 * \brief  Test suite for Lua-related internal functions.
 *
 * \copyright Copyright 2022-2024 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <gul14/catch.h>
#include <gul14/time_util.h>

#include "lua_details.h"
#include "taskolib/execute_lua_script.h"

using namespace std::literals;
using namespace task;
using namespace Catch::Matchers;

TEST_CASE("load_lua_script()", "[execute_lua_script]")
{
    sol::state lua;

    auto result = load_lua_script(lua, "return 42");
    REQUIRE(result.has_value());
    int function_call_result = (*result)();
    REQUIRE(function_call_result == 42);

    REQUIRE(load_lua_script(lua, "a = b").has_value());
    REQUIRE(load_lua_script(lua, "a = unknown.variable").has_value());
    REQUIRE(load_lua_script(lua, "a = 'asf").error() != "");
    REQUIRE(load_lua_script(lua, "a = asf'").error() != "");
    REQUIRE(load_lua_script(lua, "a = = 2").error() != "");
    REQUIRE(load_lua_script(lua, "Hello world!").error() != "");
}

TEST_CASE("execute_lua_script(): Return values from simple scripts without errors",
    "[execute_lua_script]")
{
    sol::state lua;

    SECTION("Empty script")
    {
        auto result = execute_lua_script(lua, "");
        REQUIRE(result.has_value());
        REQUIRE(*result == sol::nil);
    }

    SECTION("return nil")
    {
        auto result = execute_lua_script(lua, "return nil");
        REQUIRE(result.has_value());
        REQUIRE(*result == sol::nil);
    }

    SECTION("return true")
    {
        auto result = execute_lua_script(lua, "return true");
        REQUIRE(result.has_value());
        REQUIRE(result->as<bool>() == true);
    }

    SECTION("return false")
    {
        auto result = execute_lua_script(lua, "return false");
        REQUIRE(result.has_value());
        REQUIRE(result->as<bool>() == false);
    }

    SECTION("return 42")
    {
        auto result = execute_lua_script(lua, "return 42");
        REQUIRE(result.has_value());
        REQUIRE(result->as<int>() == 42);
    }

    SECTION("return 4.2")
    {
        auto result = execute_lua_script(lua, "return 4.2");
        REQUIRE(result.has_value());
        REQUIRE(result->as<double>() == 4.2);
    }

    SECTION("return 'pippo'")
    {
        auto result = execute_lua_script(lua, "return 'pippo'");
        REQUIRE(result.has_value());
        REQUIRE(result->as<std::string>() == "pippo");
    }
}

TEST_CASE("execute_lua_script(): Lua exceptions", "[execute_lua_script]")
{
    sol::state lua;
    open_safe_library_subset(lua); // for error() and pcall()

    SECTION("Syntax error")
    {
        auto result = execute_lua_script(lua, "not a lua program");
        REQUIRE(result.has_value() == false);
        REQUIRE(result.error() == "1: unexpected symbol near 'not'");
        // This test is somewhat brittle against changes of Lua syntax error messages.
        // This is intentional: We should realize early if a new Lua version introduces
        // weird output and decide if we need to pre-process it for our users.
    }

    SECTION("Runtime error with message")
    {
        auto result = execute_lua_script(
            lua, "function boom(); error('mindful' .. 'ness', 0); end; boom()");
        REQUIRE(result.has_value() == false);
        REQUIRE_THAT(result.error(), StartsWith("mindfulness"));
        // Lua adds a stack trace after this output. This is a somewhat brittle test,
        // but since we have control over our Lua version, we are sure to spot it if
        // the output format changes.
    }

    SECTION("Runtime error without message")
    {
        auto result = execute_lua_script(
            lua, "function boom(); error('', 0); end; boom()");
        REQUIRE(result.has_value() == false);
        REQUIRE_THAT(result.error(), not Contains("Unknown"));
        REQUIRE_THAT(result.error(), not Contains("exception"));
    }

    SECTION("Runtime error, caught by pcall()")
    {
        auto result = execute_lua_script(
            lua, "function boom(); b = nil; b(); end; pcall(boom); return 42");
        REQUIRE(result.has_value());
        REQUIRE(result->as<int>() == 42);
    }
}

TEST_CASE("execute_lua_script(): C++ exceptions", "[execute_lua_script]")
{
    sol::state lua;
    open_safe_library_subset(lua); // for error() and pcall()

    lua["throw_logic_error_with_msg"] = []() { throw std::logic_error("red rabbit"); };
    lua["throw_logic_error_without_msg"] = []() { throw std::logic_error(""); };
    lua["throw_weird_exception"] = []() { struct Weird{}; throw Weird{}; };

    SECTION("C++ standard exception with message")
    {
        auto result = execute_lua_script(lua, "throw_logic_error_with_msg()");
        REQUIRE(result.has_value() == false);
        REQUIRE_THAT(result.error(), StartsWith("red rabbit"));
    }

    SECTION("C++ standard exception without message")
    {
        auto result = execute_lua_script(lua, "throw_logic_error_without_msg()");
        REQUIRE(result.has_value() == false);
        REQUIRE_THAT(result.error(), not Contains("Unknown"));
        REQUIRE_THAT(result.error(), not Contains("exception"));
    }

    SECTION("Nonstandard C++ exceptions are reported as errors")
    {
        auto result = execute_lua_script(lua, "throw_weird_exception()");
        REQUIRE(result.has_value() == false);
        REQUIRE(result.error() == "Unknown exception");
    }

    SECTION("Standard C++ exceptions are converted to Lua errors and caught by pcall()")
    {
        auto result = execute_lua_script(lua, "pcall(throw_logic_error); return 42");
        REQUIRE(result.has_value());
        REQUIRE(result->as<int>() == 42);
    }

    SECTION("Nonstandard C++ exceptions are converted to Lua errors and caught by pcall()")
    {
        auto result = execute_lua_script(lua, "pcall(throw_weird_error); return 42");
        REQUIRE(result.has_value());
        REQUIRE(result->as<int>() == 42);
    }
}
