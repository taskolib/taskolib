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
#include "taskomat/Sequence.h"

using namespace task;

TEST_CASE("Sequence: Construct empty sequence", "[Sequence]")
{
    Sequence seq;
    REQUIRE(seq.empty());
    REQUIRE(seq.size() == 0);
}

TEST_CASE("Sequence: Constructor without descriptive name", "[Sequence]")
{
    REQUIRE_THROWS_AS(Sequence(""), Error);
}

TEST_CASE("Sequence: Constructor with too long descriptive name", "[Sequence]")
{
    REQUIRE_THROWS_AS(Sequence(std::string(Sequence::max_label_length + 1, 'c')), Error);
    REQUIRE_NOTHROW(Sequence(std::string(Sequence::max_label_length, 'c')));
}

TEST_CASE("Sequence: empty()", "[Sequence]")
{
    Sequence seq;
    REQUIRE(seq.empty());
    seq.add_step(Step{});
    REQUIRE(seq.empty() == false);
}

TEST_CASE("Sequence: check correctness of try-catch-end 1", "[Sequence]")
{
    /*
        TRY
            ACTION
        CATCH
        END
    */

    Sequence sequence("validating try-catch-end 1 correctness");

    sequence.add_step(Step{ Step::type_try });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_catch });
    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 4u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_correctness_of_steps());
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

    Sequence sequence("validating try-catch-end correctness");

    sequence.add_step(Step{ Step::type_try });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_catch });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 5u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_correctness_of_steps());
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

//     REQUIRE_NOTHROW(sequence.check_correctness_of_steps());
// }

TEST_CASE("Sequence: check fault for try", "[Sequence]")
{
    /*
        TRY
    */
    Sequence sequence("validating try-catch correctness");

    sequence.add_step(Step{ Step::type_try });

    REQUIRE(sequence.size() == 1u);
    REQUIRE(sequence[0].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for try-try", "[Sequence]")
{
    /*
        TRY
        TRY
    */
    Sequence sequence("validating try-catch correctness");

    sequence.add_step(Step{ Step::type_try });
    sequence.add_step(Step{ Step::type_try });

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for try-catch", "[Sequence]")
{
    /*
        TRY
            ACTION
        CATCH
    */
    Sequence sequence("validating try-catch correctness");

    sequence.add_step(Step{ Step::type_try });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_catch });

    REQUIRE(sequence.size() == 3u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for try-end", "[Sequence]")
{
    /*
        TRY
        END
    */
    Sequence sequence("validating try-end correctness");

    sequence.add_step(Step{ Step::type_try });
    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
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
    Sequence sequence("validating try-catch-catch-end correctness");

    sequence.add_step(Step{ Step::type_try });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_catch });
    sequence.add_step(Step{ Step::type_catch });
    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 5u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 0);
    REQUIRE(sequence[4].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check correctness of if-end", "[Sequence]")
{
    /*
        IF
            ACTION
        END
    */
    Sequence sequence("validating if-end correctness");

    sequence.add_step(Step{ Step::type_if });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 3u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_correctness_of_steps());
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
    Sequence sequence("validating if-else-end correctness");

    sequence.add_step(Step{ Step::type_if });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_else });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 5u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_correctness_of_steps());
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
    Sequence sequence("validating if-else-end correctness");

    sequence.add_step(Step{ Step::type_if });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_elseif });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_else });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 7u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 0);
    REQUIRE(sequence[5].get_indentation_level() == 1);
    REQUIRE(sequence[6].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_correctness_of_steps());
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
    Sequence sequence("validating if-else-end correctness");

    sequence.add_step(Step{ Step::type_if });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_elseif });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_elseif });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_else });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 9u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 0);
    REQUIRE(sequence[5].get_indentation_level() == 1);
    REQUIRE(sequence[6].get_indentation_level() == 0);
    REQUIRE(sequence[7].get_indentation_level() == 1);
    REQUIRE(sequence[8].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_correctness_of_steps());
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
    Sequence sequence("validating if-elseif-try-catch-end-else-end correctness");

    sequence.add_step(Step{ Step::type_if });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_elseif });
    sequence.add_step(Step{ Step::type_try });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_catch });
    sequence.add_step(Step{ Step::type_end });
    sequence.add_step(Step{ Step::type_else });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 10u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 2);
    REQUIRE(sequence[5].get_indentation_level() == 1);
    REQUIRE(sequence[6].get_indentation_level() == 1);
    REQUIRE(sequence[7].get_indentation_level() == 0);
    REQUIRE(sequence[8].get_indentation_level() == 1);
    REQUIRE(sequence[9].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_THROWS_AS(sequence.check_correctness_of_steps(), Error);
}

