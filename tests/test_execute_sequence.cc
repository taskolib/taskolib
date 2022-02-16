/**
 * \file   test_execute_sequence.cc
 * \author Marcus Walla
 * \date   Created on February 16, 2022
 * \brief  Test suite for the free function execute_sequence().
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
#include "../include/taskomat/Error.h"
#include "../include/taskomat/execute_sequence.h"
#include "../include/taskomat/execute_step.h"

using namespace std::literals;
using namespace task;

TEST_CASE("execute_sequence(): simple sequence", "[execute_sequence]")
{
    Context context;
    Step step;

    Sequence sequence("Simple sequence");
    sequence.add_step(step);

    SECTION("Simple sequence")
    {
        REQUIRE_NOTHROW(execute_sequence(sequence, context));
    }
    
    SECTION("Simple sequence with unchanged context")
    {
        context["a"] = VariableValue{ 0LL };
        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(context["a"] == VariableValue{ 0LL } );
    }
}

TEST_CASE("execute_sequence(): complex sequence", "[execute_sequence]")
{
    Context context;
    Step stepAction1;
    Step stepAction2;

    Sequence sequence("Complex sequence");
    sequence.add_step(stepAction1);
    sequence.add_step(stepAction2);

    SECTION("Complex sequence without context change")
    {
        context["a"] = VariableValue{ 0LL };
        stepAction1.set_script("print('Hello World!");
        stepAction1.set_script("print('a='..a)");
        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(context["a"] == VariableValue{ 0LL } );
    }

}

TEST_CASE("execute_sequence(): complex sequence with context change",
"[execute_sequence]")
{
    Context context;
    Step stepAction1;
    Step stepAction2;

    Sequence sequence("Complex sequence");
    sequence.add_step(stepAction1);
    sequence.add_step(stepAction2);

    context["a"] = VariableValue{ 1LL };
    
    stepAction1.set_label("Action1");
    stepAction1.set_type(Step::type_action);
    stepAction1.set_script("a=a+1");
    
    stepAction2.set_label("Action2");
    stepAction2.set_type(Step::type_action);
    stepAction2.set_script("print('a='..a)");
 
    REQUIRE_NOTHROW(execute_sequence(sequence, context));
    REQUIRE(std::get<long long>(context["a"] ) == 2LL );
}