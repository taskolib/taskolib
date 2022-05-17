/**
 * \file   test_execute_step.cc
 * \author Lars Froehlich
 * \date   Created on December 20, 2021
 * \brief  Test suite for the execute_step() function.
 *
 * \copyright Copyright 2021-2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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
#include "taskomat/sol/sol/sol.hpp"
#include "taskomat/Error.h"
#include "taskomat/execute_step.h"

using namespace std::literals;
using namespace task;

TEST_CASE("execute_step(): Boolean return value from simple scripts", "[execute_step]")
{
    Context context;
    Step step;

    SECTION("Empty step returns false")
    {
        REQUIRE(execute_step(step, context) == false);
    }

    SECTION("'return true' returns true")
    {
        step.set_script("return true");
        REQUIRE(execute_step(step, context) == true);
    }

    SECTION("'return false' returns false")
    {
        step.set_script("return true");
        REQUIRE(execute_step(step, context) == true);
    }

    SECTION("'return nil' returns false")
    {
        step.set_script("return nil");
        REQUIRE(execute_step(step, context) == false);
    }

    SECTION("'return 42' returns true")
    {
        step.set_script("return true");
        REQUIRE(execute_step(step, context) == true);
    }
}

TEST_CASE("execute_step(): Exceptions", "[execute_step]")
{
    Context context;
    Step step;

    SECTION("Syntax error")
    {
        step.set_script("not a lua program");
        REQUIRE_THROWS_AS(execute_step(step, context), Error);
    }

    SECTION("Runtime error")
    {
        step.set_script("b = nil; b()");
        REQUIRE_THROWS_AS(execute_step(step, context), Error);
    }
}

TEST_CASE("execute_step(): Sandboxing", "[execute_step]")
{
    Context context;
    Step step;

    SECTION("ipairs() is available")
    {
        step.set_script("a = {42, 43, 44}; for k,v in ipairs(a) do end; return true");
        REQUIRE(execute_step(step, context) == true);
    }

    SECTION("print() is not available")
    {
        step.set_script("print(42)");
        REQUIRE_THROWS_AS(execute_step(step, context), Error);
    }

    SECTION("io.write() is not available")
    {
        step.set_script("io.write(42)");
        REQUIRE_THROWS_AS(execute_step(step, context), Error);
    }
}

TEST_CASE("execute_step(): Timeout", "[execute_step]")
{
    Context context;
    Step step;

    SECTION("Simple infinite loop is terminated")
    {
        auto t0 = gul14::tic();
        step.set_script("while true do end; return true");
        step.set_timeout(20ms);
        REQUIRE_THROWS_AS(execute_step(step, context), Error);
        REQUIRE(gul14::toc<std::chrono::milliseconds>(t0) >= 20);
        REQUIRE(gul14::toc<std::chrono::milliseconds>(t0) < 200); // leave some time for system hiccups
    }

    SECTION("Infinite loop is terminated despite pcall protection")
    {
        auto t0 = gul14::tic();
        step.set_script(R"(
            local function infinite_loop()
                while true do
                    for i = 1, 10000 do
                    end
                end
            end

            while true do
                pcall(infinite_loop)
            end
            )");
        step.set_timeout(20ms);
        REQUIRE_THROWS_AS(execute_step(step, context), Error);
        REQUIRE(gul14::toc<std::chrono::milliseconds>(t0) >= 20);
        REQUIRE(gul14::toc<std::chrono::milliseconds>(t0) < 200); // leave some time for system hiccups
    }
}

TEST_CASE("execute_step(): Setting 'last executed' timestamp", "[execute_step]")
{
    Context context;
    Step step;

    REQUIRE(step.get_time_of_last_execution() == TimePoint{});

    execute_step(step, context);
    REQUIRE(Clock::now() - step.get_time_of_last_execution() >= 0s);
    REQUIRE(Clock::now() - step.get_time_of_last_execution() < 200ms); // leave some time for system hiccups
}

TEST_CASE("execute_step(): Importing variables from a context", "[execute_step]")
{
    Context context;
    Step step;

    SECTION("Importing nothing")
    {
        context.variables["a"] = VariableValue{ 42LL };
        step.set_script("return a == 42");
        REQUIRE(execute_step(step, context) == false);
    }

    SECTION("Importing variables from an empty context")
    {
        step.set_used_context_variable_names(VariableNames{ "a", "b" });
        step.set_script("return a == 42");
        REQUIRE(execute_step(step, context) == false);
    }

    SECTION("Importing an integer")
    {
        context.variables["a"] = VariableValue{ 42LL };
        step.set_used_context_variable_names(VariableNames{ "b", "a" });
        step.set_script("return a == 42");
        REQUIRE(execute_step(step, context) == true);
    }

    SECTION("Importing a double")
    {
        context.variables["a"] = VariableValue{ 1.5 };
        step.set_used_context_variable_names(VariableNames{ "b", "a" });
        step.set_script("return a == 1.5");
        REQUIRE(execute_step(step, context) == true);
    }

    SECTION("Importing a string")
    {
        context.variables["a"] = VariableValue{ "Hello\0world"s };
        step.set_used_context_variable_names(VariableNames{ "b", "a" });
        step.set_script("return a == 'Hello\\0world'");
        REQUIRE(execute_step(step, context) == true);
    }
}

TEST_CASE("execute_step(): Exporting variables into a context", "[execute_step]")
{
    Context context;
    Step step;

    step.set_script("a = 42; b = 1.5; c = 'string'; d = ipairs");

    SECTION("No exported variables")
    {
        context.variables["b"] = "Test";
        execute_step(step, context);
        REQUIRE(context.variables.size() == 1);
        REQUIRE(std::get<std::string>(context.variables["b"]) == "Test");
    }

    SECTION("Exporting an integer")
    {
        step.set_used_context_variable_names(VariableNames{ "a" });
        execute_step(step, context);
        REQUIRE(context.variables.size() == 1);
        REQUIRE(std::get<long long>(context.variables["a"]) == 42);
        REQUIRE_THROWS_AS(std::get<double>(context.variables["a"]),
                          std::bad_variant_access);
        REQUIRE_THROWS_AS(std::get<std::string>(context.variables["a"]),
                          std::bad_variant_access);
    }

    SECTION("Exporting a double")
    {
        step.set_used_context_variable_names(VariableNames{ "b" });
        execute_step(step, context);
        REQUIRE(context.variables.size() == 1);

        REQUIRE_THROWS_AS(std::get<long long>(context.variables["b"]), std::bad_variant_access);
        REQUIRE(std::get<double>(context.variables["b"]) == 1.5);
        REQUIRE_THROWS_AS(std::get<std::string>(context.variables["b"]), std::bad_variant_access);
    }

    SECTION("Exporting a string")
    {
        step.set_used_context_variable_names(VariableNames{ "c" });
        execute_step(step, context);
        REQUIRE(context.variables.size() == 1);
        REQUIRE_THROWS_AS(std::get<long long>(context.variables["c"]), std::bad_variant_access);
        REQUIRE_THROWS_AS(std::get<double>(context.variables["c"]), std::bad_variant_access);
        REQUIRE(std::get<std::string>(context.variables["c"]) == "string");
    }

    SECTION("Exporting multiple variables")
    {
        step.set_used_context_variable_names(VariableNames{ "c", "a", "b" });
        execute_step(step, context);
        REQUIRE(context.variables.size() == 3);
        REQUIRE(std::get<long long>(context.variables["a"]) == 42);
        REQUIRE(std::get<double>(context.variables["b"]) == 1.5);
        REQUIRE(std::get<std::string>(context.variables["c"]) == "string");
    }

    SECTION("Exporting unknown types")
    {
        step.set_used_context_variable_names(VariableNames{ "d" });
        execute_step(step, context);
        REQUIRE(context.variables.size() == 0); // d is of type function and does not get exported
    }

    SECTION("Exporting undefined variables")
    {
        step.set_used_context_variable_names(VariableNames{ "n" });
        execute_step(step, context);
        REQUIRE(context.variables.size() == 0); // n is undefined and does not get exported
    }
}

TEST_CASE("execute_step(): Running a step with multiple import and exports",
    "[execute_step]")
{
    Context context;
    Step step;

    step.set_used_context_variable_names(
        VariableNames{ "str", "num_repetitions", "separator", "result" });
    step.set_script(R"(
        if num_repetitions < 0 then
            return false
        end

        result = string.rep(str, num_repetitions, separator)

        return true
        )");

    SECTION("Empty context")
    {
        REQUIRE_THROWS_AS(execute_step(step, context), Error); // Attempt to compare nil with number
        REQUIRE(context.variables.empty() == true);
    }

    SECTION("num_repetitions < 0 returns false")
    {
        context.variables["str"] = "Test";
        context.variables["num_repetitions"] = -1LL;
        REQUIRE(execute_step(step, context) == false);
        REQUIRE(context.variables.size() == 2);
        REQUIRE(std::get<std::string>(context.variables["str"]) == "Test");
        REQUIRE(std::get<long long>(context.variables["num_repetitions"]) == -1LL);
    }

    SECTION("num_repetitions == 0 returns empty string")
    {
        context.variables["str"] = "Test";
        context.variables["num_repetitions"] = 0LL;
        REQUIRE(execute_step(step, context) == true);
        REQUIRE(context.variables.size() == 3);
        REQUIRE(std::get<std::string>(context.variables["str"]) == "Test");
        REQUIRE(std::get<long long>(context.variables["num_repetitions"]) == 0LL);
        REQUIRE(std::get<std::string>(context.variables["result"]) == "");
    }

    SECTION("num_repetitions == 2 with separator")
    {
        context.variables["str"] = "Test";
        context.variables["num_repetitions"] = 2LL;
        context.variables["separator"] = "|";
        REQUIRE(execute_step(step, context) == true);
        REQUIRE(context.variables.size() == 4);
        REQUIRE(std::get<std::string>(context.variables["str"]) == "Test");
        REQUIRE(std::get<long long>(context.variables["num_repetitions"]) == 2LL);
        REQUIRE(std::get<std::string>(context.variables["separator"]) == "|");
        REQUIRE(std::get<std::string>(context.variables["result"]) == "Test|Test");
    }
}

TEST_CASE("execute_step(): External commands on Lua scripts", "[execute_step]")
{
    Context context;
    Step step;

    step.set_script("os.execute('ls')");
    SECTION("Must prohibit execution of external command in Lua script")
    {
        REQUIRE_THROWS_AS(execute_step(step, context), Error);
    }
}

TEST_CASE("execute_step(): LUA initialization function", "[execute_step]")
{
    Context context;
    context.variables["a"] = 41LL;

    Step step;
    step.set_used_context_variable_names(VariableNames{ "a", "b" });
    step.set_script("a = a + 1");

    SECTION("Missing init function does not throw")
    {
        context.lua_init_function = nullptr;
        REQUIRE_NOTHROW(execute_step(step, context));
        REQUIRE(std::get<long long>(context.variables["a"]) == 42LL);
    }

    SECTION("Init function injecting a variable")
    {
        context.lua_init_function = [](sol::state& s) { s["b"] = 13; };
        REQUIRE_NOTHROW(execute_step(step, context));
        REQUIRE(std::get<long long>(context.variables["b"]) == 13LL);
        REQUIRE(std::get<long long>(context.variables["a"]) == 42LL);
    }

    SECTION("Init function injecting a function")
    {
        context.lua_init_function =
            [](sol::state& s)
            {
                s["f"] = [](long long n) { return n + 1LL; };
            };
        step.set_script("b = f(12); a = a + 1");
        REQUIRE_NOTHROW(execute_step(step, context));
        REQUIRE(std::get<long long>(context.variables["a"]) == 42LL);
        REQUIRE(std::get<long long>(context.variables["b"]) == 13LL);
    }
}

TEST_CASE("execute_step(): Messages", "[execute_step]")
{
    const auto t0 = Clock::now();
    MessageQueue queue(10);

    Context context;
    context.variables["a"] = 0LL;

    Step step;
    step.set_used_context_variable_names(VariableNames{ "a" });
    step.set_script("a = a + 42");

    execute_step(step, context, &queue);

    REQUIRE(queue.size() == 2);

    // First, a "step started" message
    auto msg = queue.pop();
    REQUIRE(msg.get_type() == Message::Type::step_started);
    REQUIRE(msg.get_text() != "");
    REQUIRE(msg.get_timestamp() >= t0);
    REQUIRE(msg.get_timestamp() - t0 < 1s);

    const auto t1 = msg.get_timestamp();

    // Then, a "step stopped" message
    msg = queue.pop();
    REQUIRE(msg.get_type() == Message::Type::step_stopped);
    REQUIRE(msg.get_text() != "");
    REQUIRE(msg.get_timestamp() >= t1);
    REQUIRE(msg.get_timestamp() - t1 < 1s);
}
