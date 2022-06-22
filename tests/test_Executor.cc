/**
 * \file   test_Executor.cc
 * \author Lars Froehlich
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
#include <gul14/time_util.h>
#include "taskomat/Executor.h"

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

    Sequence sequence;
    sequence.push_back(std::move(step));

    Executor executor;

    for (const auto& step : sequence)
        REQUIRE(step.is_running() == false);

    const auto t0 = gul14::tic();

    // Start the sequence in a separate thread
    executor.run_asynchronously(sequence, context);

    // Starting another sequence must fail because the first one is still running
    REQUIRE_THROWS_AS(executor.run_asynchronously(sequence, context), Error);

    // As long as the thread is running, update() and is_busy() must return true
    REQUIRE(executor.is_busy() == true);
    REQUIRE(executor.update(sequence) == true);

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
    // is_busy() keep returning false
    REQUIRE(executor.update(sequence) == false);
    REQUIRE(executor.is_busy() == false);

    for (const auto& step : sequence)
        REQUIRE(step.is_running() == false);
}

TEST_CASE("Executor: cancel() within LUA sleep()", "[Executor]")
{
    Context context;

    Step step(Step::type_action);
    step.set_script("sleep(2)");

    Sequence sequence;
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
}

TEST_CASE("Executor: cancel() within pcalls and CATCH blocks", "[Executor]")
{
    Context context;

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

    Sequence sequence;
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
        [&output](const std::string& str, CommChannel*)
        {
            output += str;
        };

    Step step( Step::type_action );
    step.set_script("print('pippo')");

    Sequence sequence;
    sequence.push_back(std::move(step));

    Executor executor;

    executor.run_asynchronously(sequence, context);

    while (executor.update(sequence))
        gul14::sleep(5ms);

    REQUIRE(output == "pippo");
}
