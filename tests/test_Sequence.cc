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

    static_assert(std::is_default_constructible<Sequence>::value,
        "Sequence is_default_constructible");
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
    seq.push_back(Step{});
    REQUIRE(seq.empty() == false);
}

TEST_CASE("Sequence: append Step to the end of Sequence", "[Sequence]")
{
    Sequence seq;

    SECTION("append lvalue Step")
    {
        seq.push_back(Step{});
        REQUIRE(seq.empty() == false);
        REQUIRE(seq.size() == 1);

        seq.push_back(Step{});
        REQUIRE(seq.size() == 2);
    }

    SECTION("append rvalue Step")
    {
        seq.push_back(Step{});
        REQUIRE(seq.empty() == false);
        REQUIRE(seq.size() == 1);

        seq.push_back(std::forward<Step>(Step{}));
        REQUIRE(seq.size() == 2);
    }
}

TEST_CASE("Sequence: remove last Steps from Sequence", "[Sequence]")
{
    Sequence seq;
    seq.push_back(Step{});
    seq.push_back(Step{});
    REQUIRE(seq.empty() == false);
    REQUIRE(seq.size() == 2);

    seq.pop_back();
    REQUIRE(seq.size() == 1);
    
    seq.pop_back();
    REQUIRE(seq.size() == 0);
    REQUIRE(seq.empty());

    seq.pop_back(); // remove from an already empty sequence again the last step
    REQUIRE(seq.size() == 0);
    REQUIRE(seq.empty());
}

TEST_CASE("Sequence: insert Step to Sequence", "[Sequence]")
{
    Sequence seq;
    seq.push_back(Step{Step::type_action});
    seq.push_back(Step{Step::type_action});

    SECTION("insert lvalue reference (position)")
    {
        Step insertStep = Step{Step::type_if};
        seq.insert(1, insertStep);

        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }

    SECTION("insert lvalue reference (position), position is out of range")
    {
        Step insertStep = Step{Step::type_if};
        seq.insert(3, insertStep);

        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());

        Step::Type expected[] = {Step::type_action, Step::type_action, Step::type_if};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }

    SECTION("insert const lvalue reference (position)")
    {
        const Step insertStep = Step{Step::type_if};
        seq.insert(1, insertStep);

        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }

    SECTION("insert const lvalue reference (position), position is out of range")
    {
        const Step insertStep = Step{Step::type_if};
        seq.insert(3, insertStep);

        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());

        Step::Type expected[] = {Step::type_action, Step::type_action, Step::type_if};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }

    SECTION("insert rvalue reference (position)")
    {
        seq.insert(1, Step{Step::type_if});

        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }

    SECTION("insert rvalue reference (position), position is out of range")
    {
        seq.insert(3, Step{Step::type_if});

        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());

        Step::Type expected[] = {Step::type_action, Step::type_action, Step::type_if};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }

    SECTION("insert rvalue reference (position, std::forward)")
    {
        seq.insert(1, std::forward<Step>(Step{Step::type_if}));

        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }

    SECTION("insert rvalue reference (position, std::forward), position is out of range")
    {
        seq.insert(3, std::forward<Step>(Step{Step::type_if}));

        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());

        Step::Type expected[] = {Step::type_action, Step::type_action, Step::type_if};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }
    
    SECTION("insert lvalue reference (iterator)")
    {
        Step insertStep = Step{Step::type_if};
        auto iter = seq.insert(seq.begin()+1, insertStep);
        // TODO: what happens here: auto iter = seq.insert(seq.end()+1, Step{Step::type_if});
        
        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());
        REQUIRE(Step::type_if == (*iter).get_type());

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }
    
    SECTION("insert const lvalue reference (iterator)")
    {
        const Step insertStep = Step{Step::type_if};
        auto iter = seq.insert(seq.begin()+1, insertStep);
        // TODO: what happens here: auto iter = seq.insert(seq.end()+1, Step{Step::type_if});
        
        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());
        REQUIRE(Step::type_if == (*iter).get_type());

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }
    
    SECTION("insert rvalue reference (iterator)")
    {
        auto iter = seq.insert(seq.begin()+1, Step{Step::type_if});
        // TODO: what happens here: auto iter = seq.insert(seq.end()+1, Step{Step::type_if});
        
        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());
        REQUIRE(Step::type_if == (*iter).get_type());

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }
    
    SECTION("insert rvalue reference (iterator, std::forward)")
    {
        auto iter = seq.insert(seq.begin()+1, std::forward<Step>(Step{Step::type_if}));

        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());
        REQUIRE(Step::type_if == (*iter).get_type());

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }
}

