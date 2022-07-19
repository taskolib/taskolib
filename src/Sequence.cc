/**
 * \file   Sequence.cc
 * \author Marcus Walla, Lars Froehlich
 * \date   Created on February 8, 2022
 * \brief  A sequence of Steps.
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

#include <gul14/substring_checks.h>
#include "taskomat/Sequence.h"
#include "taskomat/Step.h"

using gul14::cat;

namespace task {


// Anonymous namespace with implementation details
namespace {

template <typename IteratorT>
IteratorT
find_end_of_indented_block(IteratorT begin, IteratorT end, short min_indentation_level)
{
    auto it = std::find_if(begin, end,
        [=](const Step& step)
        {
            return step.get_indentation_level() < min_indentation_level;
        });

    if (it == end)
        return begin;
    else
        return it;
}

} // anonymous namespace


Sequence::Sequence(gul14::string_view label)
{
    check_label(label);
    label_ = std::string{ label };
}

void Sequence::check_syntax() const
{
    if (not indentation_error_.empty())
        throw Error(indentation_error_);

    check_syntax(steps_.begin(), steps_.end());
}

void Sequence::check_label(gul14::string_view label)
{
    if (label.empty())
        throw Error("Sequence label may not be empty");

    if (label.size() > max_label_length)
    {
        throw Error(cat("Label \"", label, "\" is too long (>", max_label_length,
                        " characters)"));
    }
}

Sequence::Iterator
Sequence::execute_else_block(Iterator begin, Iterator end, Context& context,
                             CommChannel* comm)
{
    const auto block_end = find_end_of_indented_block(
        begin + 1, end, begin->get_indentation_level() + 1);

    execute_sequence_impl(begin + 1, block_end, context, comm);

    return block_end;
}

Sequence::Iterator
Sequence::execute_if_or_elseif_block(Iterator begin, Iterator end, Context& context,
                                     CommChannel* comm)
{
    const auto block_end = find_end_of_indented_block(
        begin + 1, end, begin->get_indentation_level() + 1);

    if (begin->execute(context, comm, begin - steps_.begin()))
    {
        execute_sequence_impl(begin + 1, block_end, context, comm);

        // Skip forward past the END
        auto end_it = std::find_if(block_end, end,
            [lvl = begin->get_indentation_level()](const Step& s)
            {
                return s.get_indentation_level() == lvl &&
                       s.get_type() == Step::type_end;
            });
        if (end_it == end)
            throw Error("IF without matching END");
        return end_it + 1;
    }

    return block_end;
}

Sequence::Iterator
Sequence::execute_sequence_impl(Iterator step_begin, Iterator step_end, Context& context,
                                CommChannel* comm)
{
    Iterator step = step_begin;
    auto disable = false;

    while (step < step_end)
    {
        disable = disable or step->is_disabled();
        if (disable) {
            step = find_end_of_indented_block(step + 1, step_end, step->get_indentation_level() + 1);
            if (step->is_continuation_type())
                continue;
        }
        disable = false;
        switch (step->get_type())
        {
            case Step::type_while:
                step = execute_while_block(step, step_end, context, comm);
                break;

            case Step::type_try:
                step = execute_try_block(step, step_end, context, comm);
                break;

            case Step::type_if:
            case Step::type_elseif:
                step = execute_if_or_elseif_block(step, step_end, context, comm);
                break;

            case Step::type_else:
                step = execute_else_block(step, step_end, context, comm);
                break;

            case Step::type_end:
                ++step;
                break;

            case Step::type_action:
                step->execute(context, comm, step - steps_.begin());
                ++step;
                break;

            default:
                throw Error{"Unexpected step type"};
        }
    }

    return step;
}

Sequence::Iterator
Sequence::execute_try_block(Iterator begin, Iterator end, Context& context,
                            CommChannel* comm)
{
    const auto it_catch = find_end_of_indented_block(
        begin + 1, end, begin->get_indentation_level() + 1);

    if (it_catch == end || it_catch->get_type() != Step::type_catch)
        throw Error("Missing catch block");

    const auto it_catch_block_end = find_end_of_indented_block(
        it_catch + 1, end, begin->get_indentation_level() + 1);

    try
    {
        execute_sequence_impl(begin + 1, it_catch, context, comm);
    }
    catch (const Error& e)
    {
        // Typical error message with abort marker:
        // "Error while executing script of step 3: sol: runtime error: [ABORT] Step aborted on user request"
        if (gul14::contains(e.what(), "[ABORT]"))
            throw;

        execute_sequence_impl(it_catch + 1, it_catch_block_end, context, comm);
    }

    return it_catch_block_end;
}

Sequence::Iterator
Sequence::execute_while_block(Iterator begin, Iterator end, Context& context,
                              CommChannel* comm)
{
    const auto block_end = find_end_of_indented_block(
        begin + 1, end, begin->get_indentation_level() + 1);

    while (begin->execute(context, comm, begin - steps_.begin()))
        execute_sequence_impl(begin + 1, block_end, context, comm);

    return block_end + 1;
}

void Sequence::indent() noexcept
{
    short level = 0;

    indentation_error_.clear();

    for (Step& step : steps_)
    {
        short step_level = -1;

        switch (step.get_type())
        {
            case Step::type_action:
                step_level = level;
                break;
            case Step::type_if:
            case Step::type_try:
            case Step::type_while:
                step_level = level;
                ++level;
                break;
            case Step::type_catch:
            case Step::type_else:
            case Step::type_elseif:
                step_level = level - 1;
                break;
            case Step::type_end:
                step_level = level - 1;
                --level;
                break;
        };

        if (step_level < 0)
        {
            step_level = 0;

            if (indentation_error_.empty())
                indentation_error_ = "Steps are not nested correctly";
        }

        step.set_indentation_level(step_level);// cannot throw because we check step_level

        if (level < 0)
        {
            level = 0;
            if (indentation_error_.empty())
            {
                indentation_error_ = "Steps are not nested correctly (every END must "
                    "correspond to one IF, TRY, or WHILE)";
            }
        }
        else if (level > Step::max_indentation_level)
        {
            level = Step::max_indentation_level;
            if (indentation_error_.empty())
            {
                indentation_error_ = cat("Steps are nested too deeply (max. level: ",
                                         Step::max_indentation_level, ')');
            }
        }
    }

    if (level != 0)
    {
        if (indentation_error_.empty())
        {
            indentation_error_ = "Steps are not nested correctly (there must be one END "
                "for each IF, TRY, WHILE)";
        }
    }
}

void Sequence::check_syntax(Sequence::ConstIterator begin, Sequence::ConstIterator end)
    const
{
    ConstIterator step = begin;

    while (step < end)
    {
        switch (step->get_type())
        {
            case Step::type_while:
                step = check_syntax_for_while(step, end);
                break;

            case Step::type_try:
                step = check_syntax_for_try(step, end);
                break;

            case Step::type_if:
                step = check_syntax_for_if(step, end);
                break;

            case Step::type_action:
                ++step;
                break;

            case Step::type_catch:
                throw_syntax_error_for_step(step, "CATCH without matching TRY");
                break;

            case Step::type_elseif:
                throw_syntax_error_for_step(step, "ELSE IF without matching IF");
                break;

            case Step::type_else:
                throw_syntax_error_for_step(step, "ELSE without matching IF");
                break;

            case Step::type_end:
                throw_syntax_error_for_step(step, "END without matching IF/WHILE/TRY");
                break;

            default:
                throw_syntax_error_for_step(step, "Unexpected step type");
        }
    }
}

Sequence::ConstIterator Sequence::check_syntax_for_while(Sequence::ConstIterator begin,
    Sequence::ConstIterator end) const
{
    const auto block_end = find_end_of_indented_block(
        begin + 1, end, begin->get_indentation_level() + 1);

    if (block_end == end || block_end->get_type() != Step::type_end)
        throw_syntax_error_for_step(begin, "WHILE without matching END");

    check_syntax(begin + 1, block_end);

    return block_end + 1;
}

Sequence::ConstIterator Sequence::check_syntax_for_try(Sequence::ConstIterator begin,
    Sequence::ConstIterator end) const
{
    const auto it_catch = find_end_of_indented_block(
        begin + 1, end, begin->get_indentation_level() + 1);

    if (it_catch == end || it_catch->get_type() != Step::type_catch)
        throw_syntax_error_for_step(begin, "TRY without matching CATCH");

    check_syntax(begin + 1, it_catch); // block between TRY and CATCH

    const auto it_catch_block_end = find_end_of_indented_block(
        it_catch + 1, end, begin->get_indentation_level() + 1);

    if (it_catch_block_end == end || it_catch_block_end->get_type() != Step::type_end)
        throw_syntax_error_for_step(begin, "TRY...CATCH without matching END");

    check_syntax(it_catch + 1, it_catch_block_end); // block between CATCH and END

    return it_catch_block_end + 1;
}

Sequence::ConstIterator Sequence::check_syntax_for_if(Sequence::ConstIterator begin,
    Sequence::ConstIterator end) const
{
    bool else_found = false;
    auto it_block_statement = begin;

    while (true)
    {
        const auto it = find_end_of_indented_block(
            it_block_statement + 1, end, begin->get_indentation_level() + 1);

        if (it == end)
            throw_syntax_error_for_step(begin, "IF without matching END");

        check_syntax(it_block_statement + 1, it);

        switch (it->get_type())
        {
            case Step::type_elseif:
                if (else_found)
                    throw_syntax_error_for_step(it, "ELSE IF after ELSE clause");
                break;

            case Step::type_else:
                if (else_found)
                    throw_syntax_error_for_step(it, "Duplicate ELSE clause");
                else_found = true;
                break;

            case Step::type_end:
                return it + 1;

            default:
                throw_syntax_error_for_step(it, "Unfinished IF construct");
        }

        it_block_statement = it;
    }
}

void Sequence::execute(Context& context, CommChannel* comm)
{
    check_syntax();

    send_message(comm, Message::Type::sequence_started, "Sequence started",
                 Clock::now(), 0);

    try
    {
        execute_sequence_impl(steps_.begin(), steps_.end(), context, comm);
    }
    catch (const std::exception& e)
    {
        send_message(comm, Message::Type::sequence_stopped_with_error, e.what(),
                     Clock::now(), 0);
        throw;
    }

    send_message(comm, Message::Type::sequence_stopped, "Sequence finished",
                 Clock::now(), 0);
}

void Sequence::throw_syntax_error_for_step(Sequence::ConstIterator it,
    gul14::string_view msg) const
{
    throw Error(cat("[syntax check] Step ", it - steps_.begin() + 1, ": ", msg));
}

} // namespace task
