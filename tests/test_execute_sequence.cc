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

#include <gul14/gul.h>
#include <gul14/catch.h>
#include "taskomat/execute_sequence.h"

using namespace task;

TEST_CASE("execute_sequence(): empty sequence", "[execute_sequence]")
{
    Context context;
    Sequence sequence("Empty sequence");

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
}

TEST_CASE("execute_sequence(): simple sequence", "[execute_sequence]")
{
    Context context;
    Step step;

    Sequence sequence("Simple sequence");
    sequence.push_back(step);

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
}

TEST_CASE("execute_sequence(): Simple sequence with unchanged context", "[execute_sequence]")
{
    Context context;
    context.variables["a"] = VariableValue{ 0LL };

    Step step;
    step.set_used_context_variable_names(VariableNames{"a"});

    Sequence sequence("Simple sequence");
    sequence.push_back(step);

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
    REQUIRE(context.variables["a"] == VariableValue{ 0LL } );
}

TEST_CASE("execute_sequence(): complex sequence with disallowed 'function'",
"[execute_sequence]")
{
    Context context;

    Step step;
    step.set_label("Action");
    step.set_type(Step::type_action);
    step.set_script("print('Hello world!')"); // provokes Error exception

    Sequence sequence("Complex sequence");
    sequence.push_back(step);

    REQUIRE_THROWS(execute_sequence(sequence, context));
}

TEST_CASE("execute_sequence(): complex sequence with disallowed 'function' and context "
 "change", "[execute_sequence]")
{
    Context context;
    context.variables["a"] = VariableValue{ 1LL };

    Step step_action1;
    step_action1.set_label("Action");
    step_action1.set_type(Step::type_action);
    step_action1.set_script("print('Hello world!')"); // provokes Error exception

    Step step_action2;
    step_action2.set_label("Action");
    step_action2.set_type(Step::type_action);
    step_action2.set_script("a=a+1");

    Sequence sequence("Complex sequence");
    sequence.push_back(step_action1);
    sequence.push_back(step_action2);

    REQUIRE_THROWS(execute_sequence(sequence, context));
    REQUIRE(std::get<long long>(context.variables["a"] ) == 1LL );
}

TEST_CASE("execute_sequence(): complex sequence with context change",
"[execute_sequence]")
{
    Context context;
    context.variables["a"] = VariableValue{ 1LL };

    Step step_action1;
    step_action1.set_label("Action1");
    step_action1.set_type(Step::type_action);
    step_action1.set_script("a=a+1");
    step_action1.set_used_context_variable_names(VariableNames{"a"});

    Sequence sequence("Complex sequence");
    sequence.push_back(step_action1);

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
    REQUIRE(std::get<long long>(context.variables["a"] ) == 2LL );
}