TEST_CASE("Sequence: erase Step(s) from Sequence", "[Sequence]")
{
    Sequence seq;
    seq.push_back(Step{Step::type_action});
    seq.push_back(Step{Step::type_while});
    seq.push_back(Step{Step::type_action});
    seq.push_back(Step{Step::type_end});
    seq.push_back(Step{Step::type_action});

    SECTION("erase step (position)")
    {
        auto erase = seq.erase(1);
        REQUIRE(not seq.empty());
        REQUIRE(4 == seq.size());
        REQUIRE(Step::type_action == (*erase).get_type());
    }

    SECTION("erase step (position), error")
    {
        REQUIRE_THROWS_AS(seq.erase(6), Error);
        REQUIRE(not seq.empty());
        REQUIRE(5 == seq.size());
    }

    SECTION("erase step 1-3 (position)")
    {
        auto erase = seq.erase(1, 4);
        REQUIRE(not seq.empty());
        REQUIRE(2 == seq.size());
        REQUIRE(Step::type_action == (*erase).get_type());
    }

    SECTION("erase step 3-1 (position), error")
    {
        REQUIRE_THROWS_AS(seq.erase(3, 1), Error);
        REQUIRE(not seq.empty());
        REQUIRE(5 == seq.size());
    }

    SECTION("erase step 5-6 (position), error")
    {
        REQUIRE_THROWS_AS(seq.erase(5, 6), Error);
        REQUIRE(not seq.empty());
        REQUIRE(5 == seq.size());
    }

    SECTION("erase step 6-5 (position), error")
    {
        REQUIRE_THROWS_AS(seq.erase(6, 5), Error);
        REQUIRE(not seq.empty());
        REQUIRE(5 == seq.size());
    }

    SECTION("erase first (iterator)")
    {
        auto erase = seq.erase(seq.begin());
        REQUIRE(not seq.empty());
        REQUIRE(4 == seq.size());
        REQUIRE(Step::type_while == (*erase).get_type());
    }

    SECTION("erase middle (iterator)")
    {
        auto erase = seq.erase(seq.begin()+2);
        REQUIRE(not seq.empty());
        REQUIRE(4 == seq.size());
        REQUIRE(Step::type_end == (*erase).get_type());
    }

    SECTION("erase last (iterator)")
    {
        auto erase = seq.erase(seq.end()-1);
        REQUIRE(not seq.empty());
        REQUIRE(4 == seq.size());
        REQUIRE(Step::type_action == (*erase).get_type());
    }

    SECTION("erase end (iterator)")
    {
        auto erase = seq.erase(seq.end());
        REQUIRE(not seq.empty());
        REQUIRE(4 == seq.size());
    }

    SECTION("erase range from beginning (iterator)")
    {
        auto erase = seq.erase(seq.begin(), seq.begin()+2);
        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());
        REQUIRE(Step::type_action == (*erase).get_type());
    }

    SECTION("erase range from middle (iterator)")
    {
        auto erase = seq.erase(seq.begin()+1, seq.begin()+3);
        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());
        REQUIRE(Step::type_end == (*erase).get_type());
    }

    SECTION("erase range from end (iterator)")
    {
        auto erase = seq.erase(seq.end()-3, seq.end()-1);
        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());
        REQUIRE(Step::type_action == (*erase).get_type());
    }

    SECTION("erase range from end (iterator)")
    {
        auto erase = seq.erase(seq.end()-2, seq.end());
        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());
        REQUIRE(Step::type_end == (*erase).get_type());
    }
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

    sequence.push_back(Step{Step::type_try});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_catch});
    sequence.push_back(Step{Step::type_end});

    REQUIRE(sequence.size() == 4u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
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

    sequence.push_back(Step{Step::type_try});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_catch});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});

    REQUIRE(sequence.size() == 5u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_syntax());
}

