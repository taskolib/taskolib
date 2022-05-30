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

TEST_CASE("Executor: Run a sequence asynchonously", "[Executor]")
{
    Context context;
    context.lua_init_function =
        [](sol::state& sol)
        {
            sol["sleep_20ms"] = []{ gul14::sleep(20ms); };
        };

    Step step(Step::type_action);
    step.set_script("sleep_20ms()");

    Sequence sequence;
    sequence.push_back(std::move(step));

    Executor executor;

    // Start the sequence in a separate thread
    executor.run_asynchronously(sequence, context);

    // Starting another sequence must fail because the first one is still running
    REQUIRE_THROWS_AS(executor.run_asynchronously(sequence, context), Error);

    // As long as the thread is running, update() and is_busy() must return true
    REQUIRE(executor.is_busy() == true);
    REQUIRE(executor.update(sequence) == true);

    // Process messages as long as the thread is running
    while (executor.update(sequence))
        gul14::sleep(5ms);

    // Thread has now finished. As long as we do not start another one, update() and
    // is_busy() keep returning false
    REQUIRE(executor.update(sequence) == false);
    REQUIRE(executor.is_busy() == false);
}