TEST_CASE("execute_sequence(): if-else sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 1/2

    script:
    0: if a == 1 then
    1:     a = 2
    2: else
    4:     a = 3
    5: end

    post-condition:
    a = 2/3

    */
    Step step_if         {Step::type_if};
    Step step_action_if  {Step::type_action};
    Step step_else       {Step::type_else};
    Step step_action_else{Step::type_action};
    Step step_if_end     {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_used_context_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_action_if.set_label("if: set a to 2");
    step_action_if.set_used_context_variable_names(VariableNames{"a"});
    step_action_if.set_script("a = 2");

    step_else.set_label("else");

    step_action_else.set_label("else: set a to 3");
    step_action_else.set_used_context_variable_names(VariableNames{"a"});
    step_action_else.set_script("a = 3");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.push_back(step_if);
    sequence.push_back(step_action_if);
    sequence.push_back(step_else);
    sequence.push_back(step_action_else);
    sequence.push_back(step_if_end);

    SECTION("if-else sequence with if=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 1LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 2LL );
    }

    SECTION("if-else sequence with if=false")
    {
        Context context;
        context.variables["a"] = VariableValue{ 2LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 3LL );
    }
}

TEST_CASE("execute_sequence(): if-elseif sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0/1/2

    script:
    0: if a == 1 then
    1:     a = 2
    2: elseif a == 2 then
    4:     a = 3
    5: end

    post-condition:
    a = 0/2/3

    */
    Step step_if             {Step::type_if};
    Step step_if_action      {Step::type_action};
    Step step_elseif         {Step::type_elseif};
    Step step_elseif_action  {Step::type_action};
    Step step_if_end         {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_used_context_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_if_action.set_label("if: set a to 2");
    step_if_action.set_used_context_variable_names(VariableNames{"a"});
    step_if_action.set_script("a = 2");

    step_elseif.set_label("elseif");
    step_elseif.set_used_context_variable_names(VariableNames{"a"});
    step_elseif.set_script("return a == 2");

    step_elseif_action.set_label("elseif: set a to 3");
    step_elseif_action.set_used_context_variable_names(VariableNames{"a"});
    step_elseif_action.set_script("a = 3");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.push_back(step_if);
    sequence.push_back(step_if_action);
    sequence.push_back(step_elseif);
    sequence.push_back(step_elseif_action);
    sequence.push_back(step_if_end);

    SECTION("if-elseif sequence with if=elseif=false")
    {
        Context context;
        context.variables["a"] = VariableValue{ 0LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 0LL );
    }

    SECTION("if-elseif sequence with if=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 1LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 2LL );
    }

    SECTION("if-elseif sequence with elseif=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 2LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 3LL );
    }
}

TEST_CASE("execute_sequence(): if-elseif-else sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 1/2/3

    script:
    0: if a == 1 then
    1:     a = 2
    2: elseif a == 2 then
    3:     a = 3
    4: else
    5:     a = 4
    6: end

    post-condition:
    a = 2/3/4

    */
    Step step_if             {Step::type_if};
    Step step_if_action      {Step::type_action};
    Step step_elseif         {Step::type_elseif};
    Step step_elseif_action  {Step::type_action};
    Step step_else           {Step::type_else};
    Step step_else_action    {Step::type_action};
    Step step_if_end         {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_used_context_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_if_action.set_label("if: set a to 2");
    step_if_action.set_used_context_variable_names(VariableNames{"a"});
    step_if_action.set_script("a = 2");

    step_elseif.set_label("elseif");
    step_elseif.set_used_context_variable_names(VariableNames{"a"});
    step_elseif.set_script("return a == 2");

    step_elseif_action.set_label("elseif: set a to 3");
    step_elseif_action.set_used_context_variable_names(VariableNames{"a"});
    step_elseif_action.set_script("a = 3");

    step_else.set_label("else");
    step_else.set_used_context_variable_names(VariableNames{"a"});
    step_else.set_script("return a == 3");

    step_else_action.set_label("else: set a to 4");
    step_else_action.set_used_context_variable_names(VariableNames{"a"});
    step_else_action.set_script("a = 4");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.push_back(step_if);
    sequence.push_back(step_if_action);
    sequence.push_back(step_elseif);
    sequence.push_back(step_elseif_action);
    sequence.push_back(step_else);
    sequence.push_back(step_else_action);
    sequence.push_back(step_if_end);

    SECTION("if-elseif-else sequence with if=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 1LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 2LL );
    }

    SECTION("if-elseif-else sequence with elseif=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 2LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 3LL );
    }

    SECTION("if-elseif-else sequence with else=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 3LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 4LL );
    }
}

TEST_CASE("execute_sequence(): if-elseif-elseif sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0/1/2/3

    script:
    0: if a == 1 then
    1:     a = 2
    2: elseif a == 2 then
    3:     a = 3
    4: else if a == 3 then
    5:     a = 4
    6: end

    post-condition:
    a = 0/2/3/4

    */
    Step step_if             {Step::type_if};
    Step step_if_action      {Step::type_action};
    Step step_elseif1        {Step::type_elseif};
    Step step_elseif1_action {Step::type_action};
    Step step_elseif2        {Step::type_elseif};
    Step step_elseif2_action {Step::type_action};
    Step step_if_end         {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_used_context_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_if_action.set_label("if: set a to 2");
    step_if_action.set_used_context_variable_names(VariableNames{"a"});
    step_if_action.set_script("a = 2");

    step_elseif1.set_label("elseif");
    step_elseif1.set_used_context_variable_names(VariableNames{"a"});
    step_elseif1.set_script("return a == 2");

    step_elseif1_action.set_label("elseif: set a to 3");
    step_elseif1_action.set_used_context_variable_names(VariableNames{"a"});
    step_elseif1_action.set_script("a = 3");

    step_elseif2.set_label("elseif");
    step_elseif2.set_used_context_variable_names(VariableNames{"a"});
    step_elseif2.set_script("return a == 3");

    step_elseif2_action.set_label("elseif [action] a=4");
    step_elseif2_action.set_used_context_variable_names(VariableNames{"a"});
    step_elseif2_action.set_script("a = 4");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.push_back(step_if);
    sequence.push_back(step_if_action);
    sequence.push_back(step_elseif1);
    sequence.push_back(step_elseif1_action);
    sequence.push_back(step_elseif2);
    sequence.push_back(step_elseif2_action);
    sequence.push_back(step_if_end);

    SECTION("if-elseif-elseif sequence with if=elseif=elseif=false")
    {
        Context context;
        context.variables["a"] = VariableValue{ 0LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 0LL );
    }

    SECTION("if-elseif-elseif sequence with if=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 1LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 2LL );
    }

    SECTION("if-elseif-elseif sequence with elseif[1]=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 2LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 3LL );
    }

    SECTION("if-elseif-elseif sequence with elseif[2]=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 3LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 4LL );
    }
}

TEST_CASE("execute_sequence(): if-elseif-elseif-else sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0/1/2/3

    script:
    0: if a == 1 then
    1:     a = 2
    2: elseif a == 2 then
    3:     a = 3
    4: elseif a == 3 then
    5:     a = 4
    6: else
    7:     a = 5
    8: end

    post-condition:
    a = 5/2/3/4

    */
    Step step_if             {Step::type_if};
    Step step_if_action      {Step::type_action};
    Step step_elseif1        {Step::type_elseif};
    Step step_elseif1_action {Step::type_action};
    Step step_elseif2        {Step::type_elseif};
    Step step_elseif2_action {Step::type_action};
    Step step_else           {Step::type_else};
    Step step_else_action    {Step::type_action};
    Step step_if_end         {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_used_context_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_if_action.set_label("if: set a to 2");
    step_if_action.set_used_context_variable_names(VariableNames{"a"});
    step_if_action.set_script("a = 2");

    step_elseif1.set_label("elseif1");
    step_elseif1.set_used_context_variable_names(VariableNames{"a"});
    step_elseif1.set_script("return a == 2");

    step_elseif1_action.set_label("elseif: set a to 3");
    step_elseif1_action.set_used_context_variable_names(VariableNames{"a"});
    step_elseif1_action.set_script("a = 3");

    step_elseif2.set_label("elseif2");
    step_elseif2.set_used_context_variable_names(VariableNames{"a"});
    step_elseif2.set_script("return a == 3");

    step_elseif2_action.set_label("else: set a to 4");
    step_elseif2_action.set_used_context_variable_names(VariableNames{"a"});
    step_elseif2_action.set_script("a = 4");

    step_else.set_label("else");
    step_else.set_used_context_variable_names(VariableNames{"a"});

    step_else_action.set_label("else: set a to 5");
    step_else_action.set_used_context_variable_names(VariableNames{"a"});
    step_else_action.set_script("a = 5");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.push_back(step_if);
    sequence.push_back(step_if_action);
    sequence.push_back(step_elseif1);
    sequence.push_back(step_elseif1_action);
    sequence.push_back(step_elseif2);
    sequence.push_back(step_elseif2_action);
    sequence.push_back(step_else);
    sequence.push_back(step_else_action);
    sequence.push_back(step_if_end);

    SECTION("if-elseif-elseif sequence with if=elseif=elseif=false")
    {
        Context context;
        context.variables["a"] = VariableValue{ 0LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 5LL );
    }

    SECTION("if-elseif-elseif sequence with if=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 1LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 2LL );
    }

    SECTION("if-elseif-elseif sequence with elseif[1]=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 2LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 3LL );
    }

    SECTION("if-elseif-elseif sequence with elseif[2]=true")
    {
        Context context;
        context.variables["a"] = VariableValue{ 3LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 4LL );
    }
}

TEST_CASE("execute_sequence(): faulty if-else-elseif sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    0: if a == 1 then
    1:     a = 2
    2: else
    3:     a = 3
    4: elseif a == 3 then <= throws error due to swapping of 'else' and 'elseif'
    5:     a = 4
    6: end

    post-condition:
    throw Error exception

    */
    Step step_if               {Step::type_if};
    Step step_if_action        {Step::type_action};
    Step step_if_else          {Step::type_else};
    Step step_if_else_action   {Step::type_action};
    Step step_if_elseif        {Step::type_elseif};
    Step step_if_elseif_action {Step::type_action};
    Step step_if_end           {Step::type_end};

    step_if.set_label("if a == 1 then");
    step_if.set_used_context_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_if_action.set_label("if: set a to 2");
    step_if_action.set_used_context_variable_names(VariableNames{"a"});
    step_if_action.set_script("a = 2");

    step_if_else.set_label("else");
    step_if_else.set_used_context_variable_names(VariableNames{"a"});

    step_if_else_action.set_label("elseif: set a to 3");
    step_if_else_action.set_used_context_variable_names(VariableNames{"a"});
    step_if_else_action.set_script("a = 3");

    step_if_elseif.set_label("elseif");
    step_if_elseif.set_used_context_variable_names(VariableNames{"a"});
    step_if_elseif.set_script("return a == 3");

    step_if_elseif_action.set_label("else: set a to 4");
    step_if_elseif_action.set_used_context_variable_names(VariableNames{"a"});
    step_if_elseif_action.set_script("a = 4");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.push_back(step_if);
    sequence.push_back(step_if_action);
    sequence.push_back(step_if_else);
    sequence.push_back(step_if_else_action);
    sequence.push_back(step_if_elseif);
    sequence.push_back(step_if_elseif_action);
    sequence.push_back(step_if_end);

    Context context;
    context.variables["a"] = VariableValue{ 0LL };

    REQUIRE_THROWS_AS(execute_sequence(sequence, context), Error);
}

TEST_CASE("execute_sequence(): while sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    1: while a < 10
    2:     a = a + 1
    3: end

    post-condition:
    a = 10

    */
    Step step_while          {Step::type_while};
    Step step_while_action   {Step::type_action};
    Step step_while_end      {Step::type_end};

    step_while.set_label("while a < 10");
    step_while.set_used_context_variable_names(VariableNames{"a"});
    step_while.set_script("return a < 10");

    step_while_action.set_label("while: a = a + 1");
    step_while_action.set_used_context_variable_names(VariableNames{"a"});
    step_while_action.set_script("a = a + 1");

    step_while_end.set_label("while: end");

    Sequence sequence;
    sequence.push_back(step_while);
    sequence.push_back(step_while_action);
    sequence.push_back(step_while_end);

    SECTION("while sequence with while: a<10")
    {
        Context context;
        context.variables["a"] = VariableValue{ 0LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 10LL );
    }
}

TEST_CASE("execute_sequence(): try sequence with success", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    0: try
    1:     a = 1
    3: catch
    4:     a = 2
    5: end

    post-condition:
    a = 1

    */
    Step step_try              {Step::type_try};
    Step step_try_action       {Step::type_action};
    Step step_try_catch        {Step::type_catch};
    Step step_try_catch_action {Step::type_action};
    Step step_try_end          {Step::type_end};

    step_try.set_label("try");

    step_try_action.set_label("try: a = 1");
    step_try_action.set_used_context_variable_names(VariableNames{"a"});
    step_try_action.set_script("a = 1");

    step_try_catch.set_label("try: catch");

    step_try_catch_action.set_label("try-catch: a = 2");
    step_try_catch_action.set_used_context_variable_names(VariableNames{"a"});
    step_try_catch_action.set_script("a = 2");

    step_try_end.set_label("try: end");

    Sequence sequence;
    sequence.push_back(step_try);
    sequence.push_back(step_try_action);
    sequence.push_back(step_try_catch);
    sequence.push_back(step_try_catch_action);
    sequence.push_back(step_try_end);

    Context context;
    context.variables["a"] = VariableValue{ 0LL };

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
    REQUIRE(std::get<long long>(context.variables["a"] ) == 1LL );
}

TEST_CASE("execute_sequence(): try sequence with fault", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    0: try
    1:     a = 1
    2:     this line crashes lua
    3: catch
    4:     a = 2
    5: end

    post-condition:
    a = 2

    */
    Step step_try              {Step::type_try};
    Step step_try_action1      {Step::type_action};
    Step step_try_action2      {Step::type_action};
    Step step_try_catch        {Step::type_catch};
    Step step_try_catch_action {Step::type_action};
    Step step_try_end          {Step::type_end};

    step_try.set_label("try");

    step_try_action1.set_label("try[action1]: a = 1");
    step_try_action1.set_used_context_variable_names(VariableNames{"a"});
    step_try_action1.set_script("a = 1");

    step_try_action2.set_label("try[action2]: this line crash lua");
    step_try_action2.set_used_context_variable_names(VariableNames{"a"});
    step_try_action2.set_script("this line crashes lua");

    step_try_catch.set_label("try: catch");

    step_try_catch_action.set_label("try-catch: a = 2");
    step_try_catch_action.set_used_context_variable_names(VariableNames{"a"});
    step_try_catch_action.set_script("a = 2");

    step_try_end.set_label("try: end");

    Sequence sequence;
    sequence.push_back(step_try);
    sequence.push_back(step_try_action1);
    sequence.push_back(step_try_action2);
    sequence.push_back(step_try_catch);
    sequence.push_back(step_try_catch_action);
    sequence.push_back(step_try_end);

    Context context;
    context.variables["a"] = VariableValue{ 0LL };

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
    REQUIRE(std::get<long long>(context.variables["a"] ) == 2LL );
}

TEST_CASE("execute_sequence(): complex try sequence with nested fault condition",
"[execute_sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    00: try
    01:     a = 1
    02:     this line crashes lua
    03: catch

    04:     try
    05:         a = 2
    06:         and again crashes lua
    07:     catch
    08:         a = 3
    09:     end

    10: end

    post-condition:
    a = 3

    */
    Step step_00  {Step::type_try};
    Step step_01  {Step::type_action};
    Step step_02  {Step::type_action};
    Step step_03  {Step::type_catch};

    Step step_04  {Step::type_try};
    Step step_05  {Step::type_action};
    Step step_06  {Step::type_action};
    Step step_07  {Step::type_catch};
    Step step_08  {Step::type_action};
    Step step_09  {Step::type_end};

    Step step_10  {Step::type_end};

    step_00.set_label("try");

    step_01.set_script("a = 1");
    step_01.set_label("try [action]: a = 1");
    step_01.set_used_context_variable_names(VariableNames{"a"});

    step_02.set_script("this line crashes lua");
    step_02.set_label("try [action]: this line crash lua");
    step_02.set_used_context_variable_names(VariableNames{"a"});

    step_03.set_label("try [catch]");

    step_04.set_label("try");

    step_05.set_script("a = 2");
    step_05.set_label("try [action] a = 2");
    step_05.set_used_context_variable_names(VariableNames{"a"});

    step_06.set_script("and again crashes lua");
    step_06.set_label("try [action] and again crashes lua");
    step_06.set_used_context_variable_names(VariableNames{"a"});

    step_07.set_label("try [catch]");

    step_08.set_script("a = 3");
    step_08.set_label("catch[action] a = 3");
    step_08.set_used_context_variable_names(VariableNames{"a"});

    step_09.set_label("try [end]");

    step_10.set_label("try [end]");

    Sequence sequence;
    sequence.push_back(step_00);
    sequence.push_back(step_01);
    sequence.push_back(step_02);
    sequence.push_back(step_03);
    sequence.push_back(step_04);
    sequence.push_back(step_05);
    sequence.push_back(step_06);
    sequence.push_back(step_07);
    sequence.push_back(step_08);
    sequence.push_back(step_09);
    sequence.push_back(step_10);

    Context context;
    context.variables["a"] = VariableValue{ 0LL };

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
    REQUIRE(std::get<long long>(context.variables["a"] ) == 3LL );
}

TEST_CASE("execute_sequence(): simple try sequence with fault", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    00: try
    01:     a = 1
    02:     this line crashes lua
    03: catch
    04:     a = 2
    05:     and again crashes lua
    06: end

    post-condition:
    exception, a = 2

    */
    Step step_00  {Step::type_try};
    Step step_01  {Step::type_action};
    Step step_02  {Step::type_action};
    Step step_03  {Step::type_catch};

    Step step_04  {Step::type_action};
    Step step_05  {Step::type_action};
    Step step_06  {Step::type_end};

    step_00.set_label("try");

    step_01.set_script("a = 1");
    step_01.set_label("try [action]: a = 1");
    step_01.set_used_context_variable_names(VariableNames{"a"});

    step_02.set_script("this line crashes lua");
    step_02.set_label("try [action]: this line crash lua");
    step_02.set_used_context_variable_names(VariableNames{"a"});

    step_03.set_label("try [catch]");

    step_04.set_script("a = 2");
    step_04.set_label("try [action] a = 2");
    step_04.set_used_context_variable_names(VariableNames{"a"});

    step_05.set_script("and again crashes lua");
    step_05.set_label("try [action] and again crashes lua");
    step_05.set_used_context_variable_names(VariableNames{"a"});

    step_06.set_label("try [end]");

    Sequence sequence;
    sequence.push_back(step_00);
    sequence.push_back(step_01);
    sequence.push_back(step_02);
    sequence.push_back(step_03);
    sequence.push_back(step_04);
    sequence.push_back(step_05);
    sequence.push_back(step_06);

    Context context;
    context.variables["a"] = VariableValue{ 0LL };

    REQUIRE_THROWS_AS(execute_sequence(sequence, context), Error);
    REQUIRE(std::get<long long>(context.variables["a"] ) == 2LL );
}

TEST_CASE("execute_sequence(): complex try sequence with fault", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    00: try
    01:     a = 1
    02:     this line crashes lua
    03: catch

    04:     try
    05:         a = 2
    06:         and again crashes lua
    07:     catch
    08:         and again crashes lua
    09:         a = 3
    10:     end

    11: end

    post-condition:
    exception, a = 2

    */
    Step step_00  {Step::type_try};
    Step step_01  {Step::type_action};
    Step step_02  {Step::type_action};
    Step step_03  {Step::type_catch};

    Step step_04  {Step::type_try};
    Step step_05  {Step::type_action};
    Step step_06  {Step::type_action};
    Step step_07  {Step::type_catch};
    Step step_08  {Step::type_action};
    Step step_09  {Step::type_action};
    Step step_10  {Step::type_end};

    Step step_11  {Step::type_end};

    step_00.set_label("try");

    step_01.set_script("a = 1");
    step_01.set_label("try [action]: a = 1");
    step_01.set_used_context_variable_names(VariableNames{"a"});

    step_02.set_script("this line crashes lua");
    step_02.set_label("try [action]: this line crash lua");
    step_02.set_used_context_variable_names(VariableNames{"a"});

    step_03.set_label("try [catch]");

    step_04.set_label("try");

    step_05.set_script("a = 2");
    step_05.set_label("try [action] a = 2");
    step_05.set_used_context_variable_names(VariableNames{"a"});

    step_06.set_script("this line crashes lua");
    step_06.set_label("try [action]: this line crash lua");
    step_06.set_used_context_variable_names(VariableNames{"a"});

    step_07.set_label("try [catch]");

    step_08.set_script("this line crashes lua");
    step_08.set_label("try [action]: this line crash lua");
    step_08.set_used_context_variable_names(VariableNames{"a"});

    step_09.set_script("a = 3");
    step_09.set_label("try [action]: a = 3");
    step_09.set_used_context_variable_names(VariableNames{"a"});

    step_10.set_label("try [end]");

    step_11.set_label("try [end]");

    Sequence sequence;
    sequence.push_back(step_00);
    sequence.push_back(step_01);
    sequence.push_back(step_02);
    sequence.push_back(step_03);
    sequence.push_back(step_04);
    sequence.push_back(step_05);
    sequence.push_back(step_06);
    sequence.push_back(step_07);
    sequence.push_back(step_08);
    sequence.push_back(step_09);
    sequence.push_back(step_10);
    sequence.push_back(step_11);

    Context context;
    context.variables["a"] = VariableValue{ 0LL };

    REQUIRE_THROWS_AS(execute_sequence(sequence, context), Error);
    REQUIRE(std::get<long long>(context.variables["a"] ) == 2LL );
}

TEST_CASE("execute_sequence(): complex sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a=0/0/0/0, b=0/0/0/0, c=0/0/0/0, d=0/1/2/1, e=1/1/5/2, f=1/1/3/3, g=1/1/2/4

    script:
     0: while a < 10

     1:     try
     2:         b = 1
     3:     catch
     4:         b = 2
     5:     end

     6:     c = 1

     7:     if d == 1 then

     8:         if e == 1 then
     9:             e = 2
    10:         elseif e == 2 then
    11:             e = 3
    12:         else

    13:             try
    14:                 f = 1
    15:                 this line crashes lua
    16:             catch
    17:                 f = 2
    18:             end

    19:             if g == 2 then
    20:                 g = 1
    21:             else
    22:                 g = 2
    23:             end

    24:             e = 4

    25:         end

    26:         d = 2

    27:     elseif d == 2 then
    28:         d = 3
    29:     end

    30:     a = a + 1

    31: end

    post-condition:
    a=10/10/10/10, b=1/1/1/1, c=1/1/1/1, d=0/3/3/3, e=1/2/5/4, f=1/1//3/2, g=1/1/2/2

    */

    // 32 steps
    Step step_00{Step::type_while};

    Step step_01{Step::type_try};
    Step step_02{Step::type_action};
    Step step_03{Step::type_catch};
    Step step_04{Step::type_action};
    Step step_05{Step::type_end};

    Step step_06{Step::type_action};

    Step step_07{Step::type_if};

    Step step_08{Step::type_if};
    Step step_09{Step::type_action};
    Step step_10{Step::type_elseif};
    Step step_11{Step::type_action};
    Step step_12{Step::type_else};

    Step step_13{Step::type_try};
    Step step_14{Step::type_action};
    Step step_15{Step::type_action};
    Step step_16{Step::type_catch};
    Step step_17{Step::type_action};
    Step step_18{Step::type_end};

    Step step_19{Step::type_if};
    Step step_20{Step::type_action};
    Step step_21{Step::type_else};
    Step step_22{Step::type_action};
    Step step_23{Step::type_end};

    Step step_24{Step::type_action};

    Step step_25{Step::type_end};

    Step step_26{Step::type_action};

    Step step_27{Step::type_elseif};
    Step step_28{Step::type_action};
    Step step_29{Step::type_end};

    Step step_30{Step::type_action};

    Step step_31{Step::type_end};

    step_00.set_label("while a < 10");
    step_00.set_script("return a < 10");
    step_00.set_used_context_variable_names(VariableNames{"a"});

    step_01.set_label("try");

    step_02.set_label("try [action] b = 1");
    step_02.set_script("b = 1");
    step_02.set_used_context_variable_names(VariableNames{"b"});

    step_03.set_label("try [catch]");

    step_04.set_label("try [action] b = 2");
    step_04.set_script("b = 2");
    step_04.set_used_context_variable_names(VariableNames{"b"});

    step_05.set_label("try [end]");

    step_06.set_label("[action] c = 1");
    step_06.set_script("c = 1");
    step_06.set_used_context_variable_names(VariableNames{"c"});

    step_07.set_label("if d == 1");
    step_07.set_script("return d == 1");
    step_07.set_used_context_variable_names(VariableNames{"d"});

    step_08.set_label("if e == 1");
    step_08.set_script("return e == 1");
    step_08.set_used_context_variable_names(VariableNames{"e"});

    step_09.set_label("if [action] e = 2");
    step_09.set_script("e = 2");
    step_09.set_used_context_variable_names(VariableNames{"e"});

    step_10.set_label("if [elseif] e == 2");
    step_10.set_script("return e == 2");
    step_10.set_used_context_variable_names(VariableNames{"e"});

    step_11.set_label("if [action] e = 3");
    step_11.set_script("e = 3");
    step_11.set_used_context_variable_names(VariableNames{"e"});

    step_12.set_label("if [else]");

    step_13.set_label("try");

    step_14.set_label("try [action] f = 1");
    step_14.set_script("f = 1");
    step_14.set_used_context_variable_names(VariableNames{"f"});

    step_15.set_label("try [action] this line crashes lua");
    step_15.set_script("this line crashes lua");

    step_16.set_label("try [catch]");

    step_17.set_label("try [action] f=2");
    step_17.set_script("f=2");
    step_17.set_used_context_variable_names(VariableNames{"f"});

    step_18.set_label("try [end]");

    step_19.set_label("if g == 2");
    step_19.set_script("return g == 2");
    step_19.set_used_context_variable_names(VariableNames{"g"});

    step_20.set_label("if [action] g = 1");
    step_20.set_script("g = 1");
    step_20.set_used_context_variable_names(VariableNames{"g"});

    step_21.set_label("if [else]");

    step_22.set_label("if [action] g = 2");
    step_22.set_script("g = 2");
    step_22.set_used_context_variable_names(VariableNames{"g"});

    step_23.set_label("if [end]");

    step_24.set_label("[action] e = 4");
    step_24.set_script("e = 4");
    step_24.set_used_context_variable_names(VariableNames{"e"});

    step_25.set_label("if [end]");

    step_26.set_label("[action] d = 2");
    step_26.set_script("d = 2");
    step_26.set_used_context_variable_names(VariableNames{"d"});

    step_27.set_label("if [elseif] d == 2");
    step_27.set_script("return d == 2");
    step_27.set_used_context_variable_names(VariableNames{"d"});

    step_28.set_label("if [action] d = 3");
    step_28.set_script("d = 3");
    step_28.set_used_context_variable_names(VariableNames{"d"});

    step_29.set_label("if [end]");

    step_30.set_label("a=a+1");
    step_30.set_script("a=a+1");
    step_30.set_used_context_variable_names(VariableNames{"a"});

    step_31.set_label("while [end]");

    Sequence sequence;
    // 32 steps
    sequence.push_back(step_00);

    sequence.push_back(step_01);
    sequence.push_back(step_02);
    sequence.push_back(step_03);
    sequence.push_back(step_04);
    sequence.push_back(step_05);

    sequence.push_back(step_06);

    sequence.push_back(step_07);

    sequence.push_back(step_08);
    sequence.push_back(step_09);
    sequence.push_back(step_10);
    sequence.push_back(step_11);
    sequence.push_back(step_12);

    sequence.push_back(step_13);
    sequence.push_back(step_14);
    sequence.push_back(step_15);
    sequence.push_back(step_16);
    sequence.push_back(step_17);
    sequence.push_back(step_18);

    sequence.push_back(step_19);
    sequence.push_back(step_20);
    sequence.push_back(step_21);
    sequence.push_back(step_22);
    sequence.push_back(step_23);

    sequence.push_back(step_24);

    sequence.push_back(step_25);

    sequence.push_back(step_26);

    sequence.push_back(step_27);
    sequence.push_back(step_28);
    sequence.push_back(step_29);

    sequence.push_back(step_30);

    sequence.push_back(step_31);

    SECTION("complex sequence: a: 0->10, b: 0->1, c: 0->1, d: 0->0, e: 1->1, f: 1->1, "
            "g: 1->1")
    {
        Context context;
        // a=0, b=0, c=0, d=0/1/2, e=1/2/3, f=0, g=2/1
        context.variables["a"] = VariableValue{0LL};
        context.variables["b"] = VariableValue{0LL};
        context.variables["c"] = VariableValue{0LL};
        context.variables["d"] = VariableValue{0LL};
        context.variables["e"] = VariableValue{1LL};
        context.variables["f"] = VariableValue{1LL};
        context.variables["g"] = VariableValue{1LL};
        REQUIRE_NOTHROW(execute_sequence(sequence, context));

        // a=10, b=1, c=1, d=0/2/3, e=1/3/4, f=2, g=1/2
        REQUIRE(std::get<long long>(context.variables["a"] ) == 10LL );
        REQUIRE(std::get<long long>(context.variables["b"] ) == 1LL );
        REQUIRE(std::get<long long>(context.variables["c"] ) == 1LL );
        REQUIRE(std::get<long long>(context.variables["d"] ) == 0LL );
        REQUIRE(std::get<long long>(context.variables["e"] ) == 1LL ); // not touched
        REQUIRE(std::get<long long>(context.variables["f"] ) == 1LL ); // not touched
        REQUIRE(std::get<long long>(context.variables["g"] ) == 1LL ); // not touched
    }

    SECTION("complex sequence: a: 0->10, b: 0->1, c: 0->1, d: 1->3, e: 1->2, f: 1->1, "
            "g: 1->1")
    {
        Context context;
        context.variables["a"] = VariableValue{0LL};
        context.variables["b"] = VariableValue{0LL};
        context.variables["c"] = VariableValue{0LL};
        context.variables["d"] = VariableValue{1LL};
        context.variables["e"] = VariableValue{1LL};
        context.variables["f"] = VariableValue{1LL};
        context.variables["g"] = VariableValue{1LL};

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 10LL );
        REQUIRE(std::get<long long>(context.variables["b"] ) == 1LL );
        REQUIRE(std::get<long long>(context.variables["c"] ) == 1LL );
        REQUIRE(std::get<long long>(context.variables["d"] ) == 3LL );
        REQUIRE(std::get<long long>(context.variables["e"] ) == 2LL );
        REQUIRE(std::get<long long>(context.variables["f"] ) == 1LL ); // not touched
        REQUIRE(std::get<long long>(context.variables["g"] ) == 1LL ); // not touched
    }

    SECTION("complex sequence: a: 0->10, b: 0->1, c: 0->1, d: 2->3, e: 5->5, f: 2->2, "
            "g: 3->3")
    {
        Context context;
        context.variables["a"] = VariableValue{0LL};
        context.variables["b"] = VariableValue{0LL};
        context.variables["c"] = VariableValue{1LL};
        context.variables["d"] = VariableValue{2LL};
        context.variables["e"] = VariableValue{5LL};
        context.variables["f"] = VariableValue{3LL};
        context.variables["g"] = VariableValue{2LL};

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 10LL );
        REQUIRE(std::get<long long>(context.variables["b"] ) == 1LL );
        REQUIRE(std::get<long long>(context.variables["c"] ) == 1LL );
        REQUIRE(std::get<long long>(context.variables["d"] ) == 3LL );
        REQUIRE(std::get<long long>(context.variables["e"] ) == 5LL ); // not touched
        REQUIRE(std::get<long long>(context.variables["f"] ) == 3LL ); // not touched
        REQUIRE(std::get<long long>(context.variables["g"] ) == 2LL ); // not touched
    }

    SECTION("complex sequence: a: 0->10, b: 0->1, c: 0->1, d: 1->3, e: 2->4, f: 3->2, "
            "g: 4->2")
    {
        Context context;
        context.variables["a"] = VariableValue{0LL};
        context.variables["b"] = VariableValue{0LL};
        context.variables["c"] = VariableValue{1LL};
        context.variables["d"] = VariableValue{1LL};
        context.variables["e"] = VariableValue{2LL};
        context.variables["f"] = VariableValue{3LL};
        context.variables["g"] = VariableValue{4LL};

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context.variables["a"] ) == 10LL );
        REQUIRE(std::get<long long>(context.variables["b"] ) == 1LL );
        REQUIRE(std::get<long long>(context.variables["c"] ) == 1LL );
        REQUIRE(std::get<long long>(context.variables["d"] ) == 3LL );
        REQUIRE(std::get<long long>(context.variables["e"] ) == 3LL );
        REQUIRE(std::get<long long>(context.variables["f"] ) == 3LL ); // not touched
        REQUIRE(std::get<long long>(context.variables["g"] ) == 4LL ); // not touched
    }
}

TEST_CASE("execute_sequence(): complex sequence with misplaced if", "[execute_sequence]")
{
    /*
    pre-condition:
    not needed because syntax checker throws exception on index 12

    script:
     0: while a < 10

     1:     try
     2:         b = 1
     3:     catch
     4:         b = 2
     5:     end

     6:     c = 1

     7:     if d == 1 then

     8:         if e == 1 then
     9:             e = 2
    10:         else
    11:             e = 3
    12:         elseif e == 2 then <== FAULT

    13:             try
    14:                 f = 1
    15:                 this line crashes lua
    16:             catch
    17:                 f = 2
    18:             end

    19:             if g == 2 then
    20:                 g == 1
    21:             else
    22:                 g == 2
    23:             end

    24:             e = 4

    25:         end

    26:         d = 2

    27:     elseif d == 2 then
    28:         d = 3
    29:     end

    30:     a = a + 1

    31: end

    post-condition:
    not needed because syntax checker throws exception on index 12

    */

    // 32 steps
    Step step_00{Step::type_while};

    Step step_01{Step::type_try};
    Step step_02{Step::type_action};
    Step step_03{Step::type_catch};
    Step step_04{Step::type_action};
    Step step_05{Step::type_end};

    Step step_06{Step::type_action};

    Step step_07{Step::type_if};

    Step step_08{Step::type_if};
    Step step_09{Step::type_action};
    Step step_10{Step::type_else};
    Step step_11{Step::type_action};
    Step step_12{Step::type_elseif};

    Step step_13{Step::type_try};
    Step step_14{Step::type_action};
    Step step_15{Step::type_action};
    Step step_16{Step::type_catch};
    Step step_17{Step::type_action};
    Step step_18{Step::type_end};

    Step step_19{Step::type_if};
    Step step_20{Step::type_action};
    Step step_21{Step::type_else};
    Step step_22{Step::type_action};
    Step step_23{Step::type_end};

    Step step_24{Step::type_action};

    Step step_25{Step::type_end};

    Step step_26{Step::type_action};

    Step step_27{Step::type_elseif};
    Step step_28{Step::type_action};
    Step step_29{Step::type_end};

    Step step_30{Step::type_action};

    Step step_31{Step::type_end};

    Sequence sequence;
    // 32 steps
    sequence.push_back(step_00);

    sequence.push_back(step_01);
    sequence.push_back(step_02);
    sequence.push_back(step_03);
    sequence.push_back(step_04);
    sequence.push_back(step_05);

    sequence.push_back(step_06);

    sequence.push_back(step_07);

    sequence.push_back(step_08);
    sequence.push_back(step_09);
    sequence.push_back(step_10);
    sequence.push_back(step_11);
    sequence.push_back(step_12);

    sequence.push_back(step_13);
    sequence.push_back(step_14);
    sequence.push_back(step_15);
    sequence.push_back(step_16);
    sequence.push_back(step_17);
    sequence.push_back(step_18);

    sequence.push_back(step_19);
    sequence.push_back(step_20);
    sequence.push_back(step_21);
    sequence.push_back(step_22);
    sequence.push_back(step_23);

    sequence.push_back(step_24);

    sequence.push_back(step_25);

    sequence.push_back(step_26);

    sequence.push_back(step_27);
    sequence.push_back(step_28);
    sequence.push_back(step_29);

    sequence.push_back(step_30);

    sequence.push_back(step_31);

    Context context;
    REQUIRE_THROWS_AS(execute_sequence(sequence, context), Error);
}
