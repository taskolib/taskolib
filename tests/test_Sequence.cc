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
#include "../include/taskomat/Sequence.h"

using namespace task;

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

TEST_CASE("Sequence: check correctness of try-catch-end 1", "[Sequence]")
{
    /*
        TRY
            ACTION
        CATCH
        END
    */
    Step step_try;
    Step step_action;
    Step step_catch;
    Step step_try_end;
    Context context;
    Sequence sequence("validating try-catch-end 1 correctness");

    step_try.set_type( Step::type_try );
    step_action.set_type( Step::type_action );
    step_catch.set_type( Step::type_catch );
    step_try_end.set_type( Step::type_end );

    sequence.add_step( step_try );
    sequence.add_step( step_action );
    sequence.add_step( step_catch );
    sequence.add_step( step_try_end );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check correctness of try-catch-end 2", "[Sequence]")
{
    /*
        TRY
            ACTION
        CATCH
            ACTION
        END
    */
    Step step_try;
    Step step_action1;
    Step step_catch;
    Step step_action2;
    Step step_try_end;
    Context context;
    Sequence sequence("validating try-catch-end correctness");

    step_try.set_type( Step::type_try );
    step_action1.set_type( Step::type_action );
    step_catch.set_type( Step::type_catch );
    step_action2.set_type( Step::type_action );
    step_try_end.set_type( Step::type_end );

    sequence.add_step( step_try );
    sequence.add_step( step_action1 );
    sequence.add_step( step_catch );
    sequence.add_step( step_action2 );
    sequence.add_step( step_try_end );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

// TODO: Nested levels -> needs more improvement!
// TEST_CASE("Sequence: check correctness of try-try-catch-end-catch-end", "[Sequence]")
// {
//     /*
//         TRY
//             TRY
//                 ACTION
//             CATCH
//                 ACTION
//             END
//         CATCH
//             ACTION
//         END
//     */
//     Step step_try1;
//     Step step_try2;
//     Step step_action1;
//     Step step_catch2;
//     Step step_action2;
//     Step step_try_end2;
//     Step step_catch1;
//     Step step_action3;
//     Step step_try_end1;
//     Context context;
//     Sequence sequence("validating try-try-catch-end-catch-end correctness");

//     step_try1.set_type( Step::type_try );
//     step_try2.set_type( Step::type_try );
//     step_action1.set_type( Step::type_action );
//     step_catch2.set_type( Step::type_catch );
//     step_action2.set_type( Step::type_action );
//     step_try_end2.set_type( Step::type_end );
//     step_catch1.set_type( Step::type_catch );
//     step_action3.set_type( Step::type_action );
//     step_try_end1.set_type( Step::type_end );

//     sequence.add_step( step_try1 );
//     sequence.add_step( step_try2 );
//     sequence.add_step( step_action1 );
//     sequence.add_step( step_catch2 );
//     sequence.add_step( step_action2 );
//     sequence.add_step( step_try_end2 );
//     sequence.add_step( step_catch1 );
//     sequence.add_step( step_action3 );
//     sequence.add_step( step_try_end1 );

//     REQUIRE( sequence.check_correctness_of_steps() == true );
// }

TEST_CASE("Sequence: check fault for try", "[Sequence]")
{
    /*
        TRY
    */
    Step step_try;
    Context context;
    Sequence sequence("validating try-catch correctness");

    step_try.set_type( Step::type_try );

    sequence.add_step( step_try );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for try-try", "[Sequence]")
{
    /*
        TRY
        TRY
    */
    Step step_try1;
    Step step_try2;
    Context context;
    Sequence sequence("validating try-catch correctness");

    step_try1.set_type( Step::type_try );
    step_try2.set_type( Step::type_try );

    sequence.add_step( step_try1 );
    sequence.add_step( step_try2 );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for try-catch", "[Sequence]")
{
    /*
        TRY
            ACTION
        CATCH
    */
    Step step_try;
    Step step_action;
    Step step_catch;
    Context context;
    Sequence sequence("validating try-catch correctness");

    step_try.set_type( Step::type_try );
    step_action.set_type( Step::type_action );
    step_catch.set_type( Step::type_catch );

    sequence.add_step( step_try );
    sequence.add_step( step_action );
    sequence.add_step( step_catch );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for try-end", "[Sequence]")
{
    /*
        TRY
        END
    */
    Step step_try;
    Step step_try_end;
    Context context;
    Sequence sequence("validating try-end correctness");

    step_try.set_type( Step::type_try );
    step_try_end.set_type( Step::type_end );

    sequence.add_step( step_try );
    sequence.add_step( step_try_end );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for try-catch-catch-end", "[Sequence]")
{
    /*
        TRY
            ACTION
        CATCH
        CATCH
        END
    */
    Step step_try;
    Step step_action;
    Step step_catch1;
    Step step_catch2;
    Step step_try_end;
    Context context;
    Sequence sequence("validating try-catch-catch-end correctness");

    step_try.set_type( Step::type_try );
    step_action.set_type( Step::type_action );
    step_catch1.set_type( Step::type_catch );
    step_catch2.set_type( Step::type_catch );
    step_try_end.set_type( Step::type_end );

    sequence.add_step( step_try );
    sequence.add_step( step_action );
    sequence.add_step( step_catch1 );
    sequence.add_step( step_catch2 );
    sequence.add_step( step_try_end );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check correctness of if-end", "[Sequence]")
{
    /*
        IF
            ACTION
        END
    */
    Step step_if;
    Step step_action; // set implicit type to 'action'
    Step step_if_end;
    Context context;
    Sequence sequence("validating if-end correctness");

    step_if.set_type( Step::type_if );
    step_action.set_type( Step::type_action );
    step_if_end.set_type( Step::type_end );

    sequence.add_step( step_if );
    sequence.add_step( step_action );
    sequence.add_step( step_if_end );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check correctness of if-else-end", "[Sequence]")
{
    /*
        IF
            ACTION
        ELSE
            ACTION
        END
    */
    Step step_if;
    Step step_action1; // set implicit type to 'action'
    Step step_if_else;
    Step step_action2; // set implicit type to 'action'
    Step step_if_end;
    Context context;
    Sequence sequence("validating if-else-end correctness");

    step_if.set_type( Step::type_if );
    step_action1.set_type( Step::type_action );
    step_if_else.set_type( Step::type_else );
    step_action2.set_type( Step::type_action );
    step_if_end.set_type( Step::type_end );

    sequence.add_step( step_if );
    sequence.add_step( step_action1 );
    sequence.add_step( step_if_else );
    sequence.add_step( step_action2 );
    sequence.add_step( step_if_end );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check correctness of if-elseif-else-end", "[Sequence]")
{
    /*
        IF
            ACTION
        ELSE IF
            ACTION
        ELSE
            ACTION
        END
    */
    Step step_if;
    Step step_action1; // set implicit type to 'action'
    Step step_if_else_if;
    Step step_action2; // set implicit type to 'action'
    Step step_if_else;
    Step step_action3; // set implicit type to 'action'
    Step step_if_end;
    Context context;
    Sequence sequence("validating if-else-end correctness");

    step_if.set_type( Step::type_if );
    step_action1.set_type( Step::type_action );
    step_if_else_if.set_type( Step::type_elseif );
    step_action2.set_type( Step::type_action );
    step_if_else.set_type( Step::type_else );
    step_action3.set_type( Step::type_action );
    step_if_end.set_type( Step::type_end );

    sequence.add_step( step_if );
    sequence.add_step( step_action1 );
    sequence.add_step( step_if_else_if );
    sequence.add_step( step_action2 );
    sequence.add_step( step_if_else );
    sequence.add_step( step_action3 );
    sequence.add_step( step_if_end );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check correctness of if-elseif-elseif-else-end", "[Sequence]")
{
    /*
        IF
            ACTION
        ELSE IF <cond>
            ACTION
        ELSE IF <cond>
            ACTION
        ELSE
            ACTION
        END
    */
    Step step_if;
    Step step_action1; // set implicit type to 'action'
    Step step_if_else_if1;
    Step step_action2; // set implicit type to 'action'
    Step step_if_else_if2;
    Step step_action3; // set implicit type to 'action'
    Step step_if_else;
    Step step_action4; // set implicit type to 'action'
    Step step_if_end;
    Context context;
    Sequence sequence("validating if-else-end correctness");

    step_if.set_type( Step::type_if );
    step_action1.set_type( Step::type_action );
    step_if_else_if1.set_type( Step::type_elseif );
    step_action2.set_type( Step::type_action );
    step_if_else_if2.set_type( Step::type_elseif );
    step_action3.set_type( Step::type_action );
    step_if_else.set_type( Step::type_else );
    step_action4.set_type( Step::type_action );
    step_if_end.set_type( Step::type_end );

    sequence.add_step( step_if );
    sequence.add_step( step_action1 );
    sequence.add_step( step_if_else_if1 );
    sequence.add_step( step_action2 );
    sequence.add_step( step_if_else_if2 );
    sequence.add_step( step_action3 );
    sequence.add_step( step_if_else );
    sequence.add_step( step_action4 );
    sequence.add_step( step_if_end );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

TEST_CASE("Sequence: check fault of if-elseif-try-catch-end-elseif-end", "[Sequence]")
{
    /*
        IF
            ACTION
        ELSE IF <cond>
            TRY
                ACTION
            CATCH
            END
        ELSE IF <cond>
            ACTION
        END
    */
    Step step_if;
    Step step_action1; // set implicit type to 'action'
    Step step_if_else_if;
    Step step_try;
    Step step_action2; // set implicit type to 'action'
    Step step_try_catch;
    Step step_try_end;
    Step step_if_else;
    Step step_action3; // set implicit type to 'action'
    Step step_if_end;
    Context context;
    Sequence sequence("validating if-elseif-try-catch-end-else-end correctness");

    step_if.set_type( Step::type_if );
    step_action1.set_type( Step::type_action );
    step_if_else_if.set_type( Step::type_elseif );
    step_try.set_type( Step::type_try );
    step_action2.set_type( Step::type_action );
    step_try_catch.set_type( Step::type_catch );
    step_try_end.set_type( Step::type_end );
    step_if_else.set_type( Step::type_else );
    step_action3.set_type( Step::type_action );
    step_if_end.set_type( Step::type_end );

    sequence.add_step( step_if );
    sequence.add_step( step_action1 );
    sequence.add_step( step_if_else_if );
    sequence.add_step( step_try );
    sequence.add_step( step_action2 );
    sequence.add_step( step_try_catch );
    sequence.add_step( step_try_end );
    sequence.add_step( step_if_else );
    sequence.add_step( step_action3 );
    sequence.add_step( step_if_end );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps() == true, Error );
}

TEST_CASE("Sequence: check correctness of if-elseif-try-catch-end-elseif-end", "[Sequence]")
{
    /*
        IF
            ACTION
        ELSE IF <cond>
            TRY
                ACTION
            CATCH
                ACTION
            END
        ELSE IF <cond>
            ACTION
        END
    */
    Step step_if;
    Step step_action1; // set implicit type to 'action'
    Step step_if_else_if;
    Step step_try;
    Step step_action2; // set implicit type to 'action'
    Step step_try_catch;
    Step step_action3; // set implicit type to 'action'
    Step step_try_end;
    Step step_if_else;
    Step step_action4; // set implicit type to 'action'
    Step step_if_end;
    Context context;
    Sequence sequence("validating if-elseif-try-catch-end-else-end correctness");

    step_if.set_type( Step::type_if );
    step_action1.set_type( Step::type_action );
    step_if_else_if.set_type( Step::type_elseif );
    step_try.set_type( Step::type_try );
    step_action2.set_type( Step::type_action );
    step_try_catch.set_type( Step::type_catch );
    step_action3.set_type( Step::type_action );
    step_try_end.set_type( Step::type_end );
    step_if_else.set_type( Step::type_else );
    step_action4.set_type( Step::type_action );
    step_if_end.set_type( Step::type_end );

    sequence.add_step( step_if );
    sequence.add_step( step_action1 );
    sequence.add_step( step_if_else_if );
    sequence.add_step( step_try );
    sequence.add_step( step_action2 );
    sequence.add_step( step_try_catch );
    sequence.add_step( step_action3 );
    sequence.add_step( step_try_end );
    sequence.add_step( step_if_else );
    sequence.add_step( step_action4 );
    sequence.add_step( step_if_end );

    REQUIRE( sequence.check_correctness_of_steps() == true );
}

// TODO: Nested levels -> needs more improvement!
// TEST_CASE("Sequence: check correctness of if-elseif-while-end-else-end - we need an AST implementation!", "[Sequence]")
// {
//     /*
//         IF
//             ACTION
//         ELSE IF
//             WHILE
//             END
//         ELSE
//             ACTION
//         END
//     */
//     Step step_if;
//     Step step_action1;
//     Step step_if_else_if;
//     Step step_while;
//     Step step_whileEnd;
//     Step step_if_else;
//     Step step_action2;
//     Step step_if_end;
//     Context context;
//     Sequence sequence("validating if-elseif-while-end-else-end correctness");

//     step_if.set_type( Step::type_if );
//     step_action1.set_type( Step::type_action );
//     step_if_else_if.set_type( Step::type_elseif );
//     step_while.set_type( Step::type_while );
//     step_whileEnd.set_type( Step::type_end );
//     step_if_else.set_type( Step::type_else );
//     step_action2.set_type( Step::type_action );
//     step_if_end.set_type( Step::type_end );

//     sequence.add_step( step_if );
//     sequence.add_step( step_action1 );
//     sequence.add_step( step_if_else_if );
//     sequence.add_step( step_while );
//     sequence.add_step( step_whileEnd );
//     sequence.add_step( step_if_else );
//     sequence.add_step( step_action2 );
//     sequence.add_step( step_if_end );

//     REQUIRE( sequence.check_correctness_of_steps() == true );
// }

TEST_CASE("Sequence: check fault for end", "[Sequence]")
{
    /*
        END
    */
    Step step_end;
    Context context;
    Sequence sequence("validating end correctness");

    step_end.set_type( Step::type_end );

    sequence.add_step( step_end );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-action", "[Sequence]")
{
    /*
        END
        ACTION
    */
    Step step_end;
    Step step_action;
    Context context;
    Sequence sequence("validating end-action correctness");

    step_end.set_type( Step::type_end );
    step_action.set_type( Step::type_action );

    sequence.add_step( step_end );
    sequence.add_step( step_action );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-try", "[Sequence]")
{
    /*
        END
        TRY
    */
    Step step_end;
    Step step_try;
    Context context;
    Sequence sequence("validating end-try correctness");

    step_end.set_type( Step::type_end );
    step_try.set_type( Step::type_try );

    sequence.add_step( step_end );
    sequence.add_step( step_try );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-catch", "[Sequence]")
{
    /*
        END
        CATCH
    */
    Step step_end;
    Step step_catch;
    Context context;
    Sequence sequence("validating end-catch correctness");

    step_end.set_type( Step::type_end );
    step_catch.set_type( Step::type_catch );

    sequence.add_step( step_end );
    sequence.add_step( step_catch );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-if", "[Sequence]")
{
    /*
        END
        IF
    */
    Step step_end;
    Step step_if;
    Context context;
    Sequence sequence("validating end-if correctness");

    step_end.set_type( Step::type_end );
    step_if.set_type( Step::type_if );

    sequence.add_step( step_end );
    sequence.add_step( step_if );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-elseif", "[Sequence]")
{
    /*
        END
        ELSE IF
    */
    Step step_end;
    Step step_else_if;
    Context context;
    Sequence sequence("validating end-elseif correctness");

    step_end.set_type( Step::type_end );
    step_else_if.set_type( Step::type_elseif );

    sequence.add_step( step_end );
    sequence.add_step( step_else_if );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-else", "[Sequence]")
{
    /*
        END
        ELSE
    */
    Step step_end;
    Step step_else;
    Context context;
    Sequence sequence("validating end-else correctness");

    step_end.set_type( Step::type_end );
    step_else.set_type( Step::type_else );

    sequence.add_step( step_end );
    sequence.add_step( step_else );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-while", "[Sequence]")
{
    /*
        END
        WHILE
    */
    Step step_end;
    Step step_while;
    Context context;
    Sequence sequence("validating end-while correctness");

    step_end.set_type( Step::type_end );
    step_while.set_type( Step::type_while );

    sequence.add_step( step_end );
    sequence.add_step( step_while );

    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}
