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

using namespace task;

TEST_CASE("execute_sequence(): simple sequence", "[execute_sequence]")
{
    Context context;
    Step step;

    Sequence sequence("Simple sequence");
    sequence.add_step(step);

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
}
    
TEST_CASE("execute_sequence(): Simple sequence with unchanged context", "[execute_sequence]")
{
    Context context;
    context["a"] = VariableValue{ 0LL };

    Step step;
    step.set_imported_variable_names(VariableNames{"a"});
    step.set_exported_variable_names(VariableNames{"a"});

    Sequence sequence("Simple sequence");
    sequence.add_step(step);

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
    REQUIRE(context["a"] == VariableValue{ 0LL } );
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
    sequence.add_step(step);
 
    REQUIRE_THROWS(execute_sequence(sequence, context));
}

TEST_CASE("execute_sequence(): complex sequence with disallowed 'function' and context "
 "change", "[execute_sequence]")
{
    Context context;
    context["a"] = VariableValue{ 1LL };

    Step step_action1;
    step_action1.set_label("Action");
    step_action1.set_type(Step::type_action);
    step_action1.set_script("print('Hello world!')"); // provokes Error exception

    Step step_action2;
    step_action2.set_label("Action");
    step_action2.set_type(Step::type_action);
    step_action2.set_script("a=a+1");

    Sequence sequence("Complex sequence");
    sequence.add_step(step_action1);
    sequence.add_step(step_action2);
 
    REQUIRE_THROWS(execute_sequence(sequence, context));
    REQUIRE(std::get<long long>(context["a"] ) == 1LL );
}

TEST_CASE("execute_sequence(): complex sequence with context change",
"[execute_sequence]")
{
    Context context;
    context["a"] = VariableValue{ 1LL };

    Step step_action1;
    step_action1.set_label("Action1");
    step_action1.set_type(Step::type_action);
    step_action1.set_script("a=a+1");
    step_action1.set_imported_variable_names(VariableNames{"a"});
    step_action1.set_exported_variable_names(VariableNames{"a"});

    Sequence sequence("Complex sequence");
    sequence.add_step(step_action1);
 
    REQUIRE_NOTHROW(execute_sequence(sequence, context));
    REQUIRE(std::get<long long>(context["a"] ) == 2LL );
}

