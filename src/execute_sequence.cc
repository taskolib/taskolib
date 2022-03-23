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

/// Checks if \a Step has an excutable Lua script:
/// - \a Step::type_if 
/// - \a Step::type_elseif 
/// - \a Step::type_while
/// - \a Step::type_action
bool has_lua_script(Step::Type type)
{
    return type == Step::type_action || type == Step::type_if || type == Step::type_elseif
        || type == Step::type_while;
}

/**
 * Internal traverse execution loop depending on indentation level.
 * 
 * @param sequence executed \a Sequence
 * @param context \a Context for executing a step
 * @param step step iterator to traverse the \a Sequence
 * @exception throws \a Error when a fault on one of the statements is caught by Lua.
 */
Iterator execute_sequence_impl(Sequence& sequence, Context& context, Iterator step)
{
    bool found_while = false;
    bool found_try = false;
    bool if_condition = false;

    // store indentation level from first iteration step
    const short level = step != sequence.end() ? step->get_indentation_level() : 0;

    while(step != sequence.end())
    {
        if (step->get_indentation_level() < level)
            return step;

        const bool result = has_lua_script(step->get_type()) 
            ? execute_step((Step&)*step, context) 
            : false;

        switch(step->get_type())
        {
            case Step::type_while:
                found_while = result;
                if (result)
                    step = execute_sequence_impl(sequence, context, step + 1);
                else
                    step = find(step, sequence.cend(), [&](const Step& s) { 
                        return step->get_indentation_level() == s.get_indentation_level()
                               &&             Step::type_end == s.get_type(); });
                break;

            case Step::type_try:
                found_try = true;
                try
                {
                    step = execute_sequence_impl(sequence, context, step + 1);
                }
                catch(const Error& e)
                {
                    // Use the successor index ('.. + 1') for the first token in the catch
                    // block.
                    step = find(step, sequence.cend(), [&](const Step& s) {
                        return step->get_indentation_level() == s.get_indentation_level()
                               &&           Step::type_catch == s.get_type(); }) + 1;
                }
                
                break;

            case Step::type_catch:
                step = find(step, sequence.cend(), [&](const Step& s) {
                    return step->get_indentation_level() == s.get_indentation_level()
                           &&             Step::type_end == s.get_type(); });
                break;

            case Step::type_if:
                if_condition = result;
                if (result)
                    step = execute_sequence_impl(sequence, context, step + 1);
                else
                    step = find(step, sequence.cend(), [&](const Step& s) {
                        return step->get_indentation_level() == s.get_indentation_level()
                               && (        Step::type_elseif == s.get_type()
                                   ||      Step::type_else   == s.get_type()
                                   ||      Step::type_end    == s.get_type() ); });
                break;

            case Step::type_elseif:
                if (if_condition)
                    step = find(step, sequence.cend(), [&](const Step& s) {
                        return step->get_indentation_level() == s.get_indentation_level()
                            &&                Step::type_end == s.get_type(); });
                else if (result)
                {
                    if_condition = result;
                    step = execute_sequence_impl(sequence, context, step + 1);
                }
                else
                    step = find(step + 1, sequence.cend(), [&](const Step& s) {
                        return step->get_indentation_level() == s.get_indentation_level()
                               && (        Step::type_elseif == s.get_type()
                                   ||      Step::type_else   == s.get_type()
                                   ||      Step::type_end    == s.get_type() ); });
                break;

            case Step::type_else:
                if(if_condition)
                    step = find(step, sequence.cend(), [&](const Step& s) {
                        return step->get_indentation_level() == s.get_indentation_level()
                               &&             Step::type_end == s.get_type(); });
                else
                    step = execute_sequence_impl(sequence, context, step + 1);
                break;

            case Step::type_end:
                if (found_while)
                {
                    // Use the predecessor index ('.. - 1') for the first token in the
                    // while block.
                    step = find_reverse(step, sequence.crend(), [&](const Step& s) { 
                        return step->get_indentation_level() == s.get_indentation_level()
                               &&           Step::type_while == s.get_type(); }) - 1; 
                    found_while = false;                            
                }
                else
                {
                    if (found_try)
                        found_try = false;
                    ++step;
                }
                break;

            case Step::type_action:
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
        execute_sequence_impl(sequence, context, sequence.begin());
}

} // namespace task