TEST_CASE("Sequence: check correctness of if-elseif-try-catch-end-elseif-end",
"[Sequence]")
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
    Sequence sequence("validating if-elseif-try-catch-end-else-end correctness");

    sequence.add_step(Step{ Step::type_if });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_elseif });
    sequence.add_step(Step{ Step::type_try });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_catch });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_end });
    sequence.add_step(Step{ Step::type_else });
    sequence.add_step(Step{ Step::type_action });
    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 11u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 2);
    REQUIRE(sequence[5].get_indentation_level() == 1);
    REQUIRE(sequence[6].get_indentation_level() == 2);
    REQUIRE(sequence[7].get_indentation_level() == 1);
    REQUIRE(sequence[8].get_indentation_level() == 0);
    REQUIRE(sequence[9].get_indentation_level() == 1);
    REQUIRE(sequence[10].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_correctness_of_steps());
}

// TODO: Nested levels -> needs more improvement!
// TEST_CASE("Sequence: check correctness of if-elseif-while-end-else-end - we need an "
// "AST implementation!", "[Sequence]")
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

//     REQUIRE_NOTHROW(sequence.check_correctness_of_steps());
// }

TEST_CASE("Sequence: check fault for end", "[Sequence]")
{
    /*
        END
    */
    Sequence sequence("validating end correctness");

    sequence.add_step(Step{ Step::type_end });

    REQUIRE(sequence.size() == 1u);
    REQUIRE(sequence[0].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-action", "[Sequence]")
{
    /*
        END
        ACTION
    */
    Sequence sequence("validating end-action correctness");

    sequence.add_step(Step{ Step::type_end });
    sequence.add_step(Step{ Step::type_action });

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-try", "[Sequence]")
{
    /*
        END
        TRY
    */
    Sequence sequence("validating end-try correctness");

    sequence.add_step(Step{ Step::type_end });
    sequence.add_step(Step{ Step::type_try });

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-catch", "[Sequence]")
{
    /*
        END
        CATCH
    */
    Sequence sequence("validating end-catch correctness");

    sequence.add_step(Step{ Step::type_end });
    sequence.add_step(Step{ Step::type_catch });

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-if", "[Sequence]")
{
    /*
        END
        IF
    */
    Sequence sequence("validating end-if correctness");

    sequence.add_step(Step{ Step::type_end });
    sequence.add_step(Step{ Step::type_if });

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-elseif", "[Sequence]")
{
    /*
        END
        ELSE IF
    */
    Sequence sequence("validating end-elseif correctness");

    sequence.add_step(Step{ Step::type_end });
    sequence.add_step(Step{ Step::type_elseif });

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-else", "[Sequence]")
{
    /*
        END
        ELSE
    */
    Sequence sequence("validating end-else correctness");

    sequence.add_step(Step{ Step::type_end });
    sequence.add_step(Step{ Step::type_else });

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}

TEST_CASE("Sequence: check fault for end-while", "[Sequence]")
{
    /*
        END
        WHILE
    */
    Sequence sequence("validating end-while correctness");

    sequence.add_step(Step{ Step::type_end });
    sequence.add_step(Step{ Step::type_while });

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_correctness_of_steps(), Error );
}
