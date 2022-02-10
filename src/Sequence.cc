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

#include "avtomat/Sequence.h"
#include "avtomat/execute_step.h"

namespace avto
{

bool Sequence::check_correctness_of_steps()
{
    E_IF if_block{ NO_IF };
    E_WHILE while_block{ NO_WHILE };
    E_TRY try_block{ NO_TRY };
    E_ACTION action_block{ NO_ACTION };
    E_END end_block{ NO_END };
    Step::Type type = Step::Type::type_end;
    int line{ 1 };
    for( auto step: this->steps_ )
    {
        type = step.get_type();
        switch( type )
        {
            // try block ...
            case Step::Type::type_try:
                if ( TRY == try_block )
                    throw Error( gul14::cat( "Syntax error: called 'TRY' statement twice (line ", line, ")" ) );
                try_block = TRY;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;
   
            case Step::Type::type_catch:
                if ( TRY != try_block )
                    throw Error( gul14::cat( "Syntax error: 'CATCH' without 'TRY' (line ", line, ")" ) );
                else if ( NO_ACTION == action_block )
                    throw Error( gul14::cat( "Syntax error: try-catch block without 'ACTION' (previous line ", line-1, ")" ) );
                try_block = CATCH;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;
            
            // if block ...
            case Step::Type::type_if: // type_elif is not counted because it somewhere in between if and end.
                if ( IF == if_block )
                    throw Error( gul14::cat( "Syntax error: called 'IF' statement twice (line ", line, ")" ) );
                if_block = IF;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;

            case Step::Type::type_elseif:
                if ( IF != if_block && ELSE_IF != if_block )
                    throw Error( gul14::cat( "Syntax error: 'ELSE IF' without 'IF' (line ", line, ")" ) );
                else if ( NO_ACTION == action_block && NO_TRY == try_block && NO_WHILE == while_block )
                {
                    if ( NO_ACTION == action_block )
                        throw Error( gul14::cat( "Syntax error: 'IF' block without 'ACTION' (previous line ", line-1, ")" ) );
                    else if ( NO_TRY == try_block )
                        throw Error( gul14::cat( "Syntax error: 'IF' block without 'TRY' (previous line ", line-1, ")" ) );
                    else
                        throw Error( gul14::cat( "Syntax error: 'IF' block without 'WHILE' (previous line ", line-1, ")" ) );
                }
                if_block = ELSE_IF;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;

            case Step::Type::type_else:
                if ( IF != if_block && ELSE_IF != if_block )
                    throw Error( gul14::cat( "Syntax error: 'ELSE' without 'IF' or 'ELSE IF' (line ", line, ")" ) );
                else if ( NO_ACTION == action_block )
                    throw Error( gul14::cat( "Syntax error: 'IF' or 'ELSE IF' clause without 'ACTION' (previous line ", line-1, ")" ) );
                if_block = ELSE;
                action_block = NO_ACTION;
                end_block = NO_END;
                break;

            // while block ...
            case Step::Type::type_while:
                if ( WHILE == while_block )
                    throw Error( gul14::cat( "Syntax error: called 'WHILE' statement twice (line ", line, ")" ) );
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
                    throw Error( gul14::cat( "Syntax error: single 'END' block without initiale 'IF'/'TRY/'WHILE' (line ", line, ")" ) );

                // try ...
                if ( NO_TRY != try_block ) // 'try' clause
                {
                    if ( TRY == try_block && NO_ACTION == action_block )
                        throw Error( gul14::cat( "Syntax error: missing 'ACTION' in try-catch block (line ", line, ")" ) );
                    else if ( TRY == try_block ) // 'try' without 'catch'
                        throw Error( gul14::cat( "Syntax error: missing 'CATCH' in try-catch block (line ", line, ")" ) );
                    else
                        try_block = NO_TRY;
                }

                // if ...
                else if ( NO_IF != if_block )
                {
                    if ( WHILE != while_block && NO_ACTION == action_block )
                        throw Error( gul14::cat( "Syntax error: missing 'ACTION' in if-then clause (line ", line , ")" ) );
                    if_block = NO_IF;
                }
                
                // while ...
                else if ( NO_WHILE != while_block )
                {
                    // Remove? Because it can also be a while-end loop without any action!
                    if ( NO_ACTION == action_block )
                        throw Error( gul14::cat( "Syntax error: missing 'ACTION' in while block (line ", line , ")" ) );
                    while_block = NO_WHILE;
                }

                break;
        }
        ++line;
    }

    if ( !this->steps_.empty() && !( ACTION == action_block || END == end_block ) )
        throw Error( gul14::cat( "Syntax error: missing 'ACTION' or 'END' (line ", line, ")" ) );

    return true;
}

void Sequence::execute( Context& context )
{
    check_correctness_of_steps();
    for( auto step: this->steps_ )
        execute_step( step, context );
}

}