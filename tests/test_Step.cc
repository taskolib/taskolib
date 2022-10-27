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

#include <stdexcept>
#include <type_traits>

#include <gul14/catch.h>
#include <gul14/time_util.h>

#include "taskomat/exceptions.h"
#include "taskomat/Step.h"

using namespace std::literals;
using namespace task;
using namespace Catch::Matchers;

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

    REQUIRE(&step.set_indentation_level(3) == &step);
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
    auto time1 = step.get_time_of_last_modification();
    REQUIRE(time1 > Clock::now() - 2s);
    REQUIRE(time1 < Clock::now() + 2s);

    REQUIRE(&step.set_label("Do nothing") == &step);
    REQUIRE(step.get_label() == "Do nothing");
    auto time2 = step.get_time_of_last_modification();
    REQUIRE(time2 > Clock::now() - 2s);
    REQUIRE(time2 < Clock::now() + 2s);
    REQUIRE(time2 > time1);

    step.set_label("Do something");
    REQUIRE(step.get_label() == "Do something");
    auto time3 = step.get_time_of_last_modification();
    REQUIRE(time3 > Clock::now() - 2s);
    REQUIRE(time3 < Clock::now() + 2s);
    REQUIRE(time3 > time2);
}

TEST_CASE("Step: set_running()", "[Step]")
{
    Step step;

    REQUIRE(&step.set_running(true) == &step);
    REQUIRE(step.is_running() == true);

    step.set_running(false);
    REQUIRE(step.is_running() == false);
}

TEST_CASE("Step: set_time_of_last_execution()", "[Step]")
{
    Step step;

    auto ts = Clock::now();
    REQUIRE(&step.set_time_of_last_execution(ts) == &step);
    REQUIRE(step.get_time_of_last_execution() == ts);

    step.set_time_of_last_execution(ts - 42s);
    REQUIRE(step.get_time_of_last_execution() == ts - 42s);
}

TEST_CASE("Step: set_time_of_last_modification()", "[Step]")
{
    Step step;

    auto ts = Clock::now();
    REQUIRE(&step.set_time_of_last_modification(ts) == &step);
    REQUIRE(step.get_time_of_last_modification() == ts);

    step.set_time_of_last_modification(ts - 42s);
    REQUIRE(step.get_time_of_last_modification() == ts - 42s);
}

TEST_CASE("Step: set_timeout()", "[Step]")
{
    Step step;

    REQUIRE(&step.set_timeout(42s) == &step);
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

    REQUIRE(&step.set_type(Step::type_while) == &step);
    REQUIRE(step.get_type() == Step::type_while);
    auto time1 = step.get_time_of_last_modification();
    REQUIRE(time1 > Clock::now() - 2s);
    REQUIRE(time1 < Clock::now() + 2s);
    REQUIRE(time1 > ts);

    step.set_type(Step::type_end);
    REQUIRE(step.get_type() == Step::type_end);
    auto time2 = step.get_time_of_last_modification();
    REQUIRE(time2 > Clock::now() - 2s);
    REQUIRE(time2 < Clock::now() + 2s);
    REQUIRE(time2 > time1);
}

TEST_CASE("Step: set_script()", "[Step]")
{
    Step step;
    auto ts = Clock::now() - 100s;
    step.set_time_of_last_modification(ts);
    REQUIRE(step.get_time_of_last_modification() == ts);

    REQUIRE(&step.set_script("test") == &step);
    REQUIRE(step.get_script() == "test");
    auto time1 = step.get_time_of_last_modification();
    REQUIRE(time1 > Clock::now() - 2s);
    REQUIRE(time1 < Clock::now() + 2s);
    REQUIRE(time1 > ts);

    step.set_script("test 2");
    REQUIRE(step.get_script() == "test 2");
    auto time2 = step.get_time_of_last_modification();
    REQUIRE(time2 > Clock::now() - 2s);
    REQUIRE(time2 < Clock::now() + 2s);
    REQUIRE(time2 > time1);
}

