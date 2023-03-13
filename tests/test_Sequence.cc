/**
 * \file   test_Sequence.cc
 * \author Marcus Walla, Lars Froehlich, Ulf Fini Jastrow
 * \date   Created on February 8, 2022
 * \brief  Test suite for the the Sequence class.
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
#include <gul14/time_util.h>

#include "taskolib/Sequence.h"

using namespace std::literals;
using namespace task;
using Catch::Matchers::Contains;

TEST_CASE("Sequence: Constructor without descriptive label", "[Sequence]")
{
    REQUIRE_THROWS_AS(Sequence{ "" }, Error);
    REQUIRE_THROWS_AS(Sequence{ " \t\v\n\r\f" }, Error);
}

TEST_CASE("Sequence: Constructor with descriptive label", "[Sequence]")
{
    // label is to many characters -> throws Error
    REQUIRE_THROWS_AS(Sequence{ std::string(Sequence::max_label_length + 1, 'c') },
        Error);
    // minimum label length with one character
    REQUIRE_NOTHROW(Sequence{ "S" });
    // label length with all characters filled
    REQUIRE_NOTHROW(Sequence{ std::string(Sequence::max_label_length, 'c') });
}

TEST_CASE("Sequence: Constructor with get label", "[Sequence]")
{
    SECTION("label with leading blanks")
    {
        Sequence seq{ " test_sequence" };
        REQUIRE(seq.get_label() == "test_sequence" );
    }

    SECTION("label with trailing blanks")
    {
        Sequence seq{ "test_sequence " };
        REQUIRE(seq.get_label() == "test_sequence" );
    }

    SECTION("label with leading tab")
    {
        Sequence seq{ "\ttest_sequence" };
        REQUIRE(seq.get_label() == "test_sequence" );
    }

    SECTION("label with trailing tab")
    {
        Sequence seq{ "test_sequence\t" };
        REQUIRE(seq.get_label() == "test_sequence" );
    }

    SECTION("label surrounded with multiple whitespaces")
    {
        Sequence seq{ " \t\r\n\v\ftest_sequence \r\t\v\f\n" };
        REQUIRE(seq.get_label() == "test_sequence" );
    }
}

TEST_CASE("Sequence: Construct an empty Sequence", "[Sequence]")
{
    Sequence s{ "test_sequence" };
    REQUIRE(s.size() == 0);
    REQUIRE(s.empty());
    REQUIRE(s.begin() == s.end());
    REQUIRE(s.cbegin() == s.cend());
}

TEST_CASE("Sequence: get and set sequence label", "[Sequence]")
{
    Sequence s{ "test_sequence" };
    REQUIRE(s.get_label() == "test_sequence");

    s.set_label("modified_test_sequence");
    REQUIRE(s.get_label() == "modified_test_sequence");

    REQUIRE_THROWS_AS(s.set_label(std::string(Sequence::max_label_length + 1, 'a')),
        Error);
    REQUIRE_THROWS_AS(s.set_label(""), Error);
    REQUIRE_NOTHROW(s.set_label(std::string(Sequence::max_label_length, 'c')));
}

TEST_CASE("Sequence: assign()", "[Sequence]")
{
    Sequence seq{ "test_sequence" };
    seq.push_back(Step{Step::type_action});
    seq.push_back(Step{Step::type_action});
    seq.push_back(Step{Step::type_action});

    SECTION("assign step as lvalue (iterator)")
    {
        Step step{Step::type_if};
        seq.assign(seq.begin()+1, step);

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }

    SECTION("assign step as rvalue (iterator)")
    {
        seq.assign(seq.begin()+1, Step{Step::type_if});

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }
}

TEST_CASE("Sequence: empty()", "[Sequence]")
{
    Sequence seq{ "test_sequence" };
    REQUIRE(seq.empty());
    seq.push_back(Step{});
    REQUIRE(seq.empty() == false);
}

TEST_CASE("Sequence: erase()", "[Sequence]")
{
    Sequence seq{ "test_sequence" };
    seq.push_back(Step{Step::type_action});
    seq.push_back(Step{Step::type_while});
    seq.push_back(Step{Step::type_action});
    seq.push_back(Step{Step::type_end});
    seq.push_back(Step{Step::type_action});

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
        seq.erase(seq.end());
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

TEST_CASE("Sequence: get_error()", "[Sequence]")
{
    Sequence seq{ "test_sequence" };
    REQUIRE(seq.get_error().has_value() == false);

    seq.set_error(Error{ "Test", gul14::nullopt });
    REQUIRE(seq.get_error().has_value() == true);
    REQUIRE(seq.get_error()->what() == "Test"s);
    REQUIRE(seq.get_error()->get_index().has_value() == false);

    seq.set_error(Error{ "Test2", 42 });
    REQUIRE(seq.get_error().has_value() == true);
    REQUIRE(seq.get_error()->what() == "Test2"s);
    REQUIRE(seq.get_error()->get_index().has_value() == true);
    REQUIRE(seq.get_error()->get_index().value() == 42);
}

TEST_CASE("Sequence: insert()", "[Sequence]")
{
    Sequence seq{ "test_sequence" };
    seq.push_back(Step{Step::type_action});
    seq.push_back(Step{Step::type_action});

    SECTION("insert lvalue reference (iterator)")
    {
        Step insertStep{Step::type_if};
        auto iter = seq.insert(seq.begin()+1, insertStep);

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
        const Step insertStep{Step::type_if};
        auto iter = seq.insert(seq.begin()+1, insertStep);

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

        REQUIRE(not seq.empty());
        REQUIRE(3 == seq.size());
        REQUIRE(Step::type_if == (*iter).get_type());

        Step::Type expected[] = {Step::type_action, Step::type_if, Step::type_action};
        int idx = 0;
        for(auto step : seq)
            REQUIRE(step.get_type() == expected[idx++]);
    }
}

TEST_CASE("Sequence: is_running()", "[Sequence]")
{
    Step step1{ Step::type_action };
    step1.set_script("a = 10");

    Sequence seq{ "test_sequence" };
    seq.push_back(step1);

    REQUIRE(not seq.is_running());

    Context ctx;
    REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);

    REQUIRE(not seq.is_running());

    seq.set_running(true);
    REQUIRE(seq.is_running() == true);

    seq.set_running(false);
    REQUIRE(seq.is_running() == false);
}

TEST_CASE("Sequence: max_size()", "[Sequence]")
{
    REQUIRE(Sequence::max_size() > 60'000);
}

TEST_CASE("Sequence: modify()", "[Sequence]")
{
    Step ori_step{ Step::type_action };
    ori_step.set_label("Label");
    ori_step.set_script("a = 1");
    ori_step.set_time_of_last_execution(Clock::now());

    Sequence seq{ "test_sequence" };
    seq.push_back(Step{ Step::type_if });
    seq.push_back(ori_step);
    seq.push_back(Step{ Step::type_end });

    auto it = seq.begin() + 1;
    REQUIRE(it->get_label() == "Label");
    REQUIRE(it->get_indentation_level() == 1);

    SECTION("Modify label")
    {
        seq.modify(it, [](Step& step) { step.set_label("Test"); });

        // Check sequence
        REQUIRE(seq.get_indentation_error() == "");
        REQUIRE(seq.size() == 3);
        REQUIRE(seq[0].get_indentation_level() == 0);
        REQUIRE(seq[1].get_indentation_level() == 1);
        REQUIRE(seq[2].get_indentation_level() == 0);

        // Check modified step
        REQUIRE(it->get_label() == "Test");
        REQUIRE(it->get_script() == ori_step.get_script());
        REQUIRE(it->get_time_of_last_execution() == ori_step.get_time_of_last_execution());
    }

    SECTION("Modify step type")
    {
        seq.modify(it, [](Step& step) { step.set_type(Step::type_try); });
        REQUIRE(seq.get_indentation_error() != "");
    }

    SECTION("Modify indentation")
    {
        seq.modify(it, [](Step& step) { step.set_indentation_level(12); });
        REQUIRE(seq.get_indentation_error() == ""); // indentation is automatically corrected
        REQUIRE(seq[0].get_indentation_level() == 0);
        REQUIRE(seq[1].get_indentation_level() == 1);
        REQUIRE(seq[2].get_indentation_level() == 0);
    }

    SECTION("Throwing modification")
    {
        REQUIRE_THROWS_AS(
            seq.modify(it,
                [](Step& step)
                {
                    step.set_label("modified");
                    step.set_indentation_level(12);
                    throw Error("Test");
                }),
            Error);

        REQUIRE(seq.get_indentation_error() == ""); // Sequence class invariants are respected
        REQUIRE(seq[1].get_indentation_level() == 1); // ... e.g. by reindenting
        REQUIRE(seq[1].get_label() == "modified"); // ... but the step may have been modified.
    }
}

TEST_CASE("Sequence: pop_back()", "[Sequence]")
{
    Sequence seq{ "test_sequence" };
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

TEST_CASE("Sequence: push_back()", "[Sequence]")
{
    Sequence seq{ "test_sequence" };

    SECTION("append lvalue Step")
    {
        Step s1{};
        seq.push_back(s1);
        REQUIRE(seq.empty() == false);
        REQUIRE(seq.size() == 1);

        Step s2{};
        seq.push_back(s2);
        REQUIRE(seq.size() == 2);
    }

    SECTION("append rvalue Step")
    {
        seq.push_back(Step{});
        REQUIRE(seq.empty() == false);
        REQUIRE(seq.size() == 1);

        seq.push_back(Step{});
        REQUIRE(seq.empty() == false);
        REQUIRE(seq.size() == 2);
    }
}

TEST_CASE("Sequence: set_error()", "[Sequence]")
{
    Sequence seq{ "test_sequence" };

    seq.set_error(Error{ "Test", 42 });
    REQUIRE(seq.get_error().has_value() == true);
    REQUIRE(seq.get_error()->what() == "Test"s);
    REQUIRE(seq.get_error()->get_index().has_value() == true);
    REQUIRE(seq.get_error()->get_index().value() == 42);

    seq.set_error(gul14::nullopt);
    REQUIRE(seq.get_error().has_value() == false);
}

TEST_CASE("Sequence: set_running()", "[Sequence]")
{
    Step step1{ Step::type_action };
    step1.set_label("Long running step");
    step1.set_script("sleep(0.5)");

    Step step2{ Step::type_if };
    step2.set_label("conditional");
    step2.set_script("return true");

    Sequence seq{ "test_sequence" };
    seq.push_back(step1);

    seq.set_running(true);
    REQUIRE(seq.is_running() == true);

    SECTION("Sequence can be modified while not is_running()")
    {
        seq.set_running(false);
        REQUIRE(seq.is_running() == false);
        seq.pop_back();
        REQUIRE(seq.empty());
    }

    SECTION("Sequence cannot be modified while is_running()")
    {
        REQUIRE_THROWS_AS(seq.modify(seq.begin(),
            [](Step& step) { step.set_label("New label"); }), Error);
        REQUIRE(seq[0].get_label() == "Long running step");

        REQUIRE_THROWS_AS(seq.assign(seq.begin(), Step{ Step::type_while }), Error);
        REQUIRE(seq[0].get_type() == Step::type_action);

        REQUIRE_THROWS_AS(seq.assign(seq.begin(), step2), Error);
        REQUIRE(seq[0].get_type() == Step::type_action);

        REQUIRE_THROWS_AS(seq.insert(seq.begin(), Step{ Step::type_action }), Error);
        REQUIRE(seq.size() == 1);

        REQUIRE_THROWS_AS(seq.insert(seq.begin(), step2), Error);
        REQUIRE(seq.size() == 1);

        REQUIRE_THROWS_AS(seq.erase(seq.begin()), Error);
        REQUIRE(seq.size() == 1);

        REQUIRE_THROWS_AS(seq.erase(seq.begin(), seq.end()), Error);
        REQUIRE(seq.size() == 1);

        REQUIRE_THROWS_AS(seq.push_back(Step{Step::type_action}), Error);
        REQUIRE(seq.size() == 1);

        REQUIRE_THROWS_AS(seq.push_back(step2), Error);
        REQUIRE(seq.size() == 1);

        REQUIRE_THROWS_AS(seq.pop_back(), Error);
        REQUIRE(seq.size() == 1);

        REQUIRE_THROWS_AS(seq.set_step_setup_script("b = 1"), Error);
        REQUIRE(seq.get_step_setup_script() == "");
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
    REQUIRE_NOTHROW(sequence.check_syntax());
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

TEST_CASE("Sequence: check if-elseif-try-catch-end-elseif-end w/ empty catch block",
          "[Sequence]")
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
    REQUIRE_NOTHROW(sequence.check_syntax());
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

TEST_CASE("Sequence: if-elseif-while-end-else-end with empty while loop", "[Sequence]")
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

    REQUIRE_NOTHROW(sequence.check_syntax());
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

TEST_CASE("execute(): empty sequence", "[Sequence]")
{
    Context context;
    Sequence sequence("Empty sequence");

    REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    REQUIRE(sequence.get_error().has_value() == false);
}

TEST_CASE("execute(): simple sequence", "[Sequence]")
{
    Context context;
    Step step;

    Sequence sequence("Simple sequence");
    sequence.push_back(step);

    REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    REQUIRE(sequence.get_error().has_value() == false);
}

TEST_CASE("execute(): Simple sequence with unchanged context", "[Sequence]")
{
    Context context;
    context.variables["a"] = VarInteger{ 0 };

    Step step;
    step.set_used_context_variable_names(VariableNames{"a"});

    Sequence sequence("Simple sequence");
    sequence.push_back(step);

    REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    REQUIRE(sequence.get_error().has_value() == false);
    REQUIRE(std::get<VarInteger>(context.variables["a"]) == 0);
}

TEST_CASE("execute(): Simple sequence with more context variables", "[Sequence]")
{
    Context ctx;
    ctx.variables["a"] = VarInteger{ 0 };
    Sequence seq("Simple sequence");

    SECTION("Dont manipulate context variable") {
        Step s1;
        s1.set_script("a = 2");

        seq.push_back(s1);
        REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);
        REQUIRE(std::get<VarInteger>(ctx.variables["a"]) == 0);
    }
    SECTION("Manipulate context variable") {
        Step s1;
        s1.set_script("a = 2");
        s1.set_used_context_variable_names(VariableNames{"a"});

        seq.push_back(s1);
        REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);
        REQUIRE(std::get<VarInteger>(ctx.variables["a"]) == 2);
    }
    SECTION("Hand context variable over (not)") {
        Step s1;
        s1.set_script("a = 2");
        s1.set_used_context_variable_names(VariableNames{"a"});
        Step s2;
        s2.set_script("if a then a = a + 2 end");

        seq.push_back(s1);
        seq.push_back(s2);
        REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);
        REQUIRE(std::get<VarInteger>(ctx.variables["a"]) == 2);
    }
    SECTION("Hand context variable over") {
        Step s1;
        s1.set_script("a = 2");
        s1.set_used_context_variable_names(VariableNames{"a"});
        Step s2;
        s2.set_script("if a then a = a + 2 end");
        s2.set_used_context_variable_names(VariableNames{"a"});

        seq.push_back(s1);
        seq.push_back(s2);
        REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);
        REQUIRE(std::get<VarInteger>(ctx.variables["a"]) == 4);
    }
    SECTION("Hand variable over context without initial value") {
        Step s1;
        s1.set_script("a = 2; b = 3"); // semicolon for the C people, Lua ignores it
        s1.set_used_context_variable_names(VariableNames{"a", "b"});
        Step s2;
        s2.set_script("if b then a = a + 2 end");
        s2.set_used_context_variable_names(VariableNames{"a", "b"});

        seq.push_back(s1);
        seq.push_back(s2);
        REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);
        REQUIRE(std::get<VarInteger>(ctx.variables["a"]) == 4);
    }
    SECTION("Hand bool variable over context without initial value") {
        Step s1;
        s1.set_script("a = 2; b = true"); // semicolon for the C people, Lua ignores it
        s1.set_used_context_variable_names(VariableNames{"a", "b"});
        Step s2;
        s2.set_script("if b then a = a + 2 end");
        s2.set_used_context_variable_names(VariableNames{"a", "b"});

        seq.push_back(s1);
        seq.push_back(s2);
        REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);
        CAPTURE(std::get<VarInteger>(ctx.variables["a"]));
        REQUIRE(ctx.variables["a"] == VariableValue{ VarInteger{ 4 }}); // Another variant to check variables
    }
    SECTION("Hand nil variable over context without initial value") {
        Step s1;
        s1.set_script("a = 2; b = true"); // semicolon for the C people, Lua ignores it
        s1.set_used_context_variable_names(VariableNames{"a", "b"});
        Step s2;
        s2.set_script("if b then a = a + 2 end; b = nil");
        s2.set_used_context_variable_names(VariableNames{"a", "b"});
        Step s3;
        s3.set_script("if b then a = a + 2 end");
        s3.set_used_context_variable_names(VariableNames{"a", "b"});

        seq.push_back(s1);
        seq.push_back(s2);
        seq.push_back(s3);
        REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);
        CAPTURE(std::get<VarInteger>(ctx.variables["a"]));
        REQUIRE(std::get<VarInteger>(ctx.variables["a"]) == 4);
        REQUIRE_THROWS(ctx.variables.at("b")); // nil means not-existing
    }
    SECTION("Check nil is really non-existing") {
        Step s1;
        s1.set_script("b = 0"); // create variable b
        s1.set_used_context_variable_names(VariableNames{"a", "b"});
        Step s2;
        // In Lua _G is the global table, that contains all 'root' variables
        // (and functions, as all functions are just variables with closures).
        // The following explicitly checks if variable `b` is really present/absent
        // in/of the global table.
        // This check is of course superstitious, Lua should handle it that way anyhow.
        // It shows nicely the analogy of our ctx.variables table and Lua's _G table.
        s2.set_script("for k, _ in pairs(_G) do if k == 'b' then a = a + 1 end end");
        s2.set_used_context_variable_names(VariableNames{"a", "b"});
        Step s3;
        s3.set_script("b = nil"); // remove variable b
        s3.set_used_context_variable_names(VariableNames{"a", "b"});
        Step s4;
        s4.set_script("for k, _ in pairs(_G) do if k == 'b' then a = a + 1 end end");
        s4.set_used_context_variable_names(VariableNames{"a", "b"});
        seq.push_back(s1);
        seq.push_back(s2);
        seq.push_back(s3);
        seq.push_back(s4);
        REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);
        CAPTURE(std::get<VarInteger>(ctx.variables["a"]));
        REQUIRE(std::get<VarInteger>(ctx.variables["a"]) == 1);
        REQUIRE_THROWS(ctx.variables.at("b")); // nil means not-existing
    }
}

TEST_CASE("execute(): complex sequence with prohibited Lua function", "[Sequence]")
{
    Context context;

    Step step;
    step.set_label("Action");
    step.set_type(Step::type_action);
    step.set_script("require 'io'"); // provokes Error exception

    Sequence sequence("Complex sequence");
    sequence.push_back(step);

    auto maybe_error = sequence.execute(context, nullptr);
    REQUIRE(maybe_error.has_value() == true);
    REQUIRE(maybe_error->what() != ""s);
    REQUIRE(maybe_error->get_index().has_value() == true);
    REQUIRE(maybe_error->get_index().value() == 0);

    REQUIRE(sequence.get_error() == maybe_error);
}

TEST_CASE("execute(): complex sequence with disallowed 'function' and context "
          "change", "[Sequence]")
{
    Context context;
    context.variables["a"] = VarInteger{ 1 };

    Step step_action1;
    step_action1.set_label("Action");
    step_action1.set_type(Step::type_action);
    step_action1.set_script("require 'io'"); // provokes Error exception

    Step step_action2;
    step_action2.set_label("Action");
    step_action2.set_type(Step::type_action);
    step_action2.set_script("a=a+1");

    Sequence sequence("Complex sequence");
    sequence.push_back(step_action1);
    sequence.push_back(step_action2);

    auto maybe_error = sequence.execute(context, nullptr);
    REQUIRE(maybe_error.has_value() == true);
    REQUIRE(maybe_error->what() != ""s);
    REQUIRE(maybe_error->get_index().has_value() == true);
    REQUIRE(maybe_error->get_index().value() == 0);

    REQUIRE(sequence.get_error() == maybe_error);

    REQUIRE(std::get<VarInteger>(context.variables["a"]) == 1);
}

TEST_CASE("execute(): complex sequence with context change", "[Sequence]")
{
    Context context;
    context.variables["a"] = VarInteger{ 1 };

    Step step_action1;
    step_action1.set_label("Action1");
    step_action1.set_type(Step::type_action);
    step_action1.set_script("a=a+1");
    step_action1.set_used_context_variable_names(VariableNames{"a"});

    Sequence sequence("Complex sequence");
    sequence.push_back(step_action1);

    REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    REQUIRE(sequence.get_error().has_value() == false);
    REQUIRE(std::get<VarInteger>(context.variables["a"]) == 2);
}

TEST_CASE("execute(): if-else sequence", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_if);
    sequence.push_back(step_action_if);
    sequence.push_back(step_else);
    sequence.push_back(step_action_else);
    sequence.push_back(step_if_end);

    sequence.check_syntax();

    SECTION("if-else sequence with if=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 1 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 2);
    }

    SECTION("if-else sequence with if=false")
    {
        Context context;
        context.variables["a"] = VarInteger{ 2 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 3);
    }
}

TEST_CASE("execute(): if-elseif sequence", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_if);
    sequence.push_back(step_if_action);
    sequence.push_back(step_elseif);
    sequence.push_back(step_elseif_action);
    sequence.push_back(step_if_end);

    SECTION("if-elseif sequence with if=elseif=false")
    {
        Context context;
        context.variables["a"] = VarInteger{ 0 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 0);
    }

    SECTION("if-elseif sequence with if=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 1 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 2);
    }

    SECTION("if-elseif sequence with elseif=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 2 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 3);
    }
}

TEST_CASE("execute(): if-elseif-else sequence", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
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
        context.variables["a"] = VarInteger{ 1 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 2);
    }

    SECTION("if-elseif-else sequence with elseif=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 2 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 3);
    }

    SECTION("if-elseif-else sequence with else=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 3 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 4);
    }
}

TEST_CASE("execute(): if-elseif-elseif sequence", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
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
        context.variables["a"] = VarInteger{ 0 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 0);
    }

    SECTION("if-elseif-elseif sequence with if=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 1 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 2);
    }

    SECTION("if-elseif-elseif sequence with elseif[1]=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 2 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 3);
    }

    SECTION("if-elseif-elseif sequence with elseif[2]=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 3 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 4);
    }
}

TEST_CASE("execute(): if-elseif-elseif-else sequence", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
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
        context.variables["a"] = VarInteger{ 0 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 5);
    }

    SECTION("if-elseif-elseif sequence with if=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 1 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 2);
    }

    SECTION("if-elseif-elseif sequence with elseif[1]=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 2 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 3);
    }

    SECTION("if-elseif-elseif sequence with elseif[2]=true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 3 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 4);
    }
}

TEST_CASE("execute(): if-elseif-else-end sequence with empty blocks", "[Sequence]")
{
    /*
    pre-condition:
    a = 0/1/2/3

    script:
    0: if a == 1 then
    1: elseif a == 2 then
    2: elseif a == 3 then
    3: else
    4: end

    post-condition:
    a = 5/2/3/4

    */
    Step step_if        {Step::type_if};
    Step step_elseif1   {Step::type_elseif};
    Step step_elseif2   {Step::type_elseif};
    Step step_else      {Step::type_else};
    Step step_end       {Step::type_end};

    step_if.set_label("if a == 1");
    step_if.set_used_context_variable_names(VariableNames{"a"});
    step_if.set_script("return a == 1");

    step_elseif1.set_label("elseif1");
    step_elseif1.set_used_context_variable_names(VariableNames{"a"});
    step_elseif1.set_script("return a == 2");

    step_elseif2.set_label("elseif2");
    step_elseif2.set_used_context_variable_names(VariableNames{"a"});
    step_elseif2.set_script("return a == 3");

    step_else.set_label("else");
    step_else.set_used_context_variable_names(VariableNames{"a"});

    step_end.set_label("if: end");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_if);
    sequence.push_back(step_elseif1);
    sequence.push_back(step_elseif2);
    sequence.push_back(step_else);
    sequence.push_back(step_end);

    SECTION("IF condition true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 1 };
        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    }

    SECTION("First ELSEIF condition true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 2 };
        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    }

    SECTION("Second ELSEIF condition true")
    {
        Context context;
        context.variables["a"] = VarInteger{ 3 };
        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    }

    SECTION("All conditions false, use ELSE branch")
    {
        Context context;
        context.variables["a"] = VarInteger{ 4 };
        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    }
}

