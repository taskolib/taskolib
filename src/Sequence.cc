/**
 * \file   Sequence.cc
 * \author Marcus Walla, Lars Froehlich
 * \date   Created on February 8, 2022
 * \brief  A sequence of Steps.
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

#include <iterator>

#include <gul14/join_split.h>
#include <gul14/SmallVector.h>
#include <gul14/string_view.h>
#include <gul14/substring_checks.h>
#include <gul14/trim.h>

#include "internals.h"
#include "lua_details.h"
#include "send_message.h"
#include "taskolib/exceptions.h"
#include "taskolib/Sequence.h"
#include "taskolib/Step.h"
#include "taskolib/time_types.h"

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
    set_label(label);
}

void Sequence::assign(Sequence::ConstIterator iter, const Step& step)
{
    throw_if_running();
    auto it = steps_.begin() + (iter - steps_.cbegin());
    *it = step;
    enforce_invariants();
}

void Sequence::assign(Sequence::ConstIterator iter, Step&& step)
{
    throw_if_running();
    auto it = steps_.begin() + (iter - steps_.cbegin());
    *it = std::move(step);
    enforce_invariants();
}

void Sequence::check_syntax() const
{
    if (not indentation_error_.empty())
        throw Error(indentation_error_);

    check_syntax(steps_.begin(), steps_.end());
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

void Sequence::enforce_consistency_of_disabled_flags() noexcept
{
    auto step = steps_.begin();

    while (step != steps_.end())
    {
        short level = step->get_indentation_level();

        switch (step->get_type())
        {
            case Step::type_if:
            case Step::type_try:
            case Step::type_while:
            {
                const auto it_end = find_end_of_continuation(step);

                if (step->is_disabled())
                {
                    // Disable the entire block-with-continuation, then continue afterwards
                    std::for_each(step, it_end, [](Step& st) { st.set_disabled(true); });
                    step = it_end;
                }
                else
                {
                    // Disable the associated else/elseif/end/catch statements, then
                    // continue with the next statement
                    std::for_each(step, it_end,
                        [level](Step& st)
                        {
                            if (st.get_indentation_level() == level)
                                st.set_disabled(false);
                        });
                    ++step;
                }
                break;
            }
            case Step::type_action:
            case Step::type_catch:
            case Step::type_else:
            case Step::type_elseif:
            case Step::type_end:
                ++step;
                break;
        };
    }
}

void Sequence::enforce_invariants()
{
    indent();
    enforce_consistency_of_disabled_flags();
}

Sequence::ConstIterator Sequence::erase(Sequence::ConstIterator iter)
{
    throw_if_running();
    auto return_iter = steps_.erase(iter);
    enforce_invariants();
    return return_iter;
}

Sequence::ConstIterator Sequence::erase(Sequence::ConstIterator begin,
                                        Sequence::ConstIterator end)
{
    throw_if_running();
    auto return_iter = steps_.erase(begin, end);
    enforce_invariants();
    return return_iter;
}

gul14::optional<Error>
Sequence::execute(Context& context, CommChannel* comm_channel,
                  OptionalStepIndex opt_step_index)
{
    if (opt_step_index) // single-step execution
    {
        const auto step_index = *opt_step_index;
        const auto step_it = steps_.begin() + step_index;

        if (step_index >= size())
            throw Error(cat("Invalid step index ", step_index));

        return handle_execution(context, comm_channel,
            cat("Single-step execution (", to_string(step_it->get_type()), " \"",
                step_it->get_label(), "\")"),
            [this, step_index, step_it](Context& context, CommChannel* comm)
            {
                if (executes_script(step_it->get_type()))
                    step_it->execute(context, comm, step_index, &timeout_trigger_);
            });
    }

    // full sequence execution
    return handle_execution(context, comm_channel, "Sequence",
        [this](Context& context, CommChannel* comm)
        {
            check_syntax();
            timeout_trigger_.reset();
            execute_range(steps_.begin(), steps_.end(), context, comm);
        });
}

gul14::optional<Error>
Sequence::handle_execution(Context& context, CommChannel* comm,
                           gul14::string_view exec_block_name,
                           std::function<void(Context&, CommChannel*)> runner)
{
    const auto clear_is_running_at_function_exit =
        gul14::finally([this]{ is_running_ = false; });

    is_running_ = true;

    context.step_setup_script = step_setup_script_;

    send_message(Message::Type::sequence_started, cat(exec_block_name, " started"),
                 Clock::now(), gul14::nullopt, context, comm);

    gul14::optional<Error> maybe_error;

    try
    {
        runner(context, comm);
    }
    catch (const Error& e)
    {
        maybe_error = e;
    }
    catch (const std::exception& e)
    {
        maybe_error = Error{ e.what() };
    }

    if (maybe_error)
    {
        auto [msg, cause] = remove_abort_markers(maybe_error->what());

        switch (cause)
        {
        case ErrorCause::terminated_by_script:
            send_message(Message::Type::sequence_stopped, msg, Clock::now(),
                         maybe_error->get_index(), context, comm);
            return gul14::nullopt; // silently return to the caller
        case ErrorCause::aborted:
            msg = cat(exec_block_name, " aborted: ", msg);
            break;
        case ErrorCause::uncaught_error:
            msg = cat(exec_block_name, " stopped with error: ", msg);
            break;
        }

        send_message(Message::Type::sequence_stopped_with_error, msg, Clock::now(),
                     maybe_error->get_index(), context, comm);
    }
    else
    {
        send_message(Message::Type::sequence_stopped, cat(exec_block_name, " finished"),
                     Clock::now(), gul14::nullopt, context, comm);
    }

    set_error(maybe_error);
    return maybe_error;
}

Sequence::Iterator
Sequence::execute_else_block(Iterator begin, Iterator end, Context& context,
                             CommChannel* comm)
{
    const auto block_end = find_end_of_indented_block(
        begin + 1, end, begin->get_indentation_level() + 1);

    execute_range(begin + 1, block_end, context, comm);

    return block_end;
}

Sequence::Iterator
Sequence::execute_if_or_elseif_block(Iterator begin, Iterator end, Context& context,
                                     CommChannel* comm)
{
    const auto block_end = find_end_of_indented_block(
        begin + 1, end, begin->get_indentation_level() + 1);

    if (begin->execute(context, comm, begin - steps_.begin(), &timeout_trigger_))
    {
        execute_range(begin + 1, block_end, context, comm);

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
Sequence::execute_range(Iterator step_begin, Iterator step_end, Context& context,
                        CommChannel* comm)
{
    Iterator step = step_begin;

    while (step < step_end)
    {
        if (step->is_disabled())
        {
            ++step;
            continue;
        }

        if (comm and comm->immediate_termination_requested_)
        {
            throw Error{ gul14::cat(abort_marker, "Stop on user request"),
                         static_cast<StepIndex>(step - steps_.begin()) };
        }

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
                step->execute(context, comm, step - steps_.begin(), &timeout_trigger_);
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
        execute_range(begin + 1, it_catch, context, comm);
    }
    catch (const Error& e)
    {
        // Typical error message with (non-literal) abort marker:
        // "Error while executing script of step 3: sol: runtime error: [ABORT]Stop on user request[ABORT]"
        if (gul14::contains(e.what(), abort_marker))
            throw;

        execute_range(it_catch + 1, it_catch_block_end, context, comm);
    }

    return it_catch_block_end;
}

Sequence::Iterator
Sequence::execute_while_block(Iterator begin, Iterator end, Context& context,
                              CommChannel* comm)
{
    const auto block_end = find_end_of_indented_block(
        begin + 1, end, begin->get_indentation_level() + 1);

    while (begin->execute(context, comm, begin - steps_.begin(), &timeout_trigger_))
        execute_range(begin + 1, block_end, context, comm);

    return block_end + 1;
}

Sequence::Iterator
Sequence::find_end_of_continuation(Sequence::Iterator block_start)
{
    auto it = std::find_if(block_start, steps_.end(),
        [lvl = block_start->get_indentation_level()](const Step& s)
        {
            return s.get_indentation_level() == lvl &&
                   s.get_type() == Step::type_end;
        });

    if (it != steps_.end())
        ++it;

    return it;
}

Sequence::ConstIterator
Sequence::find_end_of_continuation(Sequence::ConstIterator block_start) const
{
    auto it = std::find_if(block_start, steps_.end(),
        [lvl = block_start->get_indentation_level()](const Step& s)
        {
            return s.get_indentation_level() == lvl &&
                   s.get_type() == Step::type_end;
        });

    if (it != steps_.end())
        ++it;

    return it;
}

// The default for disable_level must be representable:
static_assert(Step::max_indentation_level
    < std::numeric_limits<decltype(Step::max_indentation_level)>::max());

void Sequence::indent()
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

Sequence::ConstIterator Sequence::insert(Sequence::ConstIterator iter, const Step& step)
{
    throw_if_running();
    throw_if_full();

    auto return_iter = steps_.insert(iter, step);
    enforce_invariants();
    return return_iter;
}

Sequence::ConstIterator Sequence::insert(Sequence::ConstIterator iter, Step&& step)
{
    throw_if_running();
    throw_if_full();

    auto return_iter = steps_.insert(iter, std::move(step));
    enforce_invariants();
    return return_iter;
}

void Sequence::pop_back()
{
    throw_if_running();
    if (not steps_.empty())
        steps_.pop_back();
    enforce_invariants();
}

void Sequence::push_back(const Step& step)
{
    throw_if_running();
    throw_if_full();
    steps_.push_back(step);
    enforce_invariants();
}

void Sequence::push_back(Step&& step)
{
    throw_if_running();
    throw_if_full();
    steps_.push_back(step);
    enforce_invariants();
}

void Sequence::set_error(gul14::optional<Error> opt_error)
{
    error_ = std::move(opt_error);
}

void Sequence::set_label(gul14::string_view label)
{
    label = gul14::trim_sv(label);

    if (label.empty())
        throw Error("Sequence label may not be empty");

    if (label.size() > max_label_length)
    {
        throw Error(cat("Label \"", label, "\" is too long (>", max_label_length,
                        " bytes)"));
    }

    label_.assign(label.begin(), label.end());
}

void Sequence::set_step_setup_script(gul14::string_view step_setup_script)
{
    throw_if_running();

    // remove trailing whitespaces
    step_setup_script = gul14::trim_right_sv(step_setup_script);

    step_setup_script_.assign(step_setup_script.data(), step_setup_script.size());
}

void Sequence::throw_if_full() const
{
    if (steps_.size() == max_size())
        throw Error(cat("Reached maximum sequence size (", max_size(), " steps)"));
}

void Sequence::throw_if_running() const
{
    if (is_running_)
        throw Error("Cannot change a running sequence");
}

void Sequence::throw_syntax_error_for_step(Sequence::ConstIterator /*it*/,
    gul14::string_view msg) const
{
    throw Error(cat("Syntax error: ", msg));
}

} // namespace task
