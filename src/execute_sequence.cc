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

size_t find_try_end(const Step& step, Sequence& sequence, size_t idx)
{
    const short indent = step.get_indentation_level(); 

    while(++idx < sequence.size())
        if ( sequence[idx].get_indentation_level() == indent
            && sequence[idx].get_type() == Step::type_end )
            return idx;

    throw std::runtime_error(gul14::cat("taskomat script, inconsistency: finding "
        "end clause after try-catch fails [label: '", step.get_label(), "', level=",
        indent, "]"));
}

size_t find_while(const Step& step, Sequence& sequence, size_t idx)
{
    const short indent = step.get_indentation_level(); 

    while(--idx < sequence.size())
        if ( sequence[idx].get_indentation_level() == indent
            && sequence[idx].get_type() == Step::type_while )
            return idx;

    throw std::runtime_error(gul14::cat("taskomat script, inconsistency: finding "
        "while clause in loop fails [label: '", step.get_label(), "', level=",
        indent, "]"));
}

size_t find_while_end(const Step& step, Sequence& sequence, size_t idx)
{
    const short indent = step.get_indentation_level(); 

    while(++idx < sequence.size())
        if ( sequence[idx].get_indentation_level() == indent
            && sequence[idx].get_type() == Step::type_end )
            return idx;

    throw std::runtime_error(gul14::cat("taskomat script, inconsistency: finding "
        "end clause after while fails [label: '", step.get_label(), "', level=",
        indent, "]"));
}

size_t find_next_if_construct(const Step& step, Sequence& sequence, size_t idx)
{
    const short indent = step.get_indentation_level(); 

    while(++idx < sequence.size())
        if ( sequence[idx].get_indentation_level() == indent &&
            ( sequence[idx].get_type() == Step::type_elseif ||
              sequence[idx].get_type() == Step::type_else   ||
              sequence[idx].get_type() == Step::type_end    ))
            return idx;

    throw std::runtime_error(gul14::cat("taskomat script, inconsistency: finding next "
        "elseif/elseif clause for if fails [label: '", step.get_label(), "', level=",
        indent, "]"));
}

size_t find_if_end(const Step& step, Sequence& sequence, size_t idx)
{
    const short indent = step.get_indentation_level(); 

    while(++idx < sequence.size())
        if ( sequence[idx].get_indentation_level() == indent &&
            sequence[idx].get_type() == Step::type_end )
            return idx;

    throw std::runtime_error(gul14::cat("taskomat script, inconsistency: finding "
        "end clause for if fails [label: '", step.get_label(), "', level=",
        indent, "]"));
}

size_t find_try_catch(const Step& step, Sequence& sequence, size_t idx)
{
    const short indent = step.get_indentation_level(); 

    while(++idx < sequence.size())
        if ( sequence[idx].get_indentation_level() == indent
            && sequence[idx].get_type() == Step::type_catch )
            return idx;

    throw std::runtime_error(gul14::cat("taskomat script, inconsistency: finding "
        "catch clause for try fails [label: '", step.get_label(), "', level=",
        indent, "]"));
}

void execute_sequence(Sequence& sequence, Context& context)
{
    // syntax check
    sequence.check_correctness_of_steps();

    size_t idx{0};
    const Step* ptr_try_step = nullptr;
    bool while_condition = false;
    bool if_condition = false;
    bool try_condition = false;
    while(true)
    {
        try
        {
            const Step& step = sequence[idx];

            // TODO: explicit cast by dropping 'const'. Needs a fix in future release!
            const bool result = execute_step((Step&)(step), context);

            switch( step.get_type() )
            {                
                case Step::type_try: // continue, set try_step for catch-clause
                    try_condition = true;
                    ptr_try_step = &step;
                    ++idx;
                    break;

                case Step::type_while: // result: false -> find end statement
                    while_condition = result;
                    idx = result ? idx + 1 : find_while_end(step, sequence, idx);
                    break;

                case Step::type_if: // result: false -> find end statement
                    if_condition = result;
                    idx = result ? idx + 1 : find_next_if_construct(step, sequence, idx);
                    break;

                case Step::type_elseif: // result: false -> find end statement
                    if ( if_condition )
                    {
                        idx = find_if_end(step, sequence, idx);
                        break;
                    }
                    if_condition = result;
                    idx = result ? idx + 1 : find_next_if_construct(step, sequence, idx);
                    break;

                case Step::type_else: // continue
                    idx = !if_condition ? idx + 1 : find_if_end(step, sequence, idx);
                    break;

                case Step::type_catch: // forward to end
                    idx = !try_condition ? idx + 1 : find_try_end(step, sequence, idx);
                    break;

                case Step::type_action: // continue
                    ++idx;
                    break;

                case Step::type_end:
                    idx = !while_condition ? idx + 1:  find_while(step, sequence, idx);
                    if (nullptr != ptr_try_step)
                        ptr_try_step = nullptr;
                    break;
            }
        }
        catch(const Error& e)
        {
            if (nullptr != ptr_try_step)
            {
                idx = find_try_catch(*ptr_try_step, sequence, idx);
                try_condition = false;
            }
            else
                throw Error{gul14::cat("Runtime Error [line ", idx, "]: ", e.what())};
        }

        if ( idx >= sequence.size() )
            break;
    }
}

}