TEST_CASE("execute(): faulty if-else-elseif sequence", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_if);
    sequence.push_back(step_if_action);
    sequence.push_back(step_if_else);
    sequence.push_back(step_if_else_action);
    sequence.push_back(step_if_elseif);
    sequence.push_back(step_if_elseif_action);
    sequence.push_back(step_if_end);

    Context context;
    context.variables["a"] = VarInteger{ 0 };

    REQUIRE(sequence.get_error().has_value() == false);

    auto maybe_error = sequence.execute(context, nullptr);

    REQUIRE(maybe_error.has_value() == true);
    REQUIRE(maybe_error->what() != ""s);
    REQUIRE(maybe_error->get_index().has_value() == false);

    REQUIRE(sequence.get_error() == maybe_error);
}

TEST_CASE("execute(): while sequence", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_while);
    sequence.push_back(step_while_action);
    sequence.push_back(step_while_end);

    SECTION("while sequence with while: a<10")
    {
        Context context;
        context.variables["a"] = VarInteger{ 0 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 10);
    }
}

TEST_CASE("execute(): empty while sequence", "[Sequence]")
{
    /*
    pre-condition:
    a = 0

    script:
    1: while ( a = a + 1; a < 10 )
    3: end

    post-condition:
    a = 10

    */
    Step step_while {Step::type_while};
    Step step_end   {Step::type_end};

    step_while.set_label("while (a = a + 1; a < 10)");
    step_while.set_used_context_variable_names(VariableNames{"a"});
    step_while.set_script("a = a + 1; return a < 10");

    step_end.set_label("while: end");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_while);
    sequence.push_back(step_end);

    SECTION("while sequence with while: a<10")
    {
        Context context;
        context.variables["a"] = VarInteger{ 0 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 10);
    }
}

