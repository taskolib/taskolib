/**
 * \file   test_execute_step.cc
 * \author Lars Froehlich
 * \date   Created on December 20, 2021
 * \brief  Test suite for the execute_step() function.
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

#include <gul14/catch.h>
#include <gul14/time_util.h>
#include "../include/avtomat/Error.h"
#include "../include/avtomat/execute_step.h"

using namespace std::literals;
using namespace avto;

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

    SECTION("Infinite loop is terminated")
    {
        auto t0 = gul14::tic();
        step.set_script("while true do end; return true");
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

    REQUIRE(step.get_time_of_last_execution() == Timestamp{});

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
        context["a"] = VariableValue{ 42LL };
        step.set_script("return a == 42");
        REQUIRE(execute_step(step, context) == false);
    }

    SECTION("Importing variables from an empty context")
    {
        step.set_imported_variable_names(VariableNames{ "a", "b" });
        step.set_script("return a == 42");
        REQUIRE(execute_step(step, context) == false);
    }

    SECTION("Importing an integer")
    {
        context["a"] = VariableValue{ 42LL };
        step.set_imported_variable_names(VariableNames{ "b", "a" });
        step.set_script("return a == 42");
        REQUIRE(execute_step(step, context) == true);
    }

    SECTION("Importing a double")
    {
        context["a"] = VariableValue{ 1.5 };
        step.set_imported_variable_names(VariableNames{ "b", "a" });
        step.set_script("return a == 1.5");
        REQUIRE(execute_step(step, context) == true);
    }

    SECTION("Importing a string")
    {
        context["a"] = VariableValue{ "Hello\0world"s };
        step.set_imported_variable_names(VariableNames{ "b", "a" });
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
        context["b"] = "Test";
        execute_step(step, context);
        REQUIRE(context.size() == 1);
        REQUIRE(std::get<std::string>(context["b"]) == "Test");
    }

    SECTION("Exporting an integer")
    {
        step.set_exported_variable_names(VariableNames{ "a" });
        execute_step(step, context);
        REQUIRE(context.size() == 1);
        REQUIRE(std::get<long long>(context["a"]) == 42);
        REQUIRE_THROWS_AS(std::get<double>(context["a"]), std::bad_variant_access);
        REQUIRE_THROWS_AS(std::get<std::string>(context["a"]), std::bad_variant_access);
    }

    SECTION("Exporting a double")
    {
        step.set_exported_variable_names(VariableNames{ "b" });
        execute_step(step, context);
        REQUIRE(context.size() == 1);

        REQUIRE_THROWS_AS(std::get<long long>(context["b"]), std::bad_variant_access);
        REQUIRE(std::get<double>(context["b"]) == 1.5);
        REQUIRE_THROWS_AS(std::get<std::string>(context["b"]), std::bad_variant_access);
    }

    SECTION("Exporting a string")
    {
        step.set_exported_variable_names(VariableNames{ "c" });
        execute_step(step, context);
        REQUIRE(context.size() == 1);
        REQUIRE_THROWS_AS(std::get<long long>(context["c"]), std::bad_variant_access);
        REQUIRE_THROWS_AS(std::get<double>(context["c"]), std::bad_variant_access);
        REQUIRE(std::get<std::string>(context["c"]) == "string");
    }

    SECTION("Exporting multiple variables")
    {
        step.set_exported_variable_names(VariableNames{ "c", "a", "b" });
        execute_step(step, context);
        REQUIRE(context.size() == 3);
        REQUIRE(std::get<long long>(context["a"]) == 42);
        REQUIRE(std::get<double>(context["b"]) == 1.5);
        REQUIRE(std::get<std::string>(context["c"]) == "string");
    }

    SECTION("Exporting unknown types")
    {
        step.set_exported_variable_names(VariableNames{ "d" });
        execute_step(step, context);
        REQUIRE(context.size() == 0); // d is of type function and does not get exported
    }

    SECTION("Exporting undefined variables")
    {
        step.set_exported_variable_names(VariableNames{ "n" });
        execute_step(step, context);
        REQUIRE(context.size() == 0); // n is undefined and does not get exported
    }
}

TEST_CASE("execute_step(): Running a step with multiple import and exports",
    "[execute_step]")
{
    Context context;
    Step step;

    step.set_imported_variable_names(VariableNames{ "str", "num_repetitions", "separator" });
    step.set_script(R"(
        if num_repetitions < 0 then
            return false
        end

        result = string.rep(str, num_repetitions, separator)

        return true
        )");
    step.set_exported_variable_names(VariableNames{ "result" });

    SECTION("Empty context")
    {
        REQUIRE_THROWS_AS(execute_step(step, context), Error); // Attempt to compare nil with number
        REQUIRE(context.empty() == true);
    }

    SECTION("num_repetitions < 0 returns false")
    {
        context["str"] = "Test";
        context["num_repetitions"] = -1LL;
        REQUIRE(execute_step(step, context) == false);
        REQUIRE(context.size() == 2);
        REQUIRE(std::get<std::string>(context["str"]) == "Test");
        REQUIRE(std::get<long long>(context["num_repetitions"]) == -1LL);
    }

    SECTION("num_repetitions == 0 returns empty string")
    {
        context["str"] = "Test";
        context["num_repetitions"] = 0LL;
        REQUIRE(execute_step(step, context) == true);
        REQUIRE(context.size() == 3);
        REQUIRE(std::get<std::string>(context["str"]) == "Test");
        REQUIRE(std::get<long long>(context["num_repetitions"]) == 0LL);
        REQUIRE(std::get<std::string>(context["result"]) == "");
    }

    SECTION("num_repetitions == 2 with separator")
    {
        context["str"] = "Test";
        context["num_repetitions"] = 2LL;
        context["separator"] = "|";
        REQUIRE(execute_step(step, context) == true);
        REQUIRE(context.size() == 4);
        REQUIRE(std::get<std::string>(context["str"]) == "Test");
        REQUIRE(std::get<long long>(context["num_repetitions"]) == 2LL);
        REQUIRE(std::get<std::string>(context["separator"]) == "|");
        REQUIRE(std::get<std::string>(context["result"]) == "Test|Test");
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
