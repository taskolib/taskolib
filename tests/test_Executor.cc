/**
 * \file   test_Executor.cc
 * \author Lars Froehlich, Marcus Walla
 * \date   Created on May 30, 2022
 * \brief  Test suite for the Executor class.
 *
 * \copyright Copyright 2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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
#include <gul14/substring_checks.h>
#include <gul14/time_util.h>
#include "taskolib/Executor.h"

using namespace task;
using namespace std::literals;

TEST_CASE("Executor: Constructor", "[Executor]")
{
    Executor ex;
}

TEST_CASE("Executor: Absence of copy constructor and copy assignment", "[Executor]")
{
    static_assert(not std::is_copy_constructible<Executor>::value,
                  "Executor may not be copy-constructible");
    static_assert(not std::is_copy_assignable<Executor>::value,
                  "Executor may not be copy-assignable");
}

TEST_CASE("Executor: Move constructor", "[Executor]")
{
    Executor ex;
    Executor ex2(std::move(ex));
}

TEST_CASE("Executor: Move assignment", "[Executor]")
{
    Executor ex;
    Executor ex2;

    ex2 = std::move(ex);
}

TEST_CASE("Executor: Run a sequence asynchronously", "[Executor]")
{
    Context context;

    Step step(Step::type_action);
    step.set_script("sleep(0.02)");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(std::move(step));
    REQUIRE(sequence.get_error_message() == "");

    Executor executor;

    REQUIRE(sequence.is_running() == false);
    for (const auto& step : sequence)
        REQUIRE(step.is_running() == false);

    const auto t0 = gul14::tic();

    // Start the sequence in a separate thread
    executor.run_asynchronously(sequence, context);

    // Starting another sequence must fail because the first one is still running
    REQUIRE_THROWS_AS(executor.run_asynchronously(sequence, context), Error);

    // As long as the thread is running, update() and is_busy() must return true,
    // and the sequence must signalize is_running().
    REQUIRE(executor.is_busy() == true);
    REQUIRE(executor.update(sequence) == true);
    REQUIRE(sequence.is_running() == true);

    bool have_seen_running_step =
        std::any_of(sequence.begin(), sequence.end(),
                    [](const Step& s) { return s.is_running(); });

    // Process messages as long as the thread is running
    while (executor.update(sequence))
    {
        have_seen_running_step |=
            std::any_of(sequence.begin(), sequence.end(),
                        [](const Step& s) { return s.is_running(); });

        gul14::sleep(5ms);
    }

    // We must have seen a step marked as "is_running" at least once during execution.
    REQUIRE(have_seen_running_step == true);

    REQUIRE(gul14::toc(t0) >= 0.02);

    // Thread has now finished. As long as we do not start another one, update() and
    // is_busy() keep returning false.
    REQUIRE(executor.update(sequence) == false);
    REQUIRE(executor.is_busy() == false);

    // Both the sequence and all of its steps must show is_running() == false.
    REQUIRE(sequence.is_running() == false);
    for (const auto& step : sequence)
        REQUIRE(step.is_running() == false);

    REQUIRE(sequence.get_error_message() == "");
}

TEST_CASE("Executor: Run a failing sequence asynchronously", "[Executor]")
{
    Context context;
    context.log_error_function = nullptr;

    Step step(Step::type_action);
    step.set_script("not valid LUA");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(std::move(step));
    REQUIRE(sequence.get_error_message() == "");

    sequence.set_error_message("Test");

    Executor executor;
    executor.run_asynchronously(sequence, context);

    // The sequence must signalize is_running() == true and an empty error message at
    // least until the first call to is_busy() or update().
    REQUIRE(sequence.is_running() == true);
    REQUIRE(sequence.get_error_message() == "");

    // Process messages as long as the thread is running
    while (executor.update(sequence))
        gul14::sleep(5ms);

    // Thread has now finished. As long as we do not start another one, update() and
    // is_busy() keep returning false.
    REQUIRE(executor.update(sequence) == false);
    REQUIRE(executor.is_busy() == false);

    REQUIRE(sequence.get_error_message() != "");
}

TEST_CASE("Executor: cancel() within LUA sleep()", "[Executor]")
{
    Context context;
    context.log_error_function = nullptr; // suppress error output on console

    Step step(Step::type_action);
    step.set_script("sleep(2)");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(std::move(step));

    Executor executor;

    const auto t0 = gul14::tic();

    executor.run_asynchronously(sequence, context);

    gul14::sleep(5ms);
    executor.cancel();

    REQUIRE(gul14::toc(t0) >= 0.005);
    REQUIRE(gul14::toc(t0) < 0.2);

    // Thread has now finished. As long as we do not start another one, update() and
    // is_busy() keep returning false
    REQUIRE(executor.update(sequence) == false);
    REQUIRE(executor.is_busy() == false);

    for (const auto& step : sequence)
        REQUIRE(step.is_running() == false);

    // Make sure that exactly the desired error message comes out
    REQUIRE(sequence.get_error_message() == "Sequence aborted: Stop on user request");
}

TEST_CASE("Executor: cancel() within pcalls and CATCH blocks", "[Executor]")
{
    Context context;
    context.log_error_function = nullptr; // suppress error output on console

    Step step_while{ Step::type_while };
    step_while.set_script("return true");
    Step step_try{ Step::type_try };
    Step step_action( Step::type_action );
    step_action.set_script(R"(
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
    Step step_catch{ Step::type_catch };

    Sequence sequence{ "test_sequence" };
    sequence.push_back(std::move(step_while));  // while
    sequence.push_back(std::move(step_try));    //   try
    sequence.push_back(std::move(step_action)); //     action: infinite loop
    sequence.push_back(std::move(step_catch));  //   catch
    sequence.push_back(Step{ Step::type_end }); //   end
    sequence.push_back(Step{ Step::type_end }); // end

    Executor executor;

    const auto t0 = gul14::tic();

    executor.run_asynchronously(sequence, context);

    gul14::sleep(5ms);
    executor.cancel();

    REQUIRE(gul14::toc(t0) >= 0.005);
    REQUIRE(gul14::toc(t0) < 0.2);

    // Thread has now finished. As long as we do not start another one, update() and
    // is_busy() keep returning false
    REQUIRE(executor.update(sequence) == false);
    REQUIRE(executor.is_busy() == false);

    for (const auto& step : sequence)
    {
        CAPTURE(step.get_type());
        REQUIRE(step.is_running() == false);
    }
}

TEST_CASE("Executor: is_busy() on newly constructed Executor", "[Executor]")
{
    Executor ex;
    REQUIRE(ex.is_busy() == false);
}

TEST_CASE("Executor: Redirection of print() output", "[Executor]")
{
    std::string output;

    Context context;
    context.print_function =
        [&output](const std::string& str, OptionalStepIndex, CommChannel*)
        {
            output += str;
        };

    Step step( Step::type_action );
    step.set_script("print('Mary had', 3, 'little lambs.')");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(std::move(step));

    Executor executor;

    executor.run_asynchronously(sequence, context);

    SECTION("Regularly calling update()")
    {
        while (executor.update(sequence))
            gul14::sleep(5ms);

        REQUIRE(output == "Mary had\t3\tlittle lambs.\n");
    }

    SECTION("Regularly calling busy() and update() only at the end")
    {
        while (executor.is_busy())
            gul14::sleep(5ms);
        executor.update(sequence);

        // Causes a test failure with a buggy implementation of is_busy()
        REQUIRE(output == "Mary had\t3\tlittle lambs.\n");
    }
}

TEST_CASE("Executor: Access context after run", "[Executor]")
{
    Context ctx;
    ctx.variables["a"] = VariableValue{ 1LL };

    Sequence seq{ "a Seq" };
    seq.push_back(Step{ Step::type_while  }.set_script("return a % 10 ~= 0"));
    seq.push_back(Step{ Step::type_action }.set_script("a = a + 1"));
    seq.push_back(Step{ Step::type_end });
    seq.push_back(Step{ Step::type_action }.set_script("a = a + 1"));

    for (auto s = seq.begin(); s != seq.end(); ++s)
        seq.modify(s, [](Step& step) { step.set_used_context_variable_names(VariableNames{ "a" }); });

    // Execute directly
    seq.execute(ctx, nullptr);
    REQUIRE(std::get<long long>(ctx.variables["a"] ) == 11LL );

    // Execute async
    Executor executor{ };
    executor.run_asynchronously(seq, ctx);
    while (executor.update(seq))
        gul14::sleep(5ms);
    auto vars = executor.get_context_variables();
    REQUIRE(std::get<long long>(vars["a"]) == 21LL);
}

TEST_CASE("Executor: Run a sequence asynchronously with explict termination",
    "[Executor]")
{
    Context ctx;
    ctx.log_error_function = nullptr;

    Sequence seq{ "test_sequence" };

    Step step_while{Step::type_while};
    Step step_increment{Step::type_action};
    Step step_sleep{Step::type_action};
    Step step_check_termination{Step::type_action};
    Step step_while_end{Step::type_end};

    step_while.set_label("loop until a >= 10");
    step_while.set_script("return a < 10");
    step_while.set_used_context_variable_names(VariableNames{"a"});

    step_increment.set_label("increment a");
    step_increment.set_script("a = a +1");
    step_increment.set_used_context_variable_names(VariableNames{"a"});

    step_sleep.set_label("sleep 10ms");
    step_sleep.set_script("sleep(0.01)");

    step_check_termination.set_label("exit sequence when a == 4");
    step_check_termination.set_script("if a == 4 then terminate_sequence() end");
    step_check_termination.set_used_context_variable_names(VariableNames{"a"});

    step_while_end.set_label("end loop");

    ctx.variables["a"] = VariableValue{ 0LL };

    seq.push_back(step_while);
    seq.push_back(step_increment);
    seq.push_back(step_sleep);
    seq.push_back(step_check_termination);
    seq.push_back(step_while_end);

    Executor executor;

    // Start the sequence in a separate thread
    executor.run_asynchronously(seq, ctx);

    // As long as the thread is running, update() and is_busy() must return true,
    // and the sequence must signalize is_running().
    REQUIRE(executor.is_busy() == true);
    REQUIRE(executor.update(seq) == true);
    REQUIRE(seq.is_running() == true);

    // Process messages as long as the thread is running
    while (executor.update(seq))
        gul14::sleep(5ms);

    // Thread has now finished. As long as we do not start another one, update() and
    // is_busy() keep returning false.
    REQUIRE(executor.update(seq) == false);
    REQUIRE(executor.is_busy() == false);

    // Both the sequence and all of its steps must show is_running() == false.
    REQUIRE(seq.is_running() == false);
    for (const auto& step : seq)
        REQUIRE(step.is_running() == false);

    REQUIRE(seq.get_error_message() == "");
}

// A function that would return the integer value 10 in Lua
// if we do not throw ;-)
auto lua_testfun(sol::this_state s) -> sol::object
{
    throw task::Error{ "Rainbows!" };
    return sol::make_object(s, 10);
}

TEST_CASE("Executor: Run a sequence asynchronously with throw", "[Executor]")
{
    Context ctx;
    ctx.log_error_function = nullptr;

    ctx.lua_init_function = [](sol::state& s) {
        s.set_function("testfun", &lua_testfun);
    };

    Sequence seq{ "Test" };
    Step step{ Step::type_action };
    step.set_script("a = testfun()");

    seq.push_back(step);

    // I. direct execution

    REQUIRE_THROWS_AS(seq.execute(ctx, nullptr), task::Error);
    try {
        seq.execute(ctx, nullptr);
    } catch (task::Error const& e) {
        INFO("what is:");
        INFO(e.what());
        REQUIRE(gul14::contains(e.what(), "Rainbows"));
    }

    // II. run async
    Executor executor{ };
    executor.run_asynchronously(seq, ctx);

    REQUIRE(seq.is_running() == true);

    while (executor.update(seq))
        gul14::sleep(5ms);

    REQUIRE(executor.update(seq) == false);
    REQUIRE(executor.is_busy() == false);

    REQUIRE(gul14::contains(seq.get_error_message(), "Rainbows"));
}