TEST_CASE("execute(): try sequence with success", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_try);
    sequence.push_back(step_try_action);
    sequence.push_back(step_try_catch);
    sequence.push_back(step_try_catch_action);
    sequence.push_back(step_try_end);

    Context context;
    context.variables["a"] = VarInteger{ 0 };

    REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    REQUIRE(std::get<VarInteger>(context.variables["a"]) == 1);
}

TEST_CASE("execute(): try sequence with fault", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_try);
    sequence.push_back(step_try_action1);
    sequence.push_back(step_try_action2);
    sequence.push_back(step_try_catch);
    sequence.push_back(step_try_catch_action);
    sequence.push_back(step_try_end);

    Context context;
    context.variables["a"] = VarInteger{ 0 };

    REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    REQUIRE(std::get<VarInteger>(context.variables["a"]) == 2);
}

TEST_CASE("execute(): complex try sequence with nested fault condition",
          "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
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
    context.variables["a"] = VarInteger{ 0 };

    REQUIRE_NOTHROW(sequence.execute(context, nullptr));
    REQUIRE(std::get<VarInteger>(context.variables["a"]) == 3);
}

TEST_CASE("execute(): simple try sequence with fault", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_00);
    sequence.push_back(step_01);
    sequence.push_back(step_02);
    sequence.push_back(step_03);
    sequence.push_back(step_04);
    sequence.push_back(step_05);
    sequence.push_back(step_06);

    Context context;
    context.variables["a"] = VarInteger{ 0 };

    auto maybe_error = sequence.execute(context, nullptr);
    REQUIRE(maybe_error.has_value() == true);
    REQUIRE(maybe_error->what() != ""s);
    REQUIRE(maybe_error->get_index().has_value() == true);
    REQUIRE(maybe_error->get_index().value() == 5);
    REQUIRE(sequence.get_error() == maybe_error);
    REQUIRE(std::get<VarInteger>(context.variables["a"]) == 2);
}

