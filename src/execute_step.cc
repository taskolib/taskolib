/**
 * \file   execute_step.cc
 * \author Lars Froehlich
 * \date   Created on December 20, 2021
 * \brief  Implementation of the execute_step() function.
 *
 * \copyright Copyright 2021 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <gul14/cat.h>
#include "avtomat/Error.h"
#include "avtomat/execute_step.h"

#define SOL_PRINT_ERRORS 0
#include "sol/sol.hpp"

using gul14::cat;

namespace avto {


bool execute_step(Step& step, Context& context)
{
    sol::state lua;

    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table,
                       sol::lib::utf8);

    lua["_G"]["assert"] = nullptr;
    lua["_G"]["collectgarbage"] = nullptr;
    lua["_G"]["debug"] = nullptr;
    lua["_G"]["dofile"] = nullptr;
    lua["_G"]["load"] = nullptr;
    lua["_G"]["loadfile"] = nullptr;
    lua["_G"]["print"] = nullptr;
    lua["_G"]["require"] = nullptr;

    try
    {
        sol::optional<bool> result = lua.safe_script(
            step.get_script(), sol::script_default_on_error);

        if (result)
            return *result;
    }
    catch (const sol::error& e)
    {
        throw Error(cat("Error while executing script: ", e.what()));
    }

    return false;
}


} // namespace avto
