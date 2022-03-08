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
#include "taskomat/execute_sequence.h"
#include "taskomat/execute_step.h"

namespace task {

const std::string head = "[script] ";

/// Find previous 'while' token begining from 'end' with successive decrement on index.
Sequence::SizeType find_previous_while_token(Sequence& sequence, const short level
    , Sequence::SizeType idx)
{
    const Sequence::SizeType while_end_idx = idx;

    // is ok because Sequence::SizeType is an alias for unsigned short
    while(--idx < sequence.size())
    {
        if (   level            == sequence[idx].get_indentation_level()
            && Step::type_while == sequence[idx].get_type() )
                return idx;
    }

    throw std::runtime_error(gul14::cat(head, "exceed limit of sequence steps. level=",
        level, ", previous 'end' of 'while' indication=", while_end_idx));
}

/// Find 'end' token in 'while' block with successive increment on index.
Sequence::SizeType find_while_end_token(Sequence& sequence, const short level
    , Sequence::SizeType idx)
{
    const Sequence::SizeType while_idx = idx;

    while(++idx < sequence.size())
    {
        if (   level          == sequence[idx].get_indentation_level()
            && Step::type_end == sequence[idx].get_type() )
                return idx;
    }

    throw std::runtime_error(gul14::cat(head, "exceed limit of sequence steps. level=",
        level, ", previous 'while' indication=", while_idx));
}

/// Find next 'catch' token in 'try' block with successive increment on index.
Sequence::SizeType find_next_try_catch_token(Sequence& sequence, const short level
    , Sequence::SizeType idx)
{
    const Sequence::SizeType try_idx = idx;

    while(++idx < sequence.size())
    {
        if (   level            == sequence[idx].get_indentation_level()
            && Step::type_catch == sequence[idx].get_type() )
                return idx;
    }

    throw std::runtime_error(gul14::cat(head, "exceed limit of sequence steps. level=",
        level, ", previous 'try' indication=", try_idx));
}

/// Find next 'end' token in 'try' block with successive increment on index.
Sequence::SizeType find_next_try_end_token(Sequence& sequence, const short level
    , Sequence::SizeType idx)
{
    const Sequence::SizeType catch_idx = idx;

    while(++idx < sequence.size())
    {
        if (   level          == sequence[idx].get_indentation_level()
            && Step::type_end == sequence[idx].get_type() )
                return idx;
    }

    throw std::runtime_error(gul14::cat(head, "exceed limit of sequence steps. level=",
        level, ", previous 'catch' indication=", catch_idx));
}

/// Find next 'elesif', 'else', or 'end' token in 'if' block with successive increment on
/// index.
Sequence::SizeType find_next_elseif_or_else_or_end_token(Sequence& sequence
    , const short level, Sequence::SizeType idx)
{
    const Sequence::SizeType if_idx = idx;

    while(++idx < sequence.size())
    {
        if (       level             == sequence[idx].get_indentation_level()
            && (   Step::type_elseif == sequence[idx].get_type()
                || Step::type_else   == sequence[idx].get_type()
                || Step::type_end    == sequence[idx].get_type() ) )
                return idx;
    }

    throw std::runtime_error(gul14::cat(head, "exceed limit of sequence steps. level=",
        level, ", previous 'if' indication=", if_idx));
}

/// Find next 'else' or 'end' token in 'if 'block with successive increment on index.
Sequence::SizeType find_next_else_or_end_token(Sequence& sequence, const short level
    , Sequence::SizeType idx)
{
    const Sequence::SizeType elseif_or_else_idx = idx;

    while(++idx < sequence.size())
    {
        if (       level           == sequence[idx].get_indentation_level()
            && (   Step::type_else == sequence[idx].get_type() 
                || Step::type_end  == sequence[idx].get_type() ) )
                return idx;
    }

    throw std::runtime_error(gul14::cat(head, "exceed limit of sequence steps. level=",
        level, ", previous 'elseif' or 'else' indication=", elseif_or_else_idx));
}

/// Find next 'end' token in 'if' block with successive increment on index.
Sequence::SizeType find_next_if_end_token(Sequence& sequence, const short level
    , Sequence::SizeType idx)
{
    const Sequence::SizeType if_elseif_else_idx = idx;

    while(++idx < sequence.size())
    {
        if (   level          == sequence[idx].get_indentation_level()
            && Step::type_end == sequence[idx].get_type() )
                return idx;
    }

    throw std::runtime_error(gul14::cat(head, "exceed limit of sequence steps. level=",
        level, ", previous 'if', 'elseif', or 'else' indication=", if_elseif_else_idx));
}

/// Validates if we have a next \a Step from \a Sequence.
bool has_step(Sequence& sequence, Sequence::SizeType idx)
{
    return idx < sequence.size() 
        || /* check if last element is 'end' or 'action' */
           (    idx > 1 
            && (idx + 1) == sequence.size() 
            && (    Step::type_end    == sequence[idx].get_type() 
                 || Step::type_action == sequence[idx].get_type() )
            );
}

/**
 * Internal sequence execution loop depending on indentation level.
 * 
 * @param sequence executing \a Sequence
 * @param context \a Context for executing the sequence
 * @param level Indentation level
 * @param idx sequence index
 * @param fount_nested_try flag to signal that when some statement fails it will be
 *      catch and continue execution on the index of the catch block. See parameter
 *      \a idx_nested_catch
 * @param idx_nested_catch index of the catch block. This is only relevant when the
 *      previous parameter \a found_nested_try is true
 * @exception throws \a Error when a fault on one of the statements is caught by Lua.
 * @exception can throw \a std::runtime_error when an internal fault is caught. It needs
 *      an investigation by the development team
 */ 
Sequence::SizeType execute_sequence_impl(Sequence& sequence, Context& context,
    const short level, Sequence::SizeType idx,
    const bool found_nested_try, const Sequence::SizeType idx_nested_catch)
{
    bool found_try = false;
    bool found_while = false;
    bool if_condition = false;
    Sequence::SizeType idx_catch = 0;

    while(has_step(sequence, idx))
    {
        const Step& step = sequence[idx];
        const short step_level = step.get_indentation_level();

        if (step_level > level)
        {
            idx = execute_sequence_impl(sequence, context, step_level, idx
                , found_try, idx_catch);

            // no exception: need to reset the 'try' flag and catch index
            found_try = false;
            idx_catch = 0;
        }
        else if (step_level < level)
            return idx;
        else
        {
            try
            {
                // TODO: explicit cast by dropping 'const'. Needs a fix in future release!
                const bool result = execute_step((Step&)(step), context);

                switch(step.get_type())
                {
                    case Step::type_while:
                        found_while = result;
                        if (result)
                            ++idx;
                        else
                            idx = find_while_end_token(sequence, level, idx);
                        break;

                    case Step::type_try:
                        found_try = true;
                        // Use the successive index ('.. + 1') for the first statement in
                        // the catch block  because performing directly on the 'catch' 
                        // index resets the catch index which we do not want!
                        idx_catch = find_next_try_catch_token(sequence, level, idx) + 1;
                        ++idx;
                        break;

                    case Step::type_catch:
                        idx_catch = 0;
                        idx = find_next_try_end_token(sequence, level, idx);
                        break;

                    case Step::type_if:
                        if_condition = result;
                        if (result)
                            ++idx;
                        else
                            idx = find_next_elseif_or_else_or_end_token(sequence, level
                                , idx);
                        break;

                    case Step::type_elseif:
                        if (if_condition)
                            idx = find_next_if_end_token(sequence, level, idx);
                        else if (result)
                        {
                            if_condition = result;
                            ++idx;
                        }
                        else
                            idx = find_next_elseif_or_else_or_end_token(sequence, level
                                , idx);
                        break;

                    case Step::type_else:
                        if(if_condition)
                            idx = find_next_if_end_token(sequence, level, idx);
                        else
                            ++idx;
                        break;

                    case Step::type_end:
                        if (found_try)
                        {
                            ++idx;
                            found_try = false;
                        }
                        else if (found_while)
                        {
                            idx = find_previous_while_token(sequence, level, idx);
                            found_while = false;                            
                        }
                        else
                            ++idx;
                        break;

                    default:
                        ++idx;
                        break;
                }
            }
            catch(const Error& e)
            {
                if (found_nested_try)
                    return idx_nested_catch;
                else
                {
                    // Lua throws a runtime error that is not caught by a try block ...
                    throw Error{gul14::cat(head, "runtime error [index=", idx, "]: "
                        , e.what())};
                }
            }
        }
    }

    // To satisfy the base calling function 'execute_sequence' ...
    return 0;
}

void execute_sequence(Sequence& sequence, Context& context)
{
    // syntax check
    sequence.check_syntax();

    execute_sequence_impl(sequence, context, 0, 0, false, 0);
}

}