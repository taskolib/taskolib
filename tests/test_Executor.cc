/**
 * \file   test_Executor.cc
 * \author Lars Fröhlich, Marcus Walla
 * \date   Created on May 30, 2022
 * \brief  Test suite for the Executor class.
 *
 * \copyright Copyright 2022-2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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
using Catch::Matchers::Contains;

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

TEST_CASE("Executor: run_asynchronously()", "[Executor]")
{
    Context context;

    Step step(Step::type_action);
    step.set_script("sleep(0.02)");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(std::move(step));
    REQUIRE(sequence.get_error().has_value() == false);

    Executor executor;

    REQUIRE(sequence.is_running() == false);
    for (const auto& step : sequence)
        REQUIRE(step.is_running() == false);

    const auto t0 = gul14::tic();

    // Start the sequence in a separate thread
    executor.run_asynchronously(sequence, context);

    // Starting another sequence must fail because the first one is still running
    REQUIRE_THROWS_AS(executor.run_asynchronously(sequence, context), Error);

    // As long as the thread is running, update() must return true, and the sequence must
    // signalize is_running().
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

    // Thread has now finished. As long as we do not start another one, update() keeps
    // returning false.
    REQUIRE(executor.update(sequence) == false);

    // Both the sequence and all of its steps must show is_running() == false.
    REQUIRE(sequence.is_running() == false);
    for (const auto& step : sequence)
        REQUIRE(step.is_running() == false);

    REQUIRE(sequence.get_error().has_value() == false);
}

TEST_CASE("Executor: run_asynchronously(), failing sequence", "[Executor]")
{
    Context context;
    context.message_callback_function = nullptr;

    Step step(Step::type_action);
    step.set_script("not valid Lua");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(std::move(step));
    REQUIRE(sequence.get_error().has_value() == false);

    sequence.set_error(Error{ "Test", 42 });

    Executor executor;
    executor.run_asynchronously(sequence, context);

    // The sequence must signalize is_running() == true and an empty error message at
    // least until the first call to is_busy() or update().
    REQUIRE(sequence.is_running() == true);
    REQUIRE(sequence.get_error().has_value() == false);

    // Process messages as long as the thread is running
    while (executor.update(sequence))
        gul14::sleep(5ms);

    // Thread has now finished. As long as we do not start another one, update() keeps
    // returning false.
    REQUIRE(executor.update(sequence) == false);

    REQUIRE(sequence.get_error().has_value() == true);
    REQUIRE(sequence.get_error()->what() != ""s);
    REQUIRE(sequence.get_error()->get_index().has_value());
    REQUIRE(sequence.get_error()->get_index().value() == 0);
}

TEST_CASE("Executor: run_single_step_asynchronously()", "[Executor]")
{
    Context context;
    context.message_callback_function = nullptr;

    Step step1{ Step::type_action };
    step1.set_script("a = 1; error('Waldeinsamkeit')").set_used_context_variable_names(VariableNames{ "a" });
    Step step2{ Step::type_action };
    step2.set_script("a = 2; sleep(0.02)").set_used_context_variable_names(VariableNames{ "a" });

    Sequence sequence{ "test_sequence" };
    sequence.push_back(std::move(step1));
    sequence.push_back(std::move(step2));
    REQUIRE(sequence.get_error().has_value() == false);

    Executor executor;

    REQUIRE(sequence.is_running() == false);
    for (const auto& step : sequence)
        REQUIRE(step.is_running() == false);

    SECTION("Running a step successfully")
    {
        const auto t0 = gul14::tic();

        // Invalid step index must throw
        REQUIRE_THROWS_AS(
            executor.run_single_step_asynchronously(sequence, context, StepIndex{ 2 }), Error);

        // Start the second step in a separate thread
        executor.run_single_step_asynchronously(sequence, context, StepIndex{ 1 });

        // Starting another sequence must fail because the first one is still running
        REQUIRE_THROWS_AS(
            executor.run_single_step_asynchronously(sequence, context, StepIndex{ 1 }), Error);

        // As long as the thread is running, update() must return true, and the sequence must
        // signalize is_running().
        REQUIRE(executor.update(sequence) == true);
        REQUIRE(sequence.is_running() == true);

        bool step2_seen_running = false;

        // Process messages as long as the thread is running
        while (executor.update(sequence))
        {
            REQUIRE(sequence[0].is_running() == false);
            step2_seen_running |= sequence[1].is_running();
            gul14::sleep(5ms);
        }

        REQUIRE(gul14::toc(t0) >= 0.02);
        REQUIRE(step2_seen_running == true);

        // Thread has now finished. As long as we do not start another one, update() keeps
        // returning false.
        REQUIRE(executor.update(sequence) == false);

        auto vars = executor.get_context_variables();
        REQUIRE(std::get<VarInteger>(vars["a"]) == 2);

        // Both the sequence and all of its steps must show is_running() == false.
        REQUIRE(sequence.is_running() == false);
        for (const auto& step : sequence)
            REQUIRE(step.is_running() == false);

        REQUIRE(sequence.get_error().has_value() == false);
    }

    SECTION("Running a failing step")
    {
        // Start the first step in a separate thread
        executor.run_single_step_asynchronously(sequence, context, StepIndex{ 0 });
        REQUIRE(sequence.is_running() == true);

        // Process messages as long as the thread is running
        while (executor.update(sequence))
        {
            REQUIRE(sequence[1].is_running() == false);
            gul14::sleep(5ms);
        }

        REQUIRE(sequence.is_running() == false);

        auto vars = executor.get_context_variables();
        REQUIRE(std::get<VarInteger>(vars["a"]) == 1);

        REQUIRE(sequence.get_error().has_value() == true);
        REQUIRE_THAT(sequence.get_error()->what(), Contains("Waldeinsamkeit"));
        REQUIRE(sequence.get_error()->get_index().has_value() == true);
        REQUIRE(sequence.get_error()->get_index().value() == 0);
    }
}

TEST_CASE("Executor: cancel() endless step loop", "[Executor]")
{
    Context context;
    context.message_callback_function = nullptr; // suppress output on console

    Sequence sequence{ "test_sequence" };
    sequence.push_back(Step{ Step::type_while }.set_script("return true"));
    sequence.push_back(Step{ Step::type_action }.set_script("a = 1"));
    sequence.push_back(Step{ Step::type_end });

    Executor executor;

    executor.run_asynchronously(sequence, context);

    gul14::sleep(1ms);
    executor.cancel(sequence);

    while (executor.update(sequence)) {
        gul14::sleep(5ms);
    }

    // Make sure that exactly the desired error message comes out
    REQUIRE(sequence.get_error().has_value());
    REQUIRE(sequence.get_error()->what() == "Sequence aborted: Stop on user request"s);
}

TEST_CASE("Executor: Destruct while Lua script is running", "[Executor]")
{
    Context context;
    context.message_callback_function = nullptr; // suppress console output

    Sequence sequence{ "test_sequence" };
    sequence.push_back(Step{ Step::type_while }.set_script("return true"));
    sequence.push_back(Step{ Step::type_action }.set_script("a = 1"));
    sequence.push_back(Step{ Step::type_end });

    {
        Executor executor;
        executor.run_asynchronously(sequence, context);
        gul14::sleep(1ms);
    } // executor is destructed here

    REQUIRE(sequence.get_error().has_value() == false);
}

TEST_CASE("Executor: cancel() within Lua sleep()", "[Executor]")
{
    Context context;
    context.message_callback_function = nullptr; // suppress console output

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

    // Thread has now finished. As long as we do not start another one, update() keeps
    // returning false.
    REQUIRE(executor.update(sequence) == false);

    for (const auto& step : sequence)
        REQUIRE(step.is_running() == false);

    // Make sure that exactly the desired error message comes out
    REQUIRE(sequence.get_error().has_value());
    REQUIRE(sequence.get_error()->what() == "Sequence aborted: Stop on user request"s);
}

TEST_CASE("Executor: cancel() within pcalls and CATCH blocks", "[Executor]")
{
    Context context;
    context.message_callback_function = nullptr; // suppress console output

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

    // Thread has now finished. As long as we do not start another one, update() keeps
    // returning false.
    REQUIRE(executor.update(sequence) == false);

    for (const auto& step : sequence)
    {
        CAPTURE(step.get_type());
        REQUIRE(step.is_running() == false);
    }
}

TEST_CASE("Executor: Redirection of print() output", "[Executor]")
{
    std::string output;

    Context context;
    context.message_callback_function =
        [&output](const Message& msg) -> void
        {
            if (msg.get_type() == Message::Type::output)
                output += msg.get_text();
        };

    Step step( Step::type_action );
    step.set_script("print('Mary had', 3, 'little lambs.')");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(std::move(step));

    Executor executor;

    executor.run_asynchronously(sequence, context);

    while (executor.update(sequence))
        gul14::sleep(5ms);

    REQUIRE(output == "Mary had\t3\tlittle lambs.\n");
}

TEST_CASE("Executor: Access context after run", "[Executor]")
{
    Context ctx;
    ctx.variables["a"] = VarInteger{ 1 };

    Sequence seq{ "a Seq" };
    seq.push_back(Step{ Step::type_while  }.set_script("return a % 10 ~= 0"));
    seq.push_back(Step{ Step::type_action }.set_script("a = a + 1"));
    seq.push_back(Step{ Step::type_end });
    seq.push_back(Step{ Step::type_action }.set_script("a = a + 1"));

    for (auto s = seq.begin(); s != seq.end(); ++s)
        seq.modify(s, [](Step& step) { step.set_used_context_variable_names(VariableNames{ "a" }); });

    // Execute directly
    REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);
    REQUIRE(std::get<VarInteger>(ctx.variables["a"]) == 11 );

    // Execute async
    Executor executor{ };
    executor.run_asynchronously(seq, ctx);
    while (executor.update(seq))
        gul14::sleep(5ms);
    auto vars = executor.get_context_variables();
    REQUIRE(std::get<VarInteger>(vars["a"]) == 21);
}

TEST_CASE("Executor: Run a sequence asynchronously with explict termination",
    "[Executor]")
{
    Context ctx;
    ctx.message_callback_function = nullptr;

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

    ctx.variables["a"] = VarInteger{ 0 };

    seq.push_back(step_while);
    seq.push_back(step_increment);
    seq.push_back(step_sleep);
    seq.push_back(step_check_termination);
    seq.push_back(step_while_end);

    Executor executor;

    // Start the sequence in a separate thread
    executor.run_asynchronously(seq, ctx);

    // As long as the thread is running, update() must return true, and the sequence must
    // signalize is_running().
    REQUIRE(executor.update(seq) == true);
    REQUIRE(seq.is_running() == true);

    // Process messages as long as the thread is running
    while (executor.update(seq))
        gul14::sleep(5ms);

    // Thread has now finished. As long as we do not start another one, update() keeps
    // returning false.
    REQUIRE(executor.update(seq) == false);

    // Both the sequence and all of its steps must show is_running() == false.
    REQUIRE(seq.is_running() == false);
    for (const auto& step : seq)
        REQUIRE(step.is_running() == false);

    REQUIRE(seq.get_error().has_value() == false);
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
    ctx.message_callback_function = nullptr; // suppress console output

    ctx.step_setup_function = [](sol::state& s) {
        s.set_function("testfun", &lua_testfun);
    };

    Sequence seq{ "Test" };
    Step step{ Step::type_action };
    step.set_script("a = testfun()");

    seq.push_back(step);

    // I. direct execution
    auto maybe_error = seq.execute(ctx, nullptr);
    REQUIRE(maybe_error.has_value() == true);
    REQUIRE_THAT(maybe_error->what(), Contains("Rainbows"));

    // II. run async
    Executor executor{ };
    executor.run_asynchronously(seq, ctx);

    REQUIRE(seq.is_running() == true);

    while (executor.update(seq))
        gul14::sleep(5ms);

    REQUIRE(executor.update(seq) == false);

    REQUIRE(seq.get_error().has_value() == true);
    REQUIRE_THAT(seq.get_error()->what(), Contains("Rainbows"));
}

TEST_CASE("Executor: Message callbacks", "[execute_sequence]")
{
    std::string str;

    Context context;
    context.message_callback_function = [&str](const Message& msg) -> void
        {
            switch (msg.get_type())
            {
            case Message::Type::output:
                str += ("[OUTPUT(" + msg.get_text() + ")]"); break;
            case Message::Type::sequence_started:
                str += "[SEQ_START]"; break;
            case Message::Type::sequence_stopped:
                str += "[SEQ_STOP]"; break;
            case Message::Type::sequence_stopped_with_error:
                str += "[SEQ_STOP_ERR]"; break;
            case Message::Type::step_started:
                str += "[STEP_START]"; break;
            case Message::Type::step_stopped:
                str += "[STEP_STOP]"; break;
            case Message::Type::step_stopped_with_error:
                str += "[STEP_STOP_ERR]"; break;
            case Message::Type::undefined:
                throw Error("Undefined message type");
            }
        };

    Sequence sequence{ "test_sequence" };
    Executor executor;

    SECTION("Sequence ending successfully")
    {
        Step step1{ Step::type_action };
        step1.set_script("print('Rio Grande')");

        Step step2{ Step::type_action };
        step2.set_script("a = 2");

        sequence.push_back(std::move(step1));
        sequence.push_back(std::move(step2));

        executor.run_asynchronously(sequence, context);

        while (executor.update(sequence))
            gul14::sleep(5ms);

        REQUIRE(str ==
            "[SEQ_START]"
                "[STEP_START][OUTPUT(Rio Grande\n)][STEP_STOP]"
                "[STEP_START][STEP_STOP]"
            "[SEQ_STOP]");
    }

    SECTION("Sequence ending with error")
    {
        Step step1{ Step::type_action };
        step1.set_script("print('Rio Lobo')");

        Step step2{ Step::type_action };
        step2.set_script("boom()");

        sequence.push_back(std::move(step1));
        sequence.push_back(std::move(step2));

        executor.run_asynchronously(sequence, context);

        while (executor.update(sequence))
            gul14::sleep(5ms);

        REQUIRE(str ==
            "[SEQ_START]"
                "[STEP_START][OUTPUT(Rio Lobo\n)][STEP_STOP]"
                "[STEP_START][STEP_STOP_ERR]"
            "[SEQ_STOP_ERR]");
    }
}