TEST_CASE("execute(): complex try sequence with fault", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
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
    auto maybe_error = sequence.execute(context, nullptr);
    REQUIRE(maybe_error.has_value() == true);

    REQUIRE(std::get<VarInteger>(context.variables["a"]) == 2);
}

TEST_CASE("execute(): complex sequence", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
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
        context.variables["a"] = VarInteger{ 0 };
        context.variables["b"] = VarInteger{ 0 };
        context.variables["c"] = VarInteger{ 0 };
        context.variables["d"] = VarInteger{ 0 };
        context.variables["e"] = VarInteger{ 1 };
        context.variables["f"] = VarInteger{ 1 };
        context.variables["g"] = VarInteger{ 1 };
        REQUIRE_NOTHROW(sequence.execute(context, nullptr));

        // a=10, b=1, c=1, d=0/2/3, e=1/3/4, f=2, g=1/2
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 10);
        REQUIRE(std::get<VarInteger>(context.variables["b"]) == 1);
        REQUIRE(std::get<VarInteger>(context.variables["c"]) == 1);
        REQUIRE(std::get<VarInteger>(context.variables["d"]) == 0);
        REQUIRE(std::get<VarInteger>(context.variables["e"]) == 1); // not touched
        REQUIRE(std::get<VarInteger>(context.variables["f"]) == 1); // not touched
        REQUIRE(std::get<VarInteger>(context.variables["g"]) == 1); // not touched
    }

    SECTION("complex sequence: a: 0->10, b: 0->1, c: 0->1, d: 1->3, e: 1->2, f: 1->1, "
            "g: 1->1")
    {
        Context context;
        context.variables["a"] = VarInteger{ 0 };
        context.variables["b"] = VarInteger{ 0 };
        context.variables["c"] = VarInteger{ 0 };
        context.variables["d"] = VarInteger{ 1 };
        context.variables["e"] = VarInteger{ 1 };
        context.variables["f"] = VarInteger{ 1 };
        context.variables["g"] = VarInteger{ 1 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 10);
        REQUIRE(std::get<VarInteger>(context.variables["b"]) == 1);
        REQUIRE(std::get<VarInteger>(context.variables["c"]) == 1);
        REQUIRE(std::get<VarInteger>(context.variables["d"]) == 3);
        REQUIRE(std::get<VarInteger>(context.variables["e"]) == 2);
        REQUIRE(std::get<VarInteger>(context.variables["f"]) == 1); // not touched
        REQUIRE(std::get<VarInteger>(context.variables["g"]) == 1); // not touched
    }

    SECTION("complex sequence: a: 0->10, b: 0->1, c: 0->1, d: 2->3, e: 5->5, f: 2->2, "
            "g: 3->3")
    {
        Context context;
        context.variables["a"] = VarInteger{ 0 };
        context.variables["b"] = VarInteger{ 0 };
        context.variables["c"] = VarInteger{ 1 };
        context.variables["d"] = VarInteger{ 2 };
        context.variables["e"] = VarInteger{ 5 };
        context.variables["f"] = VarInteger{ 3 };
        context.variables["g"] = VarInteger{ 2 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 10);
        REQUIRE(std::get<VarInteger>(context.variables["b"]) == 1);
        REQUIRE(std::get<VarInteger>(context.variables["c"]) == 1);
        REQUIRE(std::get<VarInteger>(context.variables["d"]) == 3);
        REQUIRE(std::get<VarInteger>(context.variables["e"]) == 5); // not touched
        REQUIRE(std::get<VarInteger>(context.variables["f"]) == 3); // not touched
        REQUIRE(std::get<VarInteger>(context.variables["g"]) == 2); // not touched
    }

    SECTION("complex sequence: a: 0->10, b: 0->1, c: 0->1, d: 1->3, e: 2->4, f: 3->2, "
            "g: 4->2")
    {
        Context context;
        context.variables["a"] = VarInteger{ 0 };
        context.variables["b"] = VarInteger{ 0 };
        context.variables["c"] = VarInteger{ 1 };
        context.variables["d"] = VarInteger{ 1 };
        context.variables["e"] = VarInteger{ 2 };
        context.variables["f"] = VarInteger{ 3 };
        context.variables["g"] = VarInteger{ 4 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 10);
        REQUIRE(std::get<VarInteger>(context.variables["b"]) == 1);
        REQUIRE(std::get<VarInteger>(context.variables["c"]) == 1);
        REQUIRE(std::get<VarInteger>(context.variables["d"]) == 3);
        REQUIRE(std::get<VarInteger>(context.variables["e"]) == 3);
        REQUIRE(std::get<VarInteger>(context.variables["f"]) == 3); // not touched
        REQUIRE(std::get<VarInteger>(context.variables["g"]) == 4); // not touched
    }
}

TEST_CASE("execute(): complex sequence with misplaced if", "[Sequence]")
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

    Sequence sequence{ "test_sequence" };
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

    auto maybe_error = sequence.execute(context, nullptr);
    REQUIRE(maybe_error.has_value() == true);
}