TEST_CASE("Sequence: check correctness of try-try-catch-end-catch-end", "[Sequence]")
{
    /*
        TRY
            TRY
                ACTION
            CATCH
                ACTION
            END
        CATCH
            ACTION
        END
    */
    Sequence sequence("validating try-try-catch-end-catch-end correctness");

    sequence.push_back(Step{Step::type_try});
    sequence.push_back(Step{Step::type_try});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_catch});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_catch});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});

    Context context;

    REQUIRE_NOTHROW(sequence.check_syntax());
}

TEST_CASE("Sequence: check fault for try", "[Sequence]")
{
    /*
        TRY
    */
    Sequence sequence("validating try-catch correctness");

    sequence.push_back(Step{Step::type_try});

    REQUIRE(sequence.size() == 1u);
    REQUIRE(sequence[0].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_syntax(), Error );
}

TEST_CASE("Sequence: check fault for try-try", "[Sequence]")
{
    /*
        TRY
        TRY
    */
    Sequence sequence("validating try-catch correctness");

    sequence.push_back(Step{Step::type_try});
    sequence.push_back(Step{Step::type_try});

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check fault for try-catch", "[Sequence]")
{
    /*
        TRY
            ACTION
        CATCH
    */
    Sequence sequence("validating try-catch correctness");

    sequence.push_back(Step{Step::type_try});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_catch});

    REQUIRE(sequence.size() == 3u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check fault for try-end", "[Sequence]")
{
    /*
        TRY
        END
    */
    Sequence sequence("validating try-end correctness");

    sequence.push_back(Step{Step::type_try});
    sequence.push_back(Step{Step::type_end});

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
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

    sequence.push_back(Step{Step::type_try});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_catch});
    sequence.push_back(Step{Step::type_catch});
    sequence.push_back(Step{Step::type_end});

    REQUIRE(sequence.size() == 5u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 0);
    REQUIRE(sequence[4].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check correctness of if-end", "[Sequence]")
{
    /*
        IF THEN
            ACTION
        END
    */
    Sequence sequence("validating if-end correctness");

    sequence.push_back(Step{Step::type_if});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});

    REQUIRE(sequence.size() == 3u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_syntax());
}

TEST_CASE("Sequence: check correctness of if-else-end", "[Sequence]")
{
    /*
        IF THEN
            ACTION
        ELSE
            ACTION
        END
    */
    Sequence sequence("validating if-else-end correctness");

    sequence.push_back(Step{Step::type_if});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_else});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});

    REQUIRE(sequence.size() == 5u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_syntax());
}

TEST_CASE("Sequence: check correctness of if-elseif-else-end", "[Sequence]")
{
    /*
        IF THEN
            ACTION
        ELSE IF THEN
            ACTION
        ELSE
            ACTION
        END
    */
    Sequence sequence("validating if-else-end correctness");

    sequence.push_back(Step{Step::type_if});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_elseif});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_else});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});

    REQUIRE(sequence.size() == 7u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 0);
    REQUIRE(sequence[5].get_indentation_level() == 1);
    REQUIRE(sequence[6].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() == "");
    REQUIRE_NOTHROW(sequence.check_syntax());
}

TEST_CASE("Sequence: check correctness of if-elseif-elseif-else-end", "[Sequence]")
{
    /*
        IF THEN
            ACTION
        ELSE IF THEN
            ACTION
        ELSE IF THEN
            ACTION
        ELSE
            ACTION
        END
    */
    Sequence sequence("validating if-else-end correctness");

    sequence.push_back(Step{Step::type_if});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_elseif});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_elseif});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_else});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});

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
    REQUIRE_NOTHROW(sequence.check_syntax());
}

TEST_CASE("Sequence: check fault of if-elseif-try-catch-end-elseif-end", "[Sequence]")
{
    /*
        IF THEN
            ACTION
        ELSE IF THEN
            TRY
                ACTION
            CATCH
            END
        ELSE IF THEN
            ACTION
        END
    */
    Sequence sequence("validating if-elseif-try-catch-end-else-end correctness");

    sequence.push_back(Step{Step::type_if});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_elseif});
    sequence.push_back(Step{Step::type_try});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_catch});
    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_else});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});

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
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check correctness of if-elseif-try-catch-end-elseif-end",
"[Sequence]")
{
    /*
        IF THEN
            ACTION
        ELSE IF THEN
            TRY
                ACTION
            CATCH
                ACTION
            END
        ELSE IF THEN
            ACTION
        END
    */
    Sequence sequence("validating if-elseif-try-catch-end-else-end correctness");

    sequence.push_back(Step{Step::type_if});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_elseif});
    sequence.push_back(Step{Step::type_try});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_catch});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_else});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});

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
    REQUIRE_NOTHROW(sequence.check_syntax());
}

