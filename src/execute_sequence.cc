/**
 * \file   execute_sequence.cc
 * \author Marcus Walla
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

using Iterator = Sequence::Steps::iterator;

const std::string head = "[script] ";

/// Find next token with condition on function fetcher. Returns the found step
/// iterator.
/// Since the syntax is already validated there is a garante that it finds the proper 
/// step.
Iterator find(Sequence& sequence, Iterator step, std::function<bool(const Step& step)> 
    fetcher)
{
    return std::find_if(step, sequence.end(), [&](const Step& step) {
        return fetcher(step);
    });
}

/// Find reverse token with condition on function fetcher. Returns the found step 
/// iterator.
/// Since the syntax is already validated there is a garante that it finds the proper 
/// step.
Iterator find_reverse(Sequence& sequence, Iterator step, std::function<bool(const Step&
    step)> fetcher)
{
    auto step_reverse = std::find_if(std::make_reverse_iterator(step),
        std::make_reverse_iterator(sequence.begin()), [&](const Step& step) {
            return fetcher(step);
        });
    return step_reverse.base();
}

/**
 * Internal traverse execution loop depending on indentation level.
 * 
 * @param sequence executed \a Sequence
 * @param context \a Context for executing a step
 * @param step step iterator to traverse the \a Sequence
 * @param catch_block iterator on catch block if set. This iterator will be returned if
 *      the Lua code fails. When it is not set an \a Error exception is thrown. 
 * @exception throws \a Error when a fault on one of the statements is caught by Lua.
 */
Iterator execute_sequence_impl(Sequence& sequence, Context& context, Iterator step, 
    const short level, Iterator catch_block)
{
    bool found_while = false;
    bool found_try = false;
    bool if_condition = false;
    Iterator step_catch;

    while(step != sequence.end())
    {
        if (step->get_indentation_level() > level)
        {
            step = execute_sequence_impl(sequence, context, step, level + 1, step_catch);

            // no exception: need to reset the 'try' flag and 'catch' step
            found_try = false;
            step_catch = sequence.end();
        }
        else if (step->get_indentation_level() < level)
            return step;
        else
        {
            try
            {
            const bool result = execute_step(*step, context);

            switch(step->get_type())
            {
                case Step::type_while:
                    found_while = result;
                    if (result)
                        ++step;
                    else
                        step = find(sequence, step, [&](const Step& step) { 
                                return level          == step.get_indentation_level()
                                    && Step::type_end == step.get_type(); });
                    break;

                case Step::type_try:
                    found_try = true;
                    // Use the successive index ('.. + 1') for the first token in
                    // the catch block because its performing directly on the 'catch' 
                    // token and resets the index ()...which we do not want!)
                    step_catch = find(sequence, step, [&](const Step& step) {
                                return level            == step.get_indentation_level()
                                    && Step::type_catch == step.get_type(); }) + 1;
                    ++step;
                    break;

                case Step::type_catch:
                    step_catch = sequence.end();
                    step = find(sequence, step, [&](const Step& step) {
                                return level          == step.get_indentation_level()
                                     && Step::type_end == step.get_type(); });
                    break;

                case Step::type_if:
                    if_condition = result;
                    if (result)
                        ++step;
                    else
                        step = find(sequence, step, [&](const Step& step) {
                                return   level             == step.get_indentation_level()
                                   && (  Step::type_elseif == step.get_type()
                                       ||Step::type_else   == step.get_type()
                                       ||Step::type_end    == step.get_type() ); });
                    break;

                    case Step::type_elseif:
                        if (if_condition)
                            step = find(sequence, step, [&](const Step& step) {
                                    return level          == step.get_indentation_level()
                                        && Step::type_end == step.get_type(); });
                        else if (result)
                        {
                            if_condition = result;
                            ++step;
                        }
                        else
                            step = find(sequence, step + 1, [&](const Step& step) {
                                    return level           == step.get_indentation_level()
                                  && (   Step::type_elseif == step.get_type()
                                      || Step::type_else   == step.get_type()
                                      || Step::type_end    == step.get_type() ); });
                        break;

                    case Step::type_else:
                        if(if_condition)
                            step = find(sequence, step, [&](const Step& step) {
                                    return level          == step.get_indentation_level()
                                        && Step::type_end == step.get_type(); });
                        else
                            ++step;
                        break;

                case Step::type_end:
                    if (found_try)
                    {
                        ++step;
                        found_try = false;
                    }
                    else if (found_while)
                    {
                        step = find_reverse(sequence, step, [&](const Step& step) { 
                                return level            == step.get_indentation_level()
                                    && Step::type_while == step.get_type(); }) - 1; 
                        found_while = false;                            
                    }
                    else
                        ++step;
                    break;

                default:
                    ++step;
            }
            }
            catch(const Error& e)
            {
                if (catch_block != sequence.end())
                    return catch_block;
                else
                {
                    // Lua throws a runtime error that is not caught by a try block ...
                    throw Error{gul14::cat(head, "runtime error: ", e.what())};
                }
            }
        }
    }

    return step;
}

void execute_sequence(Sequence& sequence, Context& context)
{
    // syntax check
    sequence.check_syntax();

    execute_sequence_impl(sequence, context, sequence.begin(), 0, sequence.end());
}

}