TEST_CASE("execute_sequence(): Messages", "[execute_sequence]")
{
    const auto t0 = Clock::now();
    CommChannel comm;
    auto& queue = comm.queue_;

    Context context;
    context.variables["a"] = VarInteger{ 0 };

    Step step1{ Step::type_action };
    step1.set_used_context_variable_names(VariableNames{ "a" });
    step1.set_script("a = a + 1");

    Step step2{ Step::type_action };
    step2.set_used_context_variable_names(VariableNames{ "a" });
    step2.set_script("a = a + 2");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(std::move(step1));
    sequence.push_back(std::move(step2));

    REQUIRE(sequence.execute(context, &comm) == gul14::nullopt);

    // 2 sequence messages plus 4 step start/stop messages
    REQUIRE(queue.size() == 6);

    // First, a "sequence started" message
    auto msg = queue.pop();
    REQUIRE(msg.get_type() == Message::Type::sequence_started);
    REQUIRE(msg.get_timestamp() >= t0);
    REQUIRE(msg.get_timestamp() - t0 < 1s);

    // StepStartedMessage
    msg = queue.pop();
    REQUIRE(msg.get_type() == Message::Type::step_started);
    REQUIRE(msg.get_timestamp() >= t0);
    REQUIRE(msg.get_timestamp() - t0 < 1s);

    // StepStoppedMessage
    msg = queue.pop();
    REQUIRE(msg.get_type() == Message::Type::step_stopped);
    REQUIRE(msg.get_timestamp() >= t0);
    REQUIRE(msg.get_timestamp() - t0 < 1s);

    // StepStartedMessage
    msg = queue.pop();
    REQUIRE(msg.get_type() == Message::Type::step_started);
    REQUIRE(msg.get_timestamp() >= t0);
    REQUIRE(msg.get_timestamp() - t0 < 1s);

    // StepStoppedMessage
    msg = queue.pop();
    REQUIRE(msg.get_type() == Message::Type::step_stopped);
    REQUIRE(msg.get_timestamp() >= t0);
    REQUIRE(msg.get_timestamp() - t0 < 1s);

    // SequenceStoppedMessage
    msg = queue.pop();
    REQUIRE(msg.get_type() == Message::Type::sequence_stopped);
    REQUIRE(msg.get_timestamp() >= t0);
    REQUIRE(msg.get_timestamp() - t0 < 1s);
}

TEST_CASE("execute_sequence(): Message callbacks", "[execute_sequence]")
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

    SECTION("Sequence ending successfully")
    {
        Step step1{ Step::type_action };
        step1.set_script("print('American River')");

        Step step2{ Step::type_action };
        step2.set_script("a = 2");

        sequence.push_back(std::move(step1));
        sequence.push_back(std::move(step2));

        REQUIRE(sequence.execute(context, nullptr) == gul14::nullopt);

        REQUIRE(str ==
            "[SEQ_START]"
                "[STEP_START][OUTPUT(American River\n)][STEP_STOP]"
                "[STEP_START][STEP_STOP]"
            "[SEQ_STOP]");
    }

    SECTION("Sequence ending with error")
    {
        Step step1{ Step::type_action };
        step1.set_script("print('Coyote Creek')");

        Step step2{ Step::type_action };
        step2.set_script("boom()");

        sequence.push_back(std::move(step1));
        sequence.push_back(std::move(step2));

        REQUIRE(sequence.execute(context, nullptr) != gul14::nullopt);

        REQUIRE(str ==
            "[SEQ_START]"
                "[STEP_START][OUTPUT(Coyote Creek\n)][STEP_STOP]"
                "[STEP_START][STEP_STOP_ERR]"
            "[SEQ_STOP_ERR]");
    }
}

TEST_CASE("execute(): if-elseif-else sequence with disable", "[Sequence]")
{
    /*
    script:
    0: a = 1
    1: if a == 1 then
    2:     a = 2
    3: elseif a == 2 then
    4:     a = 3
    5: else
    6:     a = 4
    7: end
    8: b = 1
    */
    Step step_pre            {Step::type_action};
    Step step_if             {Step::type_if};
    Step step_if_action      {Step::type_action};
    Step step_elseif         {Step::type_elseif};
    Step step_elseif_action  {Step::type_action};
    Step step_else           {Step::type_else};
    Step step_else_action    {Step::type_action};
    Step step_if_end         {Step::type_end};
    Step step_post           {Step::type_action};

    step_pre.set_label("a = 1");
    step_pre.set_used_context_variable_names(VariableNames{"a"});
    step_pre.set_script("a = 1");

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
    step_else.set_script("never executed");

    step_else_action.set_label("else: set a to 4");
    step_else_action.set_used_context_variable_names(VariableNames{"a"});
    step_else_action.set_script("a = 4");

    step_if_end.set_label("if: end");

    step_post.set_label("b = 1");
    step_post.set_used_context_variable_names(VariableNames{"a", "b"});
    step_post.set_script("b = 1");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_pre); // a = 1
    sequence.push_back(step_if); // IF a == 1
    sequence.push_back(step_if_action); // a = 2
    sequence.push_back(step_elseif); // ELSEIF a == 2
    sequence.push_back(step_elseif_action); // a = 3
    sequence.push_back(step_else); // ELSE
    sequence.push_back(step_else_action); // a = 4
    sequence.push_back(step_if_end); // END
    sequence.push_back(step_post); // b = 1

    SECTION("All steps enabled")
    {
        Context context;
        context.variables["a"] = VarInteger{ 5 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 2);
        REQUIRE(std::get<VarInteger>(context.variables["b"]) == 1);
    }

    sequence.modify(sequence.begin() + 1, [](Step& s) {
        s.set_disabled(true);
    });


    SECTION("Second step disabled")
    {
        Context context;
        context.variables["a"] = VarInteger{ 5 };

        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 1);
        REQUIRE(std::get<VarInteger>(context.variables["b"]) == 1);
    }
}

TEST_CASE("execute(): sequence with multiple disabled", "[Sequence]")
{
    Step step1{ Step::type_action };
    step1.set_label("a = 1");
    step1.set_used_context_variable_names(VariableNames{"a"});
    step1.set_script("a = 1");

    Step step2{ Step::type_action };
    step2.set_label("a++");
    step2.set_used_context_variable_names(VariableNames{"a"});
    step2.set_script("a = a + 1");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step1);
    sequence.push_back(step2);
    sequence.push_back(step2);
    sequence.push_back(step2);
    sequence.push_back(step2);
    sequence.push_back(step2);

    SECTION("All steps enabled")
    {
        Context context;
        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 6);
    }

    sequence.modify(sequence.begin() + 1, [](Step& s) {
        s.set_disabled(true);
    });

    SECTION("One step disabled")
    {
        Context context;
        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 5);
    }

    sequence.modify(sequence.begin() + 2, [](Step& s) {
        s.set_disabled(true);
    });

    SECTION("Two steps disabled")
    {
        Context context;
        REQUIRE_NOTHROW(sequence.execute(context, nullptr));
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == 4);
    }
}

auto copy_and_disable(Step x) -> Step { return x.set_disabled(true), x; }

TEST_CASE("execute(): disable 'invariant' (direct)", "[Sequence]")
{
    /*
    script:
    0: a = 1
    1: if a == 1 then
    2:     a = 2
    3: elseif a == 2 then
    4:     a = 3
    5: else
    6:     a = 4
    7: end
    8: b = 1
    */
    Step step_pre            {Step::type_action};
    Step step_if             {Step::type_if};
    Step step_if_action      {Step::type_action};
    Step step_elseif         {Step::type_elseif};
    Step step_elseif_action  {Step::type_action};
    Step step_else           {Step::type_else};
    Step step_else_action    {Step::type_action};
    Step step_if_end         {Step::type_end};
    Step step_post           {Step::type_action};

    step_pre.set_label("a = 1");
    step_pre.set_used_context_variable_names(VariableNames{"a"});
    step_pre.set_script("a = 1");

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
    step_else.set_script("never executed");

    step_else_action.set_label("else: set a to 4");
    step_else_action.set_used_context_variable_names(VariableNames{"a"});
    step_else_action.set_script("a = 4");

    step_if_end.set_label("if: end");

    step_post.set_label("b = 1");
    step_post.set_used_context_variable_names(VariableNames{"a", "b"});
    step_post.set_script("b = 1");

    SECTION("Second step disabled")
    {
        Sequence sequence{ "test_sequence" };
        sequence.push_back(step_pre); // a = 1
        sequence.push_back(copy_and_disable(step_if)); // IF a == 1
        sequence.push_back(step_if_action); // a = 2
        sequence.push_back(step_elseif); // ELSEIF a == 2
        sequence.push_back(step_elseif_action); // a = 3
        sequence.push_back(step_else); // ELSE
        sequence.push_back(step_else_action); // a = 4
        sequence.push_back(step_if_end); // END
        sequence.push_back(step_post); // b = 1

        Context context;
        context.variables["a"] = VarInteger{ 5 };

        REQUIRE(sequence[0].is_disabled() == false);
        REQUIRE(sequence[1].is_disabled() == true);
        REQUIRE(sequence[2].is_disabled() == true);
        REQUIRE(sequence[3].is_disabled() == true);
        REQUIRE(sequence[4].is_disabled() == true);
        REQUIRE(sequence[5].is_disabled() == true);
        REQUIRE(sequence[6].is_disabled() == true);
        REQUIRE(sequence[7].is_disabled() == true);
        REQUIRE(sequence[8].is_disabled() == false);
    }

    SECTION("Third step disabled")
    {
        Sequence sequence{ "test_sequence" };
        sequence.push_back(step_pre); // a = 1
        sequence.push_back(step_if); // IF a == 1
        sequence.push_back(copy_and_disable(step_if_action)); // a = 2
        sequence.push_back(step_elseif); // ELSEIF a == 2
        sequence.push_back(step_elseif_action); // a = 3
        sequence.push_back(step_else); // ELSE
        sequence.push_back(step_else_action); // a = 4
        sequence.push_back(step_if_end); // END
        sequence.push_back(step_post); // b = 1

        Context context;
        context.variables["a"] = VarInteger{ 5 };

        REQUIRE(sequence[0].is_disabled() == false);
        REQUIRE(sequence[1].is_disabled() == false);
        REQUIRE(sequence[2].is_disabled() == true);
        REQUIRE(sequence[3].is_disabled() == false);
        REQUIRE(sequence[4].is_disabled() == false);
        REQUIRE(sequence[5].is_disabled() == false);
        REQUIRE(sequence[6].is_disabled() == false);
        REQUIRE(sequence[7].is_disabled() == false);
        REQUIRE(sequence[8].is_disabled() == false);
    }

    SECTION("Fourth step disabled")
    {
        Sequence sequence{ "test_sequence" };
        sequence.push_back(step_pre); // a = 1
        sequence.push_back(step_if); // IF a == 1
        sequence.push_back(step_if_action); // a = 2
        sequence.push_back(copy_and_disable(step_elseif)); // ELSEIF a == 2
        sequence.push_back(step_elseif_action); // a = 3
        sequence.push_back(step_else); // ELSE
        sequence.push_back(step_else_action); // a = 4
        sequence.push_back(step_if_end); // END
        sequence.push_back(step_post); // b = 1

        Context context;
        context.variables["a"] = VarInteger{ 5 };

        REQUIRE(sequence[0].is_disabled() == false);
        REQUIRE(sequence[1].is_disabled() == false);
        REQUIRE(sequence[2].is_disabled() == false);
        REQUIRE(sequence[3].is_disabled() == false);
        REQUIRE(sequence[4].is_disabled() == false);
        REQUIRE(sequence[5].is_disabled() == false);
        REQUIRE(sequence[6].is_disabled() == false);
        REQUIRE(sequence[7].is_disabled() == false);
        REQUIRE(sequence[8].is_disabled() == false);
    }
}

TEST_CASE("execute(): disable 'invariant' (afterwards)", "[Sequence]")
{
    /*
    script:
    0: a = 1
    1: if a == 1 then
    2:     a = 2
    3: elseif a == 2 then
    4:     a = 3
    5: else
    6:     a = 4
    7: end
    8: b = 1
    */
    Step step_pre            {Step::type_action};
    Step step_if             {Step::type_if};
    Step step_if_action      {Step::type_action};
    Step step_elseif         {Step::type_elseif};
    Step step_elseif_action  {Step::type_action};
    Step step_else           {Step::type_else};
    Step step_else_action    {Step::type_action};
    Step step_if_end         {Step::type_end};
    Step step_post           {Step::type_action};

    step_pre.set_label("a = 1");
    step_pre.set_used_context_variable_names(VariableNames{"a"});
    step_pre.set_script("a = 1");

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
    step_else.set_script("never executed");

    step_else_action.set_label("else: set a to 4");
    step_else_action.set_used_context_variable_names(VariableNames{"a"});
    step_else_action.set_script("a = 4");

    step_if_end.set_label("if: end");

    step_post.set_label("b = 1");
    step_post.set_used_context_variable_names(VariableNames{"a", "b"});
    step_post.set_script("b = 1");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_pre); // a = 1
    sequence.push_back(step_if); // IF a == 1
    sequence.push_back(step_if_action); // a = 2
    sequence.push_back(step_elseif); // ELSEIF a == 2
    sequence.push_back(step_elseif_action); // a = 3
    sequence.push_back(step_else); // ELSE
    sequence.push_back(step_else_action); // a = 4
    sequence.push_back(step_if_end); // END
    sequence.push_back(step_post); // b = 1

    Context context;
    context.variables["a"] = VarInteger{ 5 };

    SECTION("Second step disabled")
    {
        sequence.modify(sequence.begin() + 1, [](Step& s) {
            s.set_disabled(true);
        });

        REQUIRE(sequence[0].is_disabled() == false);
        REQUIRE(sequence[1].is_disabled() == true);
        REQUIRE(sequence[2].is_disabled() == true);
        REQUIRE(sequence[3].is_disabled() == true);
        REQUIRE(sequence[4].is_disabled() == true);
        REQUIRE(sequence[5].is_disabled() == true);
        REQUIRE(sequence[6].is_disabled() == true);
        REQUIRE(sequence[7].is_disabled() == true);
        REQUIRE(sequence[8].is_disabled() == false);
    }

    SECTION("Third step disabled")
    {
        sequence.modify(sequence.begin() + 2, [](Step& s) {
            s.set_disabled(true);
        });

        REQUIRE(sequence[0].is_disabled() == false);
        REQUIRE(sequence[1].is_disabled() == false);
        REQUIRE(sequence[2].is_disabled() == true);
        REQUIRE(sequence[3].is_disabled() == false);
        REQUIRE(sequence[4].is_disabled() == false);
        REQUIRE(sequence[5].is_disabled() == false);
        REQUIRE(sequence[6].is_disabled() == false);
        REQUIRE(sequence[7].is_disabled() == false);
        REQUIRE(sequence[8].is_disabled() == false);
    }

    SECTION("Fourth step disabled")
    {
        sequence.modify(sequence.begin() + 3, [](Step& s) {
            s.set_disabled(true);
        });

        REQUIRE(sequence[0].is_disabled() == false);
        REQUIRE(sequence[1].is_disabled() == false);
        REQUIRE(sequence[2].is_disabled() == false);
        REQUIRE(sequence[3].is_disabled() == false);
        REQUIRE(sequence[4].is_disabled() == false);
        REQUIRE(sequence[5].is_disabled() == false);
        REQUIRE(sequence[6].is_disabled() == false);
        REQUIRE(sequence[7].is_disabled() == false);
        REQUIRE(sequence[8].is_disabled() == false);
    }

    SECTION("Second step disabled then enabled")
    {
        sequence.modify(sequence.begin() + 1, [](Step& s) {
            s.set_disabled(true);
        });

        REQUIRE(sequence[0].is_disabled() == false);
        REQUIRE(sequence[1].is_disabled() == true);
        REQUIRE(sequence[2].is_disabled() == true);
        REQUIRE(sequence[3].is_disabled() == true);
        REQUIRE(sequence[4].is_disabled() == true);
        REQUIRE(sequence[5].is_disabled() == true);
        REQUIRE(sequence[6].is_disabled() == true);
        REQUIRE(sequence[7].is_disabled() == true);
        REQUIRE(sequence[8].is_disabled() == false);

        sequence.modify(sequence.begin() + 1, [](Step& s) {
            s.set_disabled(false);
        });

        REQUIRE(sequence[0].is_disabled() == false);
        REQUIRE(sequence[1].is_disabled() == false);
        REQUIRE(sequence[2].is_disabled() == false);
        REQUIRE(sequence[3].is_disabled() == false);
        REQUIRE(sequence[4].is_disabled() == false);
        REQUIRE(sequence[5].is_disabled() == false);
        REQUIRE(sequence[6].is_disabled() == false);
        REQUIRE(sequence[7].is_disabled() == false);
        REQUIRE(sequence[8].is_disabled() == false);
    }
}

