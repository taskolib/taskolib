/**
 * \file   Sequence.cc
 * \author Marcus Walla
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

#include "taskomat/execute_step.h"
#include "taskomat/Sequence.h"

using gul14::cat;

namespace task
{

void Sequence::check_correctness_of_steps()
{
    E_IF if_block{ NO_IF };
    E_WHILE while_block{ NO_WHILE };
    E_TRY try_block{ NO_TRY };
    E_ACTION action_block{ NO_ACTION };
    E_END end_block{ NO_END };
    Step::Type type = Step::Type::type_end;
    int line{ 1 };
    for (const Step& step : this->steps_)
    {
        type = step.get_type();
        switch( type )
        {
            // try block ...
            case Step::Type::type_try:
                if ( TRY == try_block )
                    throw Error( cat( "Syntax error: called 'TRY' statement twice (line ", line, ")" ) );
                try_block = TRY;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;

            case Step::Type::type_catch:
                if ( TRY != try_block )
                    throw Error( cat( "Syntax error: 'CATCH' without 'TRY' (line ", line, ")" ) );
                else if ( NO_ACTION == action_block )
                    throw Error( cat( "Syntax error: try-catch block without 'ACTION' (previous line ", line-1, ")" ) );
                try_block = CATCH;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;

            // if block ...
            case Step::Type::type_if: // type_elif is not counted because it somewhere in between if and end.
                if ( IF == if_block )
                    throw Error( cat( "Syntax error: called 'IF' statement twice (line ", line, ")" ) );
                if_block = IF;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;

            case Step::Type::type_elseif:
                if ( IF != if_block && ELSE_IF != if_block )
                    throw Error( cat( "Syntax error: 'ELSE IF' without 'IF' (line ", line, ")" ) );
                else if ( NO_ACTION == action_block && NO_TRY == try_block && NO_WHILE == while_block )
                {
                    if ( NO_ACTION == action_block )
                        throw Error( cat( "Syntax error: 'IF' block without 'ACTION' (previous line ", line-1, ")" ) );
                    else if ( NO_TRY == try_block )
                        throw Error( cat( "Syntax error: 'IF' block without 'TRY' (previous line ", line-1, ")" ) );
                    else
                        throw Error( cat( "Syntax error: 'IF' block without 'WHILE' (previous line ", line-1, ")" ) );
                }
                if_block = ELSE_IF;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;

            case Step::Type::type_else:
                if ( IF != if_block && ELSE_IF != if_block )
                    throw Error( cat( "Syntax error: 'ELSE' without 'IF' or 'ELSE IF' (line ", line, ")" ) );
                else if ( NO_ACTION == action_block )
                    throw Error( cat( "Syntax error: 'IF' or 'ELSE IF' clause without 'ACTION' (previous line ", line-1, ")" ) );
                if_block = ELSE;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;

            // while block ...
            case Step::Type::type_while:
                if ( WHILE == while_block )
                    throw Error( cat( "Syntax error: called 'WHILE' statement twice (line ", line, ")" ) );
                while_block = WHILE;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;

            // action block ...
            case Step::Type::type_action:
                action_block = ACTION;
                end_block = NO_END;
                break;

            // end block ...
            case Step::Type::type_end:
                end_block = END;
                // order is important

                // check if none of the initial control element is set ...
                if ( NO_TRY == try_block && NO_IF == if_block && NO_WHILE == while_block )
                    throw Error( cat( "Syntax error: single 'END' block without initiale 'IF'/'TRY/'WHILE' (line ", line, ")" ) );

                // try ...
                if ( NO_TRY != try_block ) // 'try' clause
                {
                    if ( TRY == try_block && NO_ACTION == action_block )
                        throw Error( cat( "Syntax error: missing 'ACTION' in try-catch block (line ", line, ")" ) );
                    else if ( TRY == try_block ) // 'try' without 'catch'
                        throw Error( cat( "Syntax error: missing 'CATCH' in try-catch block (line ", line, ")" ) );
                    else
                        try_block = NO_TRY;
                }

                // if ...
                else if ( NO_IF != if_block )
                {
                    if ( WHILE != while_block && NO_ACTION == action_block )
                        throw Error( cat( "Syntax error: missing 'ACTION' in if-then clause (line ", line , ")" ) );
                    if_block = NO_IF;
                }

                // while ...
                else if ( NO_WHILE != while_block )
                {
                    // Remove? Because it can also be a while-end loop without any action!
                    if ( NO_ACTION == action_block )
                        throw Error( cat( "Syntax error: missing 'ACTION' in while block (line ", line , ")" ) );
                    while_block = NO_WHILE;
                }

                break;
        }
        ++line;
    }

    if ( !this->steps_.empty() && !( ACTION == action_block || END == end_block ) )
        throw Error( cat( "Syntax error: missing 'ACTION' or 'END' (line ", line, ")" ) );
}

void Sequence::execute( Context& context )
{
    check_correctness_of_steps();
    for( auto step: this->steps_ )
        execute_step( step, context );
}

void Sequence::indent() noexcept
{
    short level = 0;

    indentation_error_.clear();

    for (Step& step : steps_)
    {
        short step_level;

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

        step.set_indentation_level(step_level); // cannot throw because we check step_level

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

} // namespace task