TEST_CASE("execute_sequence(): if-else sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 1/2

    script:
    if a == 1
        a = 2
    else
        a = 3
    end

    post-condition:
    a = 2/3

    */
    Step step_if         {Step::type_if};
    Step step_action_if  {Step::type_action};
    Step step_else       {Step::type_else};
    Step step_action_else{Step::type_action};
    Step step_if_end        {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_imported_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_action_if.set_label("if: set a to 2");
    step_action_if.set_imported_variable_names(VariableNames{"a"});
    step_action_if.set_exported_variable_names(VariableNames{"a"});
    step_action_if.set_script("a = 2");

    step_else.set_label("else");

    step_action_else.set_label("else: set a to 3");
    step_action_else.set_imported_variable_names(VariableNames{"a"});
    step_action_else.set_exported_variable_names(VariableNames{"a"});
    step_action_else.set_script("a = 3");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.add_step(step_if);
    sequence.add_step(step_action_if);
    sequence.add_step(step_else);
    sequence.add_step(step_action_else);
    sequence.add_step(step_if_end);

    SECTION("if-else sequence with if=true")
    {
        Context context;
        context["a"] = VariableValue{ 1LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 2LL );
    }

    SECTION("if-else sequence with if=false")
    {
        Context context;
        context["a"] = VariableValue{ 2LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 3LL );
    }
}

TEST_CASE("execute_sequence(): if-elseif sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0/1/2

    script:
    if a == 1
        a = 2
    else if a == 2
        a = 3
    end

    post-condition:
    a = 0/2/3

    */
    Step step_if             {Step::type_if};
    Step step_if_action      {Step::type_action};
    Step step_elseif         {Step::type_elseif};
    Step step_elseif_action  {Step::type_action};
    Step step_if_end            {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_imported_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_if_action.set_label("if: set a to 2");
    step_if_action.set_imported_variable_names(VariableNames{"a"});
    step_if_action.set_exported_variable_names(VariableNames{"a"});
    step_if_action.set_script("a = 2");

    step_elseif.set_label("elseif");
    step_elseif.set_imported_variable_names(VariableNames{"a"});
    step_elseif.set_script("return a == 2");

    step_elseif_action.set_label("elseif: set a to 3");
    step_elseif_action.set_imported_variable_names(VariableNames{"a"});
    step_elseif_action.set_exported_variable_names(VariableNames{"a"});
    step_elseif_action.set_script("a = 3");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.add_step(step_if);
    sequence.add_step(step_if_action);
    sequence.add_step(step_elseif);
    sequence.add_step(step_elseif_action);
    sequence.add_step(step_if_end);

    SECTION("if-elseif sequence with if=elseif=false")
    {
        Context context;
        context["a"] = VariableValue{ 0LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 0LL );
    }

    SECTION("if-elseif sequence with if=true")
    {
        Context context;
        context["a"] = VariableValue{ 1LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 2LL );
    }

    SECTION("if-elseif sequence with elseif=true")
    {
        Context context;
        context["a"] = VariableValue{ 2LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 3LL );
    }    
}

TEST_CASE("execute_sequence(): if-elseif-else sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 1/2/3

    script:
    if a == 1
        a = 2
    else if a == 2
        a = 3
    else
        a = 4
    end

    post-condition:
    a = 2/3/4

    */
    Step step_if             {Step::type_if};
    Step step_if_action      {Step::type_action};
    Step step_elseif         {Step::type_elseif};
    Step step_elseif_action  {Step::type_action};
    Step step_else           {Step::type_else};
    Step step_else_action    {Step::type_action};
    Step step_if_end            {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_imported_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_if_action.set_label("if: set a to 2");
    step_if_action.set_imported_variable_names(VariableNames{"a"});
    step_if_action.set_exported_variable_names(VariableNames{"a"});
    step_if_action.set_script("a = 2");

    step_elseif.set_label("elseif");
    step_elseif.set_imported_variable_names(VariableNames{"a"});
    step_elseif.set_script("return a == 2");

    step_elseif_action.set_label("elseif: set a to 3");
    step_elseif_action.set_imported_variable_names(VariableNames{"a"});
    step_elseif_action.set_exported_variable_names(VariableNames{"a"});
    step_elseif_action.set_script("a = 3");

    step_else.set_label("else");
    step_else.set_imported_variable_names(VariableNames{"a"});
    step_else.set_script("return a == 3");

    step_else_action.set_label("else: set a to 4");
    step_else_action.set_imported_variable_names(VariableNames{"a"});
    step_else_action.set_exported_variable_names(VariableNames{"a"});
    step_else_action.set_script("a = 4");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.add_step(step_if);
    sequence.add_step(step_if_action);
    sequence.add_step(step_elseif);
    sequence.add_step(step_elseif_action);
    sequence.add_step(step_else);
    sequence.add_step(step_else_action);
    sequence.add_step(step_if_end);

    SECTION("if-elseif-else sequence with if=true")
    {
        Context context;
        context["a"] = VariableValue{ 1LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 2LL );
    }

    SECTION("if-elseif-else sequence with elseif=true")
    {
        Context context;
        context["a"] = VariableValue{ 2LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 3LL );
    }
    
    SECTION("if-elseif-else sequence with else=true")
    {
        Context context;
        context["a"] = VariableValue{ 3LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 4LL );
    }    
}

TEST_CASE("execute_sequence(): if-elseif-elseif sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0/1/2/3

    script:
    if a == 1
        a = 2
    else if a == 2
        a = 3
    else if a == 3
        a = 4
    end

    post-condition:
    a = 0/2/3/4

    */
    Step step_if             {Step::type_if};
    Step step_if_action      {Step::type_action};
    Step step_elseif1        {Step::type_elseif};
    Step step_elseif1_action {Step::type_action};
    Step step_elseif2        {Step::type_elseif};
    Step step_elseif2_action {Step::type_action};
    Step step_if_end            {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_imported_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_if_action.set_label("if: set a to 2");
    step_if_action.set_imported_variable_names(VariableNames{"a"});
    step_if_action.set_exported_variable_names(VariableNames{"a"});
    step_if_action.set_script("a = 2");

    step_elseif1.set_label("elseif1");
    step_elseif1.set_imported_variable_names(VariableNames{"a"});
    step_elseif1.set_script("return a == 2");

    step_elseif1_action.set_label("elseif: set a to 3");
    step_elseif1_action.set_imported_variable_names(VariableNames{"a"});
    step_elseif1_action.set_exported_variable_names(VariableNames{"a"});
    step_elseif1_action.set_script("a = 3");

    step_elseif2.set_label("elseif2");
    step_elseif2.set_imported_variable_names(VariableNames{"a"});
    step_elseif2.set_script("return a == 3");

    step_elseif2_action.set_label("else: set a to 4");
    step_elseif2_action.set_imported_variable_names(VariableNames{"a"});
    step_elseif2_action.set_exported_variable_names(VariableNames{"a"});
    step_elseif2_action.set_script("a = 4");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.add_step(step_if);
    sequence.add_step(step_if_action);
    sequence.add_step(step_elseif1);
    sequence.add_step(step_elseif1_action);
    sequence.add_step(step_elseif2);
    sequence.add_step(step_elseif2_action);
    sequence.add_step(step_if_end);

    SECTION("if-elseif-elseif sequence with if=elseif=elseif=false")
    {
        Context context;
        context["a"] = VariableValue{ 0LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 0LL );
    }

    SECTION("if-elseif-elseif sequence with if=true")
    {
        Context context;
        context["a"] = VariableValue{ 1LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 2LL );
    }

    SECTION("if-elseif-elseif sequence with elseif[1]=true")
    {
        Context context;
        context["a"] = VariableValue{ 2LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 3LL );
    }
    
    SECTION("if-elseif-elseif sequence with elseif[2]=true")
    {
        Context context;
        context["a"] = VariableValue{ 3LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 4LL );
    }
}

TEST_CASE("execute_sequence(): if-elseif-elseif-else sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0/1/2/3

    script:
    if a == 1
        a = 2
    else if a == 2
        a = 3
    else if a == 3
        a = 4
    else
        a = 5
    end

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
    Step step_if_end            {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_imported_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_if_action.set_label("if: set a to 2");
    step_if_action.set_imported_variable_names(VariableNames{"a"});
    step_if_action.set_exported_variable_names(VariableNames{"a"});
    step_if_action.set_script("a = 2");

    step_elseif1.set_label("elseif1");
    step_elseif1.set_imported_variable_names(VariableNames{"a"});
    step_elseif1.set_script("return a == 2");

    step_elseif1_action.set_label("elseif: set a to 3");
    step_elseif1_action.set_imported_variable_names(VariableNames{"a"});
    step_elseif1_action.set_exported_variable_names(VariableNames{"a"});
    step_elseif1_action.set_script("a = 3");

    step_elseif2.set_label("elseif2");
    step_elseif2.set_imported_variable_names(VariableNames{"a"});
    step_elseif2.set_script("return a == 3");

    step_elseif2_action.set_label("else: set a to 4");
    step_elseif2_action.set_imported_variable_names(VariableNames{"a"});
    step_elseif2_action.set_exported_variable_names(VariableNames{"a"});
    step_elseif2_action.set_script("a = 4");

    step_else.set_label("else");
    step_else.set_imported_variable_names(VariableNames{"a"});

    step_else_action.set_label("else: set a to 5");
    step_else_action.set_imported_variable_names(VariableNames{"a"});
    step_else_action.set_exported_variable_names(VariableNames{"a"});
    step_else_action.set_script("a = 5");

    step_if_end.set_label("if: end");

    Sequence sequence;
    sequence.add_step(step_if);
    sequence.add_step(step_if_action);
    sequence.add_step(step_elseif1);
    sequence.add_step(step_elseif1_action);
    sequence.add_step(step_elseif2);
    sequence.add_step(step_elseif2_action);
    sequence.add_step(step_else);
    sequence.add_step(step_else_action);
    sequence.add_step(step_if_end);

    SECTION("if-elseif-elseif sequence with if=elseif=elseif=false")
    {
        Context context;
        context["a"] = VariableValue{ 0LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 5LL );
    }

    SECTION("if-elseif-elseif sequence with if=true")
    {
        Context context;
        context["a"] = VariableValue{ 1LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 2LL );
    }

    SECTION("if-elseif-elseif sequence with elseif[1]=true")
    {
        Context context;
        context["a"] = VariableValue{ 2LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 3LL );
    }
    
    SECTION("if-elseif-elseif sequence with elseif[2]=true")
    {
        Context context;
        context["a"] = VariableValue{ 3LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 4LL );
    }
}

TEST_CASE("execute_sequence(): while sequence", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    while a < 10
        a = a + 1
    end

    post-condition:
    a = 10

    */
    Step step_while          {Step::type_while};
    Step step_while_action   {Step::type_action};
    Step step_while_end      {Step::type_end};

    step_while.set_label("while a < 10");
    step_while.set_imported_variable_names(VariableNames{"a"});
    step_while.set_exported_variable_names(VariableNames{"a"});
    step_while.set_script("return a < 10");

    step_while_action.set_label("while: a = a + 1");
    step_while_action.set_imported_variable_names(VariableNames{"a"});
    step_while_action.set_exported_variable_names(VariableNames{"a"});
    step_while_action.set_script("a = a + 1");

    step_while_end.set_label("while: end");

    Sequence sequence;
    sequence.add_step(step_while);
    sequence.add_step(step_while_action);
    sequence.add_step(step_while_end);

    SECTION("while sequence with while: a<10")
    {
        Context context;
        context["a"] = VariableValue{ 0LL };

        REQUIRE_NOTHROW(execute_sequence(sequence, context));
        REQUIRE(std::get<long long>(context["a"] ) == 10LL );
    }
}

TEST_CASE("execute_sequence(): try sequence with success", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    try
        a = 1
    catch
        a = 2
    end

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
    step_try_action.set_imported_variable_names(VariableNames{"a"});
    step_try_action.set_exported_variable_names(VariableNames{"a"});
    step_try_action.set_script("a = 1");

    step_try_catch.set_label("try: catch");

    step_try_catch_action.set_label("try-catch: a = 2");
    step_try_catch_action.set_imported_variable_names(VariableNames{"a"});
    step_try_catch_action.set_exported_variable_names(VariableNames{"a"});
    step_try_catch_action.set_script("a = 2");

    step_try_end.set_label("try: end");

    Sequence sequence;
    sequence.add_step(step_try);
    sequence.add_step(step_try_action);
    sequence.add_step(step_try_catch);
    sequence.add_step(step_try_catch_action);
    sequence.add_step(step_try_end);

    Context context;
    context["a"] = VariableValue{ 0LL };

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
    REQUIRE(std::get<long long>(context["a"] ) == 1LL );
}

TEST_CASE("execute_sequence(): try sequence with fault", "[execute_sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    try
        a = 1
        this line crash lua
    catch
        a = 2
    end

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
    step_try_action1.set_imported_variable_names(VariableNames{"a"});
    step_try_action1.set_exported_variable_names(VariableNames{"a"});
    step_try_action1.set_script("a = 1");

    step_try_action2.set_label("try[action2]: this line crash lua");
    step_try_action2.set_imported_variable_names(VariableNames{"a"});
    step_try_action2.set_exported_variable_names(VariableNames{"a"});
    step_try_action2.set_script("this line crash lua");

    step_try_catch.set_label("try: catch");

    step_try_catch_action.set_label("try-catch: a = 2");
    step_try_catch_action.set_imported_variable_names(VariableNames{"a"});
    step_try_catch_action.set_exported_variable_names(VariableNames{"a"});
    step_try_catch_action.set_script("a = 2");

    step_try_end.set_label("try: end");

    Sequence sequence;
    sequence.add_step(step_try);
    sequence.add_step(step_try_action1);
    sequence.add_step(step_try_action2);
    sequence.add_step(step_try_catch);
    sequence.add_step(step_try_catch_action);
    sequence.add_step(step_try_end);

    Context context;
    context["a"] = VariableValue{ 0LL };

    REQUIRE_NOTHROW(execute_sequence(sequence, context));
    REQUIRE(std::get<long long>(context["a"] ) == 2LL );
}