TEST_CASE("execute(): disable 'invariant' (complex)", "[Sequence]")
{
    //  0    ACTION
    //  1    IF ...
    //  2        ACTION
    //  3        IF ...
    //  4            ACTION
    //  5            ACTION
    //  6        ELSE
    //  7            ACTION
    //  8        END
    //  9        ACTION
    // 10    END
    // 11    ACTION

    Sequence s{ "test_sequence" };
    s.push_back(Step{ Step::type_action });
    s.push_back(Step{ Step::type_if });
    s.push_back(Step{ Step::type_action });
    s.push_back(Step{ Step::type_if });
    s.push_back(Step{ Step::type_action });
    s.push_back(Step{ Step::type_action });
    s.push_back(Step{ Step::type_else });
    s.push_back(Step{ Step::type_action });
    s.push_back(Step{ Step::type_end });
    s.push_back(Step{ Step::type_action });
    s.push_back(Step{ Step::type_end });
    s.push_back(Step{ Step::type_action });

    REQUIRE(s[0].get_indentation_level() == 0);
    REQUIRE(s[1].get_indentation_level() == 0);
    REQUIRE(s[2].get_indentation_level() == 1);
    REQUIRE(s[3].get_indentation_level() == 1);
    REQUIRE(s[4].get_indentation_level() == 2);
    REQUIRE(s[5].get_indentation_level() == 2);
    REQUIRE(s[6].get_indentation_level() == 1);
    REQUIRE(s[7].get_indentation_level() == 2);
    REQUIRE(s[8].get_indentation_level() == 1);
    REQUIRE(s[9].get_indentation_level() == 1);
    REQUIRE(s[10].get_indentation_level() == 0);
    REQUIRE(s[11].get_indentation_level() == 0);

    SECTION("Disable and Enable 3 (IF...)")
    {
        s.modify(s.begin() + 3, [](Step& st) {
            st.set_disabled(true);
        });
        REQUIRE(s[0].is_disabled() == false);
        REQUIRE(s[1].is_disabled() == false);
        REQUIRE(s[2].is_disabled() == false);
        REQUIRE(s[3].is_disabled() == true);
        REQUIRE(s[4].is_disabled() == true);
        REQUIRE(s[5].is_disabled() == true);
        REQUIRE(s[6].is_disabled() == true);
        REQUIRE(s[7].is_disabled() == true);
        REQUIRE(s[8].is_disabled() == true);
        REQUIRE(s[9].is_disabled() == false);
        REQUIRE(s[10].is_disabled() == false);
        REQUIRE(s[11].is_disabled() == false);

        s.modify(s.begin() + 3, [](Step& st) {
            st.set_disabled(false);
        });
        REQUIRE(s[0].is_disabled() == false);
        REQUIRE(s[1].is_disabled() == false);
        REQUIRE(s[2].is_disabled() == false);
        REQUIRE(s[3].is_disabled() == false);
        REQUIRE(s[4].is_disabled() == false);
        REQUIRE(s[5].is_disabled() == false);
        REQUIRE(s[6].is_disabled() == false);
        REQUIRE(s[7].is_disabled() == false);
        REQUIRE(s[8].is_disabled() == false);
        REQUIRE(s[9].is_disabled() == false);
        REQUIRE(s[10].is_disabled() == false);
        REQUIRE(s[11].is_disabled() == false);
    }

    SECTION("Disable and Enable 3 (IF...) with 9 disabled")
    {
        s.modify(s.begin() + 0, [](Step& st) {
            st.set_disabled(true);
        });
        s.modify(s.begin() + 9, [](Step& st) {
            st.set_disabled(true);
        });
        s.modify(s.begin() + 3, [](Step& st) {
            st.set_disabled(true);
        });
        REQUIRE(s[0].is_disabled() == true);
        REQUIRE(s[1].is_disabled() == false);
        REQUIRE(s[2].is_disabled() == false);
        REQUIRE(s[3].is_disabled() == true);
        REQUIRE(s[4].is_disabled() == true);
        REQUIRE(s[5].is_disabled() == true);
        REQUIRE(s[6].is_disabled() == true);
        REQUIRE(s[7].is_disabled() == true);
        REQUIRE(s[8].is_disabled() == true);
        REQUIRE(s[9].is_disabled() == true);
        REQUIRE(s[10].is_disabled() == false);
        REQUIRE(s[11].is_disabled() == false);

        s.modify(s.begin() + 3, [](Step& st) {
            st.set_disabled(false);
        });
        REQUIRE(s[0].is_disabled() == true);
        REQUIRE(s[1].is_disabled() == false);
        REQUIRE(s[2].is_disabled() == false);
        REQUIRE(s[3].is_disabled() == false);
        REQUIRE(s[4].is_disabled() == false);
        REQUIRE(s[5].is_disabled() == false);
        REQUIRE(s[6].is_disabled() == false);
        REQUIRE(s[7].is_disabled() == false);
        REQUIRE(s[8].is_disabled() == false);
        REQUIRE(s[9].is_disabled() == true);
        REQUIRE(s[10].is_disabled() == false);
        REQUIRE(s[11].is_disabled() == false);
    }
}

TEST_CASE("execute(): Disable + re-enable action inside while loop", "[Sequence]")
{
    //  0 WHILE ...
    //  1   ACTION
    //  2 END

    Sequence s{ "test_sequence" };
    s.push_back(Step{ Step::type_while });
    s.push_back(Step{ Step::type_action });
    s.push_back(Step{ Step::type_end });

    REQUIRE(s[0].get_indentation_level() == 0);
    REQUIRE(s[1].get_indentation_level() == 1);
    REQUIRE(s[2].get_indentation_level() == 0);

    SECTION("Disable WHILE, then attempt to re-enable ACTION")
    {
        s.modify(s.begin(), [](Step& st) { st.set_disabled(true); });
        REQUIRE(s[0].is_disabled() == true);
        REQUIRE(s[1].is_disabled() == true);
        REQUIRE(s[2].is_disabled() == true);

        s.modify(s.begin() + 1, [](Step& st) { st.set_disabled(false); });
        REQUIRE(s[0].is_disabled() == true);
        REQUIRE(s[1].is_disabled() == true);
        REQUIRE(s[2].is_disabled() == true);
    }
}

TEST_CASE("execute(): Single step", "[Sequence]")
{
    // A deliberately invalid sequence for testing single-step execution:
    // END
    // WHILE
    // ACTION

    Step step_end{ Step::type_end };
    step_end.set_used_context_variable_names(VariableNames{ "a" })
            .set_script("a = 1; error('End Boom')");

    Step step_while{ Step::type_while };
    step_while.set_used_context_variable_names(VariableNames{ "a" })
              .set_script("a = 2; return true");

    Step step_action{ Step::type_action };
    step_action.set_used_context_variable_names(VariableNames{ "a" })
               .set_script("a = 3; error('Action Boom')");

    Sequence sequence{ "test_sequence" };
    sequence.push_back(step_end);
    sequence.push_back(step_while);
    sequence.push_back(step_action);

    Context context;
    context.variables["a"] = VarInteger{ 0 };

    SECTION("Index 0 (END step): Script is not executed because of the step type")
    {
        REQUIRE(sequence.execute(context, nullptr, 0) == gul14::nullopt);
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == VarInteger{ 0 });
    }

    SECTION("Index 1 (WHILE step): Script is executed")
    {
        REQUIRE(sequence.execute(context, nullptr, 1) == gul14::nullopt);
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == VarInteger{ 2 });
    }

    SECTION("Index 2 (ACTION step): Script is executed")
    {
        auto maybe_error = sequence.execute(context, nullptr, 2);
        REQUIRE(maybe_error.has_value() == true);
        REQUIRE_THAT(maybe_error->what(), Contains("Action Boom"));
        REQUIRE(maybe_error->get_index().has_value() == true);
        REQUIRE(maybe_error->get_index().value() == 2);
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == VarInteger{ 3 });
    }

    SECTION("Invalid index")
    {
        REQUIRE_THROWS_AS(sequence.execute(context, nullptr, 3), Error);
        REQUIRE(std::get<VarInteger>(context.variables["a"]) == VarInteger{ 0 });
    }
}

TEST_CASE("Sequence: terminate sequence with Lua exit function", "[Sequence]")
{
    CommChannel comm;
    auto& queue = comm.queue_;

    Context ctx;
    Sequence seq{ "test_sequence" };

    Step step_while{Step::type_while};
    Step step_increment{Step::type_action};
    Step step_check_termination{Step::type_action};
    Step step_while_end{Step::type_end};

    step_while.set_label("loop until a >= 10");
    step_while.set_script("return a < 10");
    step_while.set_used_context_variable_names(VariableNames{"a"});

    step_increment.set_label("increment a");
    step_increment.set_script("a = a + 1");
    step_increment.set_used_context_variable_names(VariableNames{"a"});

    step_check_termination.set_label("exist sequence when a == 4");
    step_check_termination.set_script("if a == 4 then terminate_sequence() end");
    step_check_termination.set_used_context_variable_names(VariableNames{"a"});

    step_while_end.set_label("end loop");

    ctx.variables["a"] = VarInteger{ 0 };

    seq.push_back(step_while);
    seq.push_back(step_increment);
    seq.push_back(step_check_termination);
    seq.push_back(step_while_end);

    REQUIRE(seq.execute(ctx, &comm) == gul14::nullopt);

    // Both the sequence and all of its steps must show is_running() == false.
    REQUIRE(seq.is_running() == false);
    for (const auto& step : seq)
        REQUIRE(step.is_running() == false);

    REQUIRE(std::get<VarInteger>(ctx.variables["a"]) == 4);
    REQUIRE(not queue.empty());
    REQUIRE(queue.size() == 26);
    auto msg = queue.back();
    REQUIRE(msg.get_type() == Message::Type::sequence_stopped);
    REQUIRE(msg.get_text() == "Script called terminate_sequence()");
    REQUIRE(msg.get_index().has_value());
    REQUIRE(*(msg.get_index()) == 2);
}

TEST_CASE("Sequence: add step setup with variable", "[Sequence]")
{
    Context ctx;
    ctx.variables["a"] = VarString{ "" };
    ctx.variables["b"] = VarString{ "" };

    Step step_action_1{Step::type_action};
    step_action_1.set_script("a = preface .. 'Bob'");
    step_action_1.set_used_context_variable_names({VariableName{"a"}});

    Step step_action_2{Step::type_action};
    step_action_2.set_script("b = a .. ' and ' .. preface .. 'Marvin!'");
    step_action_2.set_used_context_variable_names({VariableName{"a"}, VariableName{"b"}});

    Sequence seq{ "test_sequence" };
    seq.push_back(step_action_1);
    seq.push_back(step_action_2);
    seq.set_step_setup_script("preface = 'Alice calls '");

    REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);

    REQUIRE(std::get<std::string>(ctx.variables["a"]) == "Alice calls Bob");
    REQUIRE(std::get<std::string>(ctx.variables["b"]) == "Alice calls Bob and Alice"
        " calls Marvin!");
    REQUIRE(ctx.step_setup_script == "preface = 'Alice calls '");
}

TEST_CASE("Sequence: add step setup with function", "[Sequence]")
{
    Context ctx;
    ctx.variables["a"] = VarString{ "" };
    ctx.variables["b"] = VarString{ "" };

    Step step_action_1{Step::type_action};
    step_action_1.set_script("a = preface('Bob!')");
    step_action_1.set_used_context_variable_names({VariableName{"a"}});

    Step step_action_2{Step::type_action};
    step_action_2.set_script("b = preface('Charlie') .. ' and ' .. preface('Eve!')");
    step_action_2.set_used_context_variable_names({VariableName{"b"}});

    Sequence seq{ "test_sequence" };
    seq.push_back(step_action_1);
    seq.push_back(step_action_2);
    seq.set_step_setup_script("function preface(name) return 'Alice calls ' .. name end");

    REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);

    REQUIRE(std::get<std::string>(ctx.variables["a"]) == "Alice calls Bob!");
    REQUIRE(std::get<std::string>(ctx.variables["b"]) == "Alice calls Charlie and"
        " Alice calls Eve!");
    REQUIRE(ctx.step_setup_script == "function preface(name) return 'Alice calls ' .. name end");
}

TEST_CASE("Sequence: add step setup with isolated function modification", "[Sequence]")
{
    Context ctx;
    ctx.variables["a"] = VarString{ "" };
    ctx.variables["b"] = VarString{ "" };
    ctx.variables["c"] = VarString{ "" };
    ctx.variables["d"] = VarString{ "" };

    Step step_action_1{Step::type_action};
    step_action_1.set_script("a = preface('Bob!')");
    step_action_1.set_used_context_variable_names({VariableName{"a"}});

    Step step_action_2{Step::type_action};
    step_action_2.set_script(
        R"(b = preface('Charlie!')
           function preface(name) return 'Bob calls ' .. name end
           c = preface('Marvin!')
        )");
    step_action_2.set_used_context_variable_names({VariableName{"b"}, VariableName{"c"}});

    Step step_action_3{Step::type_action};
    step_action_3.set_script("d = preface('Eve!')");
    step_action_3.set_used_context_variable_names({VariableName{"d"}});

    Sequence seq{ "test_sequence" };
    seq.push_back(step_action_1);
    seq.push_back(step_action_2);
    seq.push_back(step_action_3);
    seq.set_step_setup_script("function preface(name) return 'Alice calls ' .. name end");

    REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);

    REQUIRE(std::get<VarString>(ctx.variables["a"]) == "Alice calls Bob!");
    REQUIRE(std::get<VarString>(ctx.variables["b"]) == "Alice calls Charlie!");
    REQUIRE(std::get<VarString>(ctx.variables["c"]) == "Bob calls Marvin!");
    REQUIRE(std::get<VarString>(ctx.variables["d"]) == "Alice calls Eve!");
    REQUIRE(ctx.step_setup_script == "function preface(name) return 'Alice calls ' .. name end");
}

TEST_CASE("Sequence: Check line number on failure (setup at line 2)", "[Sequence]")
{
    Context ctx;

    Step step_action_1{Step::type_action};
    step_action_1.set_script("a = preface .. ' and Bob'");
    step_action_1.set_used_context_variable_names({VariableName{"a"}});

    Sequence seq{ "test_sequence" };
    seq.push_back(step_action_1);
    seq.set_step_setup_script(
        R"(preface = 'Alice'
           this line will fail
        )");

    auto maybe_error = seq.execute(ctx, nullptr);
    REQUIRE(maybe_error.has_value() == true);
    REQUIRE_THAT(maybe_error->what(), Catch::Matchers::StartsWith("[setup] 2: syntax error"));
}

TEST_CASE("Sequence: Check line number on failure (script at line 2)", "[Sequence]")
{
    Context ctx;

    Step step_action_1{Step::type_action};
    step_action_1.set_script(
        R"(a = preface .. ' and Bob'
           Oops this line will fail
        )");
    step_action_1.set_used_context_variable_names({VariableName{"a"}});

    Sequence seq{ "test_sequence" };
    seq.push_back(step_action_1);
    seq.set_step_setup_script("preface = 'Alice'");

    auto maybe_error = seq.execute(ctx, nullptr);
    REQUIRE(maybe_error.has_value() == true);
    REQUIRE_THAT(maybe_error->what(), Catch::Matchers::StartsWith("2: syntax error"));
}

TEST_CASE("Sequence: Check line number on failure (script at line 3)", "[Sequence]")
{
    Context ctx;

    Step step_action_1{Step::type_action};
    step_action_1.set_script(
        R"(a = preface .. ' and Bob'
           a = 1
           And again this line will fail
        )");
    step_action_1.set_used_context_variable_names({VariableName{"a"}});

    Sequence seq{ "test_sequence" };
    seq.push_back(step_action_1);
    seq.set_step_setup_script("preface = 'Alice'");

    auto maybe_error = seq.execute(ctx, nullptr);
    REQUIRE(maybe_error.has_value() == true);
    REQUIRE_THAT(maybe_error->what(), Catch::Matchers::StartsWith("3: syntax error"));
}

TEST_CASE("Sequence: Check setter/getter function for step setup script", "[Sequence]")
{
    const char script[] = "a = 'Alice'\nb = 'Bob'";

    Sequence seq{ "test_sequence" };
    seq.set_step_setup_script(script);

    SECTION("Sequence::get_step_setup_script()")
    {
        REQUIRE(seq.get_step_setup_script() == script);
    }

    SECTION("Context::step_setup_script")
    {
        Context ctx;
        REQUIRE(ctx.step_setup_script.empty());

        REQUIRE(seq.execute(ctx, nullptr) == gul14::nullopt);
        REQUIRE(ctx.step_setup_script == script);
    }
}

TEST_CASE("Sequence: sequence timeout", "[Sequence]")
{
    Step step{Step::type_action};
    step.set_script("sleep(1)");

    Sequence seq{"test_sequence"};
    seq.push_back(std::move(step));

    seq.set_timeout(10ms);
    REQUIRE(seq.get_timeout() == 10ms);

    Context ctx;
    REQUIRE(seq.get_time_of_last_execution() == task::TimePoint{});
    auto maybe_error = seq.execute(ctx, nullptr);
    REQUIRE(seq.get_time_of_last_execution() != task::TimePoint{});

    REQUIRE(maybe_error.has_value() == true);
    REQUIRE_THAT(maybe_error->what(), Contains("Timeout: Sequence"));
}
