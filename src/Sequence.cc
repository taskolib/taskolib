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
    E_IF hasIf{ IF_NONE };
    E_WHILE hasWhile{ WHILE_NONE };
    E_TRY hasTry{ TRY_NONE };
    Step::Type type = Step::Type::type_end;
    int line{ 1 };
    for( auto step: this->steps )
    {
        type = step.get_type();
        switch( type )
        {
            case Step::Type::type_try:
                hasTry = HAS_TRY;
                break;
   
            case Step::Type::type_catch:
                if ( HAS_TRY != hasTry )
                    throw Error( gul14::cat( "Syntax error: 'CATCH' without 'TRY' (line ", line, ")" ) );
                hasTry = HAS_CATCH;
                break;
            
            case Step::Type::type_if: // type_elif is not counted because it somewhere in between if and end.
                hasIf = HAS_IF;
                break;

            case Step::Type::type_elseif:
                if ( HAS_IF != hasIf && HAS_ELSE_IF != hasIf )
                    throw Error( gul14::cat( "Syntax error: 'ELIF' without 'IF' (line ", line, ")" ) );
                hasIf = HAS_ELSE_IF;
                break;

            case Step::Type::type_else:
                if ( HAS_IF != hasIf && HAS_ELSE_IF != hasIf )
                    throw Error( gul14::cat( "Syntax error: 'ELSE' without 'IF' or 'ELSE IF' (line ", line, ")" ) );
                hasIf = HAS_ELSE;
                break;

            case Step::Type::type_while:
                hasWhile = HAS_WHILE;
                break;

            case Step::Type::type_action:
                // we do not care ... must be handled as free code
                break;

            case Step::Type::type_end:
                // order is important

                // check if none of the initial control element is set ...
                if ( TRY_NONE == hasTry && IF_NONE == hasIf && WHILE_NONE == hasWhile )
                    throw Error( gul14::cat( "Syntax error: single 'END' clause without initiate 'IF'/'TRY/'WHILE' (line ", line, ")" ) );

                // try ...
                if ( TRY_NONE != hasTry ) // some 'try' clause
                    if ( HAS_TRY == hasTry ) // 'try' without 'catch'
                        throw Error( gul14::cat( "Syntax error: missing 'CATCH' in try-catch clause (line ", line, ")" ) );
                    else
                        hasTry = TRY_NONE;
                
                // if ...
                else if ( IF_NONE != hasIf )
                    hasIf = IF_NONE;

                // while ...
                else if ( WHILE_NONE != hasWhile )
                    hasWhile = WHILE_NONE;

                break;
        }
        ++line;
    }

    return true;
}

void Sequence::execute( Context& context )
{
    check_correctness_of_steps();
    for( auto step: this->steps )
        execute_step( step, context );
}

}