TEST_CASE("Sequence: check fault of if-elseif-while-end-else-end", "[Sequence]")
{
    /*
        00: IF THEN
        01:     ACTION
        02: ELSE IF THEN
        03:     WHILE
        04:     END
        05: ELSE
        06:     ACTION
        07: END
    */
    Sequence sequence("validating if-elseif-while-end-else-end correctness");
    sequence.push_back(Step{Step::type_if});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_elseif});
    sequence.push_back(Step{Step::type_while});
    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_elseif});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});

    REQUIRE(sequence.size() == 8u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 1);
    REQUIRE(sequence[5].get_indentation_level() == 0);
    REQUIRE(sequence[6].get_indentation_level() == 1);
    REQUIRE(sequence[7].get_indentation_level() == 0);
    
    Context context;

    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check correctness of if-elseif-while-end-else-end", "[Sequence]")
{
    /*
        00: IF
        01:     ACTION
        02: ELSE IF
        03:     WHILE
        04:         ACTION
        05:     END
        06: ELSE
        07:     ACTION
        08: END
    */
    Sequence sequence("validating if-elseif-while-end-else-end correctness");
    sequence.push_back(Step{Step::type_if});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_elseif});
    sequence.push_back(Step{Step::type_while});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_elseif});
    sequence.push_back(Step{Step::type_action});
    sequence.push_back(Step{Step::type_end});

    REQUIRE(sequence.size() == 9u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 1);
    REQUIRE(sequence[2].get_indentation_level() == 0);
    REQUIRE(sequence[3].get_indentation_level() == 1);
    REQUIRE(sequence[4].get_indentation_level() == 2);
    REQUIRE(sequence[5].get_indentation_level() == 1);
    REQUIRE(sequence[6].get_indentation_level() == 0);
    REQUIRE(sequence[7].get_indentation_level() == 1);
    REQUIRE(sequence[8].get_indentation_level() == 0);

    REQUIRE_NOTHROW(sequence.check_syntax());
}

TEST_CASE("Sequence: check fault for end", "[Sequence]")
{
    /*
        END
    */
    Sequence sequence("validating end correctness");

    sequence.push_back(Step{Step::type_end});

    REQUIRE(sequence.size() == 1u);
    REQUIRE(sequence[0].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check fault for end-action", "[Sequence]")
{
    /*
        END
        ACTION
    */
    Sequence sequence("validating end-action correctness");

    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_action});

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check fault for end-try", "[Sequence]")
{
    /*
        END
        TRY
    */
    Sequence sequence("validating end-try correctness");

    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_try});

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check fault for end-catch", "[Sequence]")
{
    /*
        END
        CATCH
    */
    Sequence sequence("validating end-catch correctness");

    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_catch});

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check fault for end-if", "[Sequence]")
{
    /*
        END
        IF
    */
    Sequence sequence("validating end-if correctness");

    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_if});

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check fault for end-elseif", "[Sequence]")
{
    /*
        END
        ELSE IF
    */
    Sequence sequence("validating end-elseif correctness");

    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_elseif});

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}

TEST_CASE("Sequence: check fault for end-else", "[Sequence]")
{
    /*
        END
        ELSE
    */
    Sequence sequence("validating end-else correctness");

    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_else});

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS( sequence.check_syntax(), Error );
}

TEST_CASE("Sequence: check fault for end-while", "[Sequence]")
{
    /*
        END
        WHILE
    */
    Sequence sequence("validating end-while correctness");

    sequence.push_back(Step{Step::type_end});
    sequence.push_back(Step{Step::type_while});

    REQUIRE(sequence.size() == 2u);
    REQUIRE(sequence[0].get_indentation_level() == 0);
    REQUIRE(sequence[1].get_indentation_level() == 0);

    REQUIRE(sequence.get_indentation_error() != "");
    REQUIRE_THROWS_AS(sequence.check_syntax(), Error);
}
