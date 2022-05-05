/**
 * \file   execute_sequence.cc
 * \author Marcus Walla, Lars Froehlich
 * \date   Created on February 16, 2022
 * \brief  Implementation of the execute_sequence() free function.
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
#include <algorithm>
#include <iterator>
#include "taskomat/execute_sequence.h"
#include "taskomat/execute_step.h"

namespace task {

using Iterator = Sequence::Steps::const_iterator;
using ReverseIterator = Sequence::Steps::const_reverse_iterator;

namespace
{

/// Find next token with condition on predicate. Returns the found step iterator.
/// Since the syntax is already validated there is a garante to find the proper step.
template<typename Predicate>
Iterator find(Iterator step, Iterator end, Predicate pred)
{
    return std::find_if(step, end, [&](const Step& step) { return pred(step); });
}

/// Find reverse token with condition on predicate. Returns the found step iterator.
/// Since the syntax is already validated there is a garante to find the proper step.
template <typename Predicate>
Iterator find_reverse(Iterator step, ReverseIterator end, Predicate pred)
{
    auto step_reverse = std::find_if(
        std::make_reverse_iterator(step),
        end,
        [&](const Step& step) { return pred(step); });
    return step_reverse.base();
}

/**
 * Internal traverse execution loop depending on indentation level.
 *
 * @param step_begin Iterator to the first step that should be executed
 * @param step_end   Iterator past the last step that should be executed
 * @param context    Context for executing the steps
 * @exception Error is thrown if a fault on one of the statements is caught by Lua.
 */
Iterator execute_sequence_impl(Iterator step_begin, Iterator step_end, Context& context)
{
    Iterator step = step_begin;

    while (step < step_end)
    {
        switch (step->get_type())
        {
            case Step::type_while:
            {
                const auto block_end = detail::find_end_of_indented_block(
                    step + 1, step_end, step->get_indentation_level() + 1);

                while (execute_step((Step&)*step, context))
                    execute_sequence_impl(step + 1, block_end, context);

                step = block_end;
            }
            break;

            case Step::type_try:
            {
                const auto it_catch = detail::find_end_of_indented_block(
                    step + 1, step_end, step->get_indentation_level() + 1);

                if (it_catch == step_end || it_catch->get_type() != Step::type_catch)
                    throw Error("Missing catch block");

                const auto it_catch_block_end = detail::find_end_of_indented_block(
                    it_catch + 1, step_end, step->get_indentation_level() + 1);

                try
                {
                    execute_sequence_impl(step + 1, it_catch, context);
                }
                catch (const Error&)
                {
                    execute_sequence_impl(it_catch + 1, it_catch_block_end, context);
                }

                step = it_catch_block_end;
            }
            break;

            case Step::type_catch:
                throw Error("Catch block without associated try");

            case Step::type_if:
            case Step::type_elseif:
            {
                const auto block_end = detail::find_end_of_indented_block(
                    step + 1, step_end, step->get_indentation_level() + 1);

                if (execute_step((Step&)*step, context))
                {
                    execute_sequence_impl(step + 1, block_end, context);
                    step = std::find_if(block_end, step_end,
                        [lvl = step->get_indentation_level()](const Step& s)
                        {
                            return s.get_indentation_level() == lvl &&
                                    s.get_type() == Step::type_end;
                        });
                    if (step == step_end)
                        throw Error("IF without matching END");
                    ++step;
                }
                else
                {
                    step = block_end;
                }
            }
            break;

            case Step::type_else:
            {
                const auto block_end = detail::find_end_of_indented_block(
                    step + 1, step_end, step->get_indentation_level() + 1);

                execute_sequence_impl(step + 1, block_end, context);

                step = block_end;
            }
            break;

            case Step::type_end:
                ++step;
                break;

            case Step::type_action:
                execute_step((Step&)*step, context);
                ++step;
                break;

            default:
                throw Error{"wrong step type"};
        }
    }

    return step;
}

} // anonymous namespace

void execute_sequence(Sequence& sequence, Context& context)
{
    // syntax check
    sequence.check_syntax();

    if (not sequence.empty())
        execute_sequence_impl(sequence.begin(), sequence.end(), context);
}

} // namespace task