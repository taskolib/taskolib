/**
 * \file   test_Step.cc
 * \author Lars Froehlich
 * \date   Created on November 26, 2021
 * \brief  Test suite for the Step class.
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

#include <type_traits>
#include <gul14/catch.h>
#include <gul14/time_util.h>
#include "taskomat/Error.h"
#include "taskomat/Step.h"

using namespace std::literals;
using namespace task;

TEST_CASE("Step: Default constructor", "[Step]")
{
    static_assert(std::is_default_constructible<Step>::value,
        "Step is_default_constructible");

    Step s;
}

TEST_CASE("Step: get_indentation_level()", "[Step]")
{
    Step step;
    REQUIRE(step.get_indentation_level() == 0);

    step.set_indentation_level(3);
    REQUIRE(step.get_indentation_level() == 3);
}

TEST_CASE("Step: get_label()", "[Step]")
{
    Step step;
    REQUIRE(step.get_label() == "");
    step.set_label("Do nothing");
    REQUIRE(step.get_label() == "Do nothing");
}

TEST_CASE("Step: get_script()", "[Step]")
{
    Step step;
    REQUIRE(step.get_script() == "");
    step.set_script("a = 42");
    REQUIRE(step.get_script() == "a = 42");
}

TEST_CASE("Step: get_time_of_last_execution()", "[Step]")
{
    Step step;
    REQUIRE(step.get_time_of_last_execution() == TimePoint{});

    auto ts = Clock::now();
    step.set_time_of_last_execution(ts);
    REQUIRE(step.get_time_of_last_execution() == ts);
}

TEST_CASE("Step: get_time_of_last_modification()", "[Step]")
{
    Step step;
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);

    auto ts = Clock::now() + 100s;
    step.set_time_of_last_modification(ts);
    REQUIRE(step.get_time_of_last_modification() == ts);
}

TEST_CASE("Step: get_timeout()", "[Step]")
{
    Step step;
    REQUIRE(step.get_timeout() == Step::infinite_timeout);

    step.set_timeout(42s);
    REQUIRE(step.get_timeout() == 42'000ms);
}

TEST_CASE("Step: get_type()", "[Step]")
{
    Step step;
    REQUIRE(step.get_type() == Step::type_action);
    step.set_type(Step::type_catch);
    REQUIRE(step.get_type() == Step::type_catch);
    step.set_type(Step::type_if);
    REQUIRE(step.get_type() == Step::type_if);
}

TEST_CASE("Step: get_used_context_variable_names()", "[Step]")
{
    Step step;
    REQUIRE(step.get_used_context_variable_names().empty() == true);

    step.set_used_context_variable_names(VariableNames{ "b52", "a" });
    REQUIRE(step.get_used_context_variable_names() == VariableNames{ "a", "b52" });
}

TEST_CASE("Step: is_running()", "[Step]")
{
    Step step;
    REQUIRE(step.is_running() == false);

    step.set_running(true);
    REQUIRE(step.is_running() == true);
}

TEST_CASE("Step: set_indentation_level()", "[Step]")
{
    Step step;

    step.set_indentation_level(3);
    REQUIRE(step.get_indentation_level() == 3);

    step.set_indentation_level(0);
    REQUIRE(step.get_indentation_level() == 0);

    step.set_indentation_level(Step::max_indentation_level);
    REQUIRE(step.get_indentation_level() == Step::max_indentation_level);

    REQUIRE_THROWS_AS(step.set_indentation_level(-1), Error);
    REQUIRE_THROWS_AS(step.set_indentation_level(-42), Error);
    REQUIRE_THROWS_AS(step.set_indentation_level(Step::max_indentation_level + 1), Error);
    REQUIRE_THROWS_AS(step.set_indentation_level(30'000), Error);
}

TEST_CASE("Step: set_label()", "[Step]")
{
    Step step;
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);

    step.set_label("Do nothing");
    REQUIRE(step.get_label() == "Do nothing");
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);

    step.set_label("Do something");
    REQUIRE(step.get_label() == "Do something");
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);
}

TEST_CASE("Step: set_running()", "[Step]")
{
    Step step;

    step.set_running(true);
    REQUIRE(step.is_running() == true);

    step.set_running(false);
    REQUIRE(step.is_running() == false);
}

TEST_CASE("Step: set_time_of_last_execution()", "[Step]")
{
    Step step;

    auto ts = Clock::now();
    step.set_time_of_last_execution(ts);
    REQUIRE(step.get_time_of_last_execution() == ts);

    step.set_time_of_last_execution(ts - 42s);
    REQUIRE(step.get_time_of_last_execution() == ts - 42s);
}

TEST_CASE("Step: set_time_of_last_modification()", "[Step]")
{
    Step step;

    auto ts = Clock::now();
    step.set_time_of_last_modification(ts);
    REQUIRE(step.get_time_of_last_modification() == ts);

    step.set_time_of_last_modification(ts - 42s);
    REQUIRE(step.get_time_of_last_modification() == ts - 42s);
}

TEST_CASE("Step: set_timeout()", "[Step]")
{
    Step step;

    step.set_timeout(42s);
    REQUIRE(step.get_timeout() == 42s);

    step.set_timeout(-2ms);
    REQUIRE(step.get_timeout() == 0s);

    step.set_timeout(Step::infinite_timeout);
    REQUIRE(step.get_timeout() == Step::infinite_timeout);
}

TEST_CASE("Step: set_type()", "[Step]")
{
    Step step;
    auto ts = Clock::now() - 100s;
    step.set_time_of_last_modification(ts);
    REQUIRE(step.get_time_of_last_modification() == ts);

    step.set_type(Step::type_while);
    REQUIRE(step.get_type() == Step::type_while);
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);

    step.set_type(Step::type_end);
    REQUIRE(step.get_type() == Step::type_end);
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);
}

TEST_CASE("Step: set_script()", "[Step]")
{
    Step step;
    auto ts = Clock::now() - 100s;
    step.set_time_of_last_modification(ts);
    REQUIRE(step.get_time_of_last_modification() == ts);

    step.set_script("test");
    REQUIRE(step.get_script() == "test");
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);

    step.set_script("test 2");
    REQUIRE(step.get_script() == "test 2");
    REQUIRE(step.get_time_of_last_modification() > Clock::now() - 2s);
    REQUIRE(step.get_time_of_last_modification() < Clock::now() + 2s);
}

TEST_CASE("Step: set_used_context_variable_names()", "[Step]")
{
    Step step;

    step.set_used_context_variable_names(VariableNames{ "b52", "a" });
    REQUIRE(step.get_used_context_variable_names() == VariableNames{ "a", "b52" });
}

TEST_CASE("execute(): Boolean return value from simple scripts", "[Step]")
{
    Context context;
    Step step;

    SECTION("Empty step returns false")
    {
        REQUIRE(step.execute(context) == false);
    }

    SECTION("'return true' returns true")
    {
        step.set_script("return true");
        REQUIRE(step.execute(context) == true);
    }

    SECTION("'return false' returns false")
    {
        step.set_script("return true");
        REQUIRE(step.execute(context) == true);
    }

    SECTION("'return nil' returns false")
    {
        step.set_script("return nil");
        REQUIRE(step.execute(context) == false);
    }

    SECTION("'return 42' returns true")
    {
        step.set_script("return true");
        REQUIRE(step.execute(context) == true);
    }
}

TEST_CASE("execute(): Exceptions", "[Step]")
{
    Context context;
    Step step;

    SECTION("Syntax error")
    {
        step.set_script("not a lua program");
        REQUIRE_THROWS_AS(step.execute(context), Error);
    }

    SECTION("Runtime error")
    {
        step.set_script("b = nil; b()");
        REQUIRE_THROWS_AS(step.execute(context), Error);
    }
}

TEST_CASE("execute(): Sandboxing", "[Step]")
{
    Context context;
    Step step;

    SECTION("ipairs() is available")
    {
        step.set_script("a = {42, 43, 44}; for k,v in ipairs(a) do end; return true");
        REQUIRE(step.execute(context) == true);
    }

    SECTION("print() is not available")
    {
        step.set_script("print(42)");
        REQUIRE_THROWS_AS(step.execute(context), Error);
    }

    SECTION("io.write() is not available")
    {
        step.set_script("io.write(42)");
        REQUIRE_THROWS_AS(step.execute(context), Error);
    }
}

TEST_CASE("execute(): Timeout", "[Step]")
{
    Context context;
    Step step;

    SECTION("Simple infinite loop is terminated")
    {
        auto t0 = gul14::tic();
        step.set_script("while true do end; return true");
        step.set_timeout(20ms);
        REQUIRE_THROWS_AS(step.execute(context), Error);
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
        REQUIRE_THROWS_AS(step.execute(context), Error);
        REQUIRE(gul14::toc<std::chrono::milliseconds>(t0) >= 20);
        REQUIRE(gul14::toc<std::chrono::milliseconds>(t0) < 200); // leave some time for system hiccups
    }
}

TEST_CASE("execute(): Setting 'last executed' timestamp", "[Step]")
{
    Context context;
    Step step;
    auto ts = Clock::now() - 100s;
    step.set_time_of_last_modification(ts);
    REQUIRE(step.get_time_of_last_modification() == ts);

    step.execute(context);
    REQUIRE(Clock::now() - step.get_time_of_last_execution() >= 0s);
    REQUIRE(Clock::now() - step.get_time_of_last_execution() < 200ms); // leave some time for system hiccups
}

TEST_CASE("execute(): Importing variables from a context", "[Step]")
{
    Context context;
    Step step;

    SECTION("Importing nothing")
    {
        context.variables["a"] = VariableValue{ 42LL };
        step.set_script("return a == 42");
        REQUIRE(step.execute(context) == false);
    }

    SECTION("Importing variables from an empty context")
    {
        step.set_used_context_variable_names(VariableNames{ "a", "b" });
        step.set_script("return a == 42");
        REQUIRE(step.execute(context) == false);
    }

    SECTION("Importing an integer")
    {
        context.variables["a"] = VariableValue{ 42LL };
        step.set_used_context_variable_names(VariableNames{ "b", "a" });
        step.set_script("return a == 42");
        REQUIRE(step.execute(context) == true);
    }

    SECTION("Importing a double")
    {
        context.variables["a"] = VariableValue{ 1.5 };
        step.set_used_context_variable_names(VariableNames{ "b", "a" });
        step.set_script("return a == 1.5");
        REQUIRE(step.execute(context) == true);
    }

    SECTION("Importing a string")
    {
        context.variables["a"] = VariableValue{ "Hello\0world"s };
        step.set_used_context_variable_names(VariableNames{ "b", "a" });
        step.set_script("return a == 'Hello\\0world'");
        REQUIRE(step.execute(context) == true);
    }
}

TEST_CASE("execute(): Exporting variables into a context", "[Step]")
{
    Context context;
    Step step;

    step.set_script("a = 42; b = 1.5; c = 'string'; d = ipairs");

    SECTION("No exported variables")
    {
        context.variables["b"] = "Test";
        step.execute(context);
        REQUIRE(context.variables.size() == 1);
        REQUIRE(std::get<std::string>(context.variables["b"]) == "Test");
    }

    SECTION("Exporting an integer")
    {
        step.set_used_context_variable_names(VariableNames{ "a" });
        step.execute(context);
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
        step.execute(context);
        REQUIRE(context.variables.size() == 1);

        REQUIRE_THROWS_AS(std::get<long long>(context.variables["b"]), std::bad_variant_access);
        REQUIRE(std::get<double>(context.variables["b"]) == 1.5);
        REQUIRE_THROWS_AS(std::get<std::string>(context.variables["b"]), std::bad_variant_access);
    }

    SECTION("Exporting a string")
    {
        step.set_used_context_variable_names(VariableNames{ "c" });
        step.execute(context);
        REQUIRE(context.variables.size() == 1);
        REQUIRE_THROWS_AS(std::get<long long>(context.variables["c"]), std::bad_variant_access);
        REQUIRE_THROWS_AS(std::get<double>(context.variables["c"]), std::bad_variant_access);
        REQUIRE(std::get<std::string>(context.variables["c"]) == "string");
    }

    SECTION("Exporting multiple variables")
    {
        step.set_used_context_variable_names(VariableNames{ "c", "a", "b" });
        step.execute(context);
        REQUIRE(context.variables.size() == 3);
        REQUIRE(std::get<long long>(context.variables["a"]) == 42);
        REQUIRE(std::get<double>(context.variables["b"]) == 1.5);
        REQUIRE(std::get<std::string>(context.variables["c"]) == "string");
    }

    SECTION("Exporting unknown types")
    {
        step.set_used_context_variable_names(VariableNames{ "d" });
        step.execute(context);
        REQUIRE(context.variables.size() == 0); // d is of type function and does not get exported
    }

    SECTION("Exporting undefined variables")
    {
        step.set_used_context_variable_names(VariableNames{ "n" });
        step.execute(context);
        REQUIRE(context.variables.size() == 0); // n is undefined and does not get exported
    }
}

TEST_CASE("execute(): Running a step with multiple import and exports",
    "[Step]")
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
        REQUIRE_THROWS_AS(step.execute(context), Error); // Attempt to compare nil with number
        REQUIRE(context.variables.empty() == true);
    }

    SECTION("num_repetitions < 0 returns false")
    {
        context.variables["str"] = "Test";
        context.variables["num_repetitions"] = -1LL;
        REQUIRE(step.execute(context) == false);
        REQUIRE(context.variables.size() == 2);
        REQUIRE(std::get<std::string>(context.variables["str"]) == "Test");
        REQUIRE(std::get<long long>(context.variables["num_repetitions"]) == -1LL);
    }

    SECTION("num_repetitions == 0 returns empty string")
    {
        context.variables["str"] = "Test";
        context.variables["num_repetitions"] = 0LL;
        REQUIRE(step.execute(context) == true);
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
        REQUIRE(step.execute(context) == true);
        REQUIRE(context.variables.size() == 4);
        REQUIRE(std::get<std::string>(context.variables["str"]) == "Test");
        REQUIRE(std::get<long long>(context.variables["num_repetitions"]) == 2LL);
        REQUIRE(std::get<std::string>(context.variables["separator"]) == "|");
        REQUIRE(std::get<std::string>(context.variables["result"]) == "Test|Test");
    }
}

TEST_CASE("execute(): External commands on Lua scripts", "[Step]")
{
    Context context;
    Step step;

    step.set_script("os.execute('ls')");
    SECTION("Must prohibit execution of external command in Lua script")
    {
        REQUIRE_THROWS_AS(step.execute(context), Error);
    }
}

TEST_CASE("execute(): LUA initialization function", "[Step]")
{
    Context context;
    context.variables["a"] = 41LL;

    Step step;
    step.set_used_context_variable_names(VariableNames{ "a", "b" });
    step.set_script("a = a + 1");

    SECTION("Missing init function does not throw")
    {
        context.lua_init_function = nullptr;
        REQUIRE_NOTHROW(step.execute(context));
        REQUIRE(std::get<long long>(context.variables["a"]) == 42LL);
    }

    SECTION("Init function injecting a variable")
    {
        context.lua_init_function = [](sol::state& s) { s["b"] = 13; };
        REQUIRE_NOTHROW(step.execute(context));
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
        REQUIRE_NOTHROW(step.execute(context));
        REQUIRE(std::get<long long>(context.variables["a"]) == 42LL);
        REQUIRE(std::get<long long>(context.variables["b"]) == 13LL);
    }
}

TEST_CASE("execute(): Messages", "[Step]")
{
    const auto t0 = Clock::now();
    MessageQueue queue(10);

    Context context;
    context.variables["a"] = 0LL;

    Step step;
    step.set_used_context_variable_names(VariableNames{ "a" });
    step.set_script("a = a + 42");

    step.execute(context, &queue, 42);

    REQUIRE(queue.size() == 2);

    // First, a "step started" message
    auto msg = queue.pop();
    REQUIRE(msg.get_type() == Message::Type::step_started);
    REQUIRE(msg.get_text() != "");
    REQUIRE(msg.get_timestamp() >= t0);
    REQUIRE(msg.get_timestamp() - t0 < 1s);
    REQUIRE(msg.get_index() == 42);

    const auto t1 = msg.get_timestamp();

    // Then, a "step stopped" message
    msg = queue.pop();
    REQUIRE(msg.get_type() == Message::Type::step_stopped);
    REQUIRE(msg.get_text() != "");
    REQUIRE(msg.get_timestamp() >= t1);
    REQUIRE(msg.get_timestamp() - t1 < 1s);
    REQUIRE(msg.get_index() == 42);
}