TEST_CASE("Step: set_used_context_variable_names()", "[Step]")
{
    Step step;

    REQUIRE(&step.set_used_context_variable_names(VariableNames{ "b52", "a" }) == &step);
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

TEST_CASE("execute(): Lua exceptions", "[Step]")
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
        step.set_script("function boom(); error('pippo', 0); end; boom()");
        try
        {
            step.execute(context); // Must throw
            FAIL("No exception thrown by Lua error()");
        }
        catch(const Error& e)
        {
            REQUIRE_THAT(e.what(), StartsWith("Script execution error: pippo"));
            // Lua adds a stack trace after this output. This is a somewhat brittle test,
            // but since we have control over our Lua version, we are sure to spot it if
            // the output format changes.
        }
    }

    SECTION("Runtime error, caught by pcall()")
    {
        step.set_script("function boom(); b = nil; b(); end; pcall(boom)");
        REQUIRE_NOTHROW(step.execute(context));
    }
}

TEST_CASE("execute(): C++ exceptions", "[Step]")
{
    Context context;

    Step step;
    step.set_used_context_variable_names(VariableNames{ "a" });

    context.variables["a"] = 0LL;
    context.lua_init_function =
        [](sol::state& sol)
        {
            sol["throw_logic_error"] = []() { throw std::logic_error("Test"); };
            sol["throw_weird_exception"] = []() { struct Weird{}; throw Weird{}; };
        };

    SECTION("C++ standard exception thrown at script runtime")
    {
        step.set_script("throw_logic_error(); a = 42");

        // A C++ exception bubbles through the Lua execution callstack, but must be caught
        // by Sol2, returned as a protected_function_result, and reported as a task::Error.
        REQUIRE_THROWS_AS(step.execute(context), Error);
        REQUIRE(std::get<long long>(context.variables["a"]) == 0);
    }

    SECTION("Nonstandard C++ exception thrown at script runtime")
    {
        step.set_script("throw_weird_exception(); a = 42");

        // A C++ exception bubbles through the Lua execution callstack, but must be caught
        // by Sol2, returned as a protected_function_result, and reported as a task::Error.
        REQUIRE_THROWS_AS(step.execute(context), Error);
        REQUIRE(std::get<long long>(context.variables["a"]) == 0);
    }

    SECTION("C++ exceptions are not caught by pcall()")
    {
        step.set_script("pcall(throw_logic_error); a = 42");

        // A C++ exception bubbles through the Lua execution callstack and cannot be
        // caught by pcall(). It must, however, be caught by Step::execute() and reported
        // as a task::Error.
        REQUIRE_THROWS_AS(step.execute(context), Error);
        REQUIRE(std::get<long long>(context.variables["a"]) == 0);
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

    SECTION("require() is not available")
    {
        step.set_script("require('io')");
        REQUIRE_THROWS_AS(step.execute(context), Error);
    }

    SECTION("io.write() is not available")
    {
        step.set_script("io.write(42)");
        REQUIRE_THROWS_AS(step.execute(context), Error);
    }
}

TEST_CASE("execute(): Custom commands", "[Step]")
{
    Context context;
    Step step;

    SECTION("sleep()")
    {
        step.set_script("sleep(0.001)");

        auto t0 = gul14::tic();
        step.execute(context);
        REQUIRE(gul14::toc(t0) >= 0.001);
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

    SECTION("sleep() is terminated")
    {
        auto t0 = gul14::tic();
        step.set_script("sleep(10)");
        step.set_timeout(5ms);
        REQUIRE_THROWS_AS(step.execute(context), Error);
        REQUIRE(gul14::toc<std::chrono::milliseconds>(t0) >= 5);
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

TEST_CASE("execute(): Immediate termination", "[Step]")
{
    Context context;
    context.variables["a"] = 0LL;

    Step step;
    CommChannel comm;

    context.lua_init_function =
        [&comm](sol::state& sol)
        {
            sol["request_termination"] =
                [&comm]
                {
                    comm.immediate_termination_requested_ = true;
                };
        };

    step.set_used_context_variable_names(VariableNames{ "a" });

    // The script needs a certain number of steps to make sure the termination
    // condition is even tested
    step.set_script("a = -1; request_termination(); for a = 1, 1000 do end; return true");

    REQUIRE_THROWS_AS(step.execute(context, &comm, 0), Error);
    REQUIRE(comm.immediate_termination_requested_);
    REQUIRE(std::get<long long>(context.variables["a"]) == 0LL);
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

TEST_CASE("execute(): Running a step with multiple import and exports", "[Step]")
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
    CommChannel comm;

    Context context;
    context.variables["a"] = 0LL;

    Step step;
    step.set_used_context_variable_names(VariableNames{ "a" });
    step.set_script("a = a + 42");

    step.execute(context, &comm, 42);

    REQUIRE(comm.queue_.size() == 2);

    // First, a "step started" message
    auto msg = comm.queue_.pop();
    REQUIRE(msg.get_type() == Message::Type::step_started);
    REQUIRE(msg.get_text() != "");
    REQUIRE(msg.get_timestamp() >= t0);
    REQUIRE(msg.get_timestamp() - t0 < 1s);
    REQUIRE(msg.get_index().has_value());
    REQUIRE(*(msg.get_index()) == 42);

    const auto t1 = msg.get_timestamp();

    // Then, a "step stopped" message
    msg = comm.queue_.pop();
    REQUIRE(msg.get_type() == Message::Type::step_stopped);
    REQUIRE(msg.get_text() != "");
    REQUIRE(msg.get_timestamp() >= t1);
    REQUIRE(msg.get_timestamp() - t1 < 1s);
    REQUIRE(msg.get_index().has_value());
    REQUIRE(*(msg.get_index()) == 42);
}

TEST_CASE("execute(): print function", "[Step]")
{
    std::string output;

    Context context;
    context.print_function =
        [&output](const std::string& str, OptionalStepIndex, CommChannel*)
        {
            output += str;
        };

    Step step;
    step.set_script("print('Hello', 42, '!')");

    step.execute(context);

    REQUIRE(output == "Hello\t42\t!\n");
}

TEST_CASE("Step: set_disabled()", "[Step]")
{
    Step step;

    REQUIRE(step.is_disabled() == false);
    REQUIRE(&step.set_disabled(true) == &step);
    REQUIRE(step.is_disabled() == true);

    // (There are no disable_unable types anymore)
    std::vector<Step::Type> disable_able =
        { Step::Type::type_catch, Step::Type::type_else, Step::Type::type_elseif, Step::Type::type_end,
        Step::Type::type_action, Step::Type::type_if, Step::Type::type_while, Step::Type::type_try };

    for (auto t : disable_able) {

        // set type first, then disabled
        step.set_type(Step::Type::type_action); // reset
        step.set_disabled(false); // reset
        step.set_type(t);
        REQUIRE(step.is_disabled() == false);
        step.set_disabled(true);
        REQUIRE(step.is_disabled() == true);
        REQUIRE(step.get_type() == t);

        // set disabled first, then type
        step.set_type(Step::Type::type_action); // reset
        step.set_disabled(false); // reset
        step.set_disabled(true);
        step.set_type(t);
        REQUIRE(step.is_disabled() == true);
        REQUIRE(step.get_type() == t);
    }
}

//
// Unit tests for free functions
//

TEST_CASE("to_string(Step::Type)", "[Step]")
{
    REQUIRE(to_string(Step::type_action) == "action");
    REQUIRE(to_string(Step::type_elseif) == "elseif");
    REQUIRE(to_string(static_cast<Step::Type>(127)) == "unknown");
}
