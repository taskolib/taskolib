/**
 * \file   test_Sequence.cc
 * \author Marcus Walla
 * \date   Created on February 8, 2022
 * \brief  Test suite for the the Sequence class.
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
#include "../include/avtomat/Sequence.h"

using namespace avto;

// Question: why does this test fails?
// TEST_CASE("Sequence: constructor without discriptive name", "[Sequence]")
// {
//     REQUIRE_THROWS_AS( Sequence(""), Error );
// }

// Question: why does this test fails?
// TEST_CASE("Sequence: constructor with a too large discriptive name (65 characters)", "[Sequence]")
// {
//     REQUIRE_THROWS_AS( Sequence("abcdefghijABCDEFGHIJabcdefghijABCDEFGHIJabcdefghijABCDEFGHIJabcde"), Error );
// }

TEST_CASE("Sequence: check correctness of try-catch", "[Sequence]")
{
    Step stepTry;
    Step stepCatch;
    Step stepTryEnd;
    Context context;
    Sequence sequence("validating try-catch-end correctness");

    stepTry.set_type( Step::type_try );
    stepCatch.set_type( Step::type_catch );
    stepTryEnd.set_type( Step::type_end );

    sequence.add_step( stepTry );
    sequence.add_step( stepCatch );
    sequence.add_step( stepTryEnd );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check fault for try-end", "[Sequence]")
{
    Step stepTry;
    Step stepTryEnd;
    Context context;
    Sequence sequence("validating try-end correctness");

    stepTry.set_type( Step::type_try );
    stepTryEnd.set_type( Step::type_end );

    sequence.add_step( stepTry );
    sequence.add_step( stepTryEnd );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for try-catch-catch-end", "[Sequence]")
{
    Step stepTry;
    Step stepCatch1;
    Step stepCatch2;
    Step stepTryEnd;
    Context context;
    Sequence sequence("validating try-catch-catch-end correctness");

    stepTry.set_type( Step::type_try );
    stepCatch1.set_type( Step::type_catch );
    stepCatch2.set_type( Step::type_catch );
    stepTryEnd.set_type( Step::type_end );

    sequence.add_step( stepTry );
    sequence.add_step( stepCatch1 );
    sequence.add_step( stepCatch2 );
    sequence.add_step( stepTryEnd );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check correctness of if-end", "[Sequence]")
{
    Step stepIf;
    Step stepIfEnd;
    Context context;
    Sequence sequence("validating if-end correctness");

    stepIf.set_type( Step::type_if );
    stepIfEnd.set_type( Step::type_end );

    sequence.add_step( stepIf );
    sequence.add_step( stepIfEnd );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check correctness of if-else-end", "[Sequence]")
{
    Step stepIf;
    Step stepIfElse;
    Step stepIfEnd;
    Context context;
    Sequence sequence("validating if-else-end correctness");

    stepIf.set_type( Step::type_if );
    stepIfElse.set_type( Step::type_else );
    stepIfEnd.set_type( Step::type_end );

    sequence.add_step( stepIf );
    sequence.add_step( stepIfElse );
    sequence.add_step( stepIfEnd );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check correctness of if-elseif-end", "[Sequence]")
{
    Step stepIf;
    Step stepIfElseIf;
    Step stepIfEnd;
    Context context;
    Sequence sequence("validating if-elseif-end correctness");

    stepIf.set_type( Step::type_if );
    stepIfElseIf.set_type( Step::type_elseif );
    stepIfEnd.set_type( Step::type_end );

    sequence.add_step( stepIf );
    sequence.add_step( stepIfElseIf );
    sequence.add_step( stepIfEnd );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check correctness of if-elseif-else-end", "[Sequence]")
{
    Step stepIf;
    Step stepIfElseIf;
    Step stepIfElse;
    Step stepIfEnd;
    Context context;
    Sequence sequence("validating if-else-end correctness");

    stepIf.set_type( Step::type_if );
    stepIfElseIf.set_type( Step::type_elseif );
    stepIfElse.set_type( Step::type_else );
    stepIfEnd.set_type( Step::type_end );

    sequence.add_step( stepIf );
    sequence.add_step( stepIfElseIf );
    sequence.add_step( stepIfElse );
    sequence.add_step( stepIfEnd );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check correctness of if-elseif-elseif-else-end", "[Sequence]")
{
    Step stepIf;
    Step stepIfElseIf1;
    Step stepIfElseIf2;
    Step stepIfElse;
    Step stepIfEnd;
    Context context;
    Sequence sequence("validating if-else-end correctness");

    stepIf.set_type( Step::type_if );
    stepIfElseIf1.set_type( Step::type_elseif );
    stepIfElseIf2.set_type( Step::type_elseif );
    stepIfElse.set_type( Step::type_else );
    stepIfEnd.set_type( Step::type_end );

    sequence.add_step( stepIf );
    sequence.add_step( stepIfElseIf1 );
    sequence.add_step( stepIfElseIf2 );
    sequence.add_step( stepIfElse );
    sequence.add_step( stepIfEnd );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check correctness of if-elseif-try-catch-end-else-end", "[Sequence]")
{
    Step stepIf;
    Step stepIfElseIf;
    Step stepTry;
    Step stepTryCatch;
    Step stepTryEnd;
    Step stepIfElse;
    Step stepIfEnd;
    Context context;
    Sequence sequence("validating if-elseif-try-catch-end-else-end correctness");

    stepIf.set_type( Step::type_if );
    stepIfElseIf.set_type( Step::type_elseif );
    stepTry.set_type( Step::type_try );
    stepTryCatch.set_type( Step::type_catch );
    stepTryEnd.set_type( Step::type_end );
    stepIfElse.set_type( Step::type_else );
    stepIfEnd.set_type( Step::type_end );

    sequence.add_step( stepIf );
    sequence.add_step( stepIfElseIf );
    sequence.add_step( stepTry );
    sequence.add_step( stepTryCatch );
    sequence.add_step( stepTryEnd );
    sequence.add_step( stepIfElse );
    sequence.add_step( stepIfEnd );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

// TODO: Improve!
// TEST_CASE("Sequence: check correctness of if-elseif-while-end-else-end", "[Sequence]")
// {
//     Step stepIf;
//     Step stepIfElseIf;
//     Step stepWhile;
//     Step stepWhileEnd;
//     Step stepIfElse;
//     Step stepIfEnd;
//     Context context;
//     Sequence sequence("validating if-elseif-while-end-else-end correctness");

//     stepIf.set_type( Step::type_if );
//     stepIfElseIf.set_type( Step::type_elseif );
//     stepWhile.set_type( Step::type_while );
//     stepWhileEnd.set_type( Step::type_end );
//     stepIfElse.set_type( Step::type_else );
//     stepIfEnd.set_type( Step::type_end );

//     sequence.add_step( stepIf );
//     sequence.add_step( stepIfElseIf );
//     sequence.add_step( stepWhile );
//     sequence.add_step( stepWhileEnd );
//     sequence.add_step( stepIfElse );
//     sequence.add_step( stepIfEnd );

//     REQUIRE( sequence.check_correctness_of_steps() == true );
// }

TEST_CASE("Sequence: check correctness of if-elseif-action-else-end", "[Sequence]")
{
    Step stepIf;
    Step stepIfElseIf;
    Step stepAction;
    Step stepIfElse;
    Step stepIfEnd;
    Context context;
    Sequence sequence("validating if-elseif-action-else-end correctness");

    stepIf.set_type( Step::type_if );
    stepIfElseIf.set_type( Step::type_elseif );
    stepAction.set_type( Step::type_action );
    stepIfElse.set_type( Step::type_else );
    stepIfEnd.set_type( Step::type_end );

    sequence.add_step( stepIf );
    sequence.add_step( stepIfElseIf );
    sequence.add_step( stepAction );
    sequence.add_step( stepIfElse );
    sequence.add_step( stepIfEnd );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check fault for end-action", "[Sequence]")
{
    Step stepEnd;
    Step stepAction;
    Context context;
    Sequence sequence("validating end-action correctness");

    stepEnd.set_type( Step::type_end );
    stepAction.set_type( Step::type_action );

    sequence.add_step( stepEnd );
    sequence.add_step( stepAction );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-try", "[Sequence]")
{
    Step stepEnd;
    Step stepTry;
    Context context;
    Sequence sequence("validating end-try correctness");

    stepEnd.set_type( Step::type_end );
    stepTry.set_type( Step::type_try );

    sequence.add_step( stepEnd );
    sequence.add_step( stepTry );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-catch", "[Sequence]")
{
    Step stepEnd;
    Step stepCatch;
    Context context;
    Sequence sequence("validating end-catch correctness");

    stepEnd.set_type( Step::type_end );
    stepCatch.set_type( Step::type_catch );

    sequence.add_step( stepEnd );
    sequence.add_step( stepCatch );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-if", "[Sequence]")
{
    Step stepEnd;
    Step stepIf;
    Context context;
    Sequence sequence("validating end-if correctness");

    stepEnd.set_type( Step::type_end );
    stepIf.set_type( Step::type_if );

    sequence.add_step( stepEnd );
    sequence.add_step( stepIf );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-elseif", "[Sequence]")
{
    Step stepEnd;
    Step stepElseIf;
    Context context;
    Sequence sequence("validating end-elseif correctness");

    stepEnd.set_type( Step::type_end );
    stepElseIf.set_type( Step::type_elseif );

    sequence.add_step( stepEnd );
    sequence.add_step( stepElseIf );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-else", "[Sequence]")
{
    Step stepEnd;
    Step stepElse;
    Context context;
    Sequence sequence("validating end-else correctness");

    stepEnd.set_type( Step::type_end );
    stepElse.set_type( Step::type_else );

    sequence.add_step( stepEnd );
    sequence.add_step( stepElse );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-while", "[Sequence]")
{
    Step stepEnd;
    Step stepWhile;
    Context context;
    Sequence sequence("validating end-while correctness");

    stepEnd.set_type( Step::type_end );
    stepWhile.set_type( Step::type_while );

    sequence.add_step( stepEnd );
    sequence.add_step( stepWhile );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}
