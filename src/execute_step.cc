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

#include <limits>
#include <gul14/cat.h>
#include "avtomat/Error.h"
#include "avtomat/execute_step.h"

#define SOL_PRINT_ERRORS 0
#include "sol/sol.hpp"

using gul14::cat;


namespace avto {


namespace {

void check_script_timeout(lua_State* lua_state, lua_Debug*) noexcept
{
    sol::state_view lua(lua_state);

    sol::optional<long long> timeout_ms = lua["AVTOMAT_TIMEOUT_MS_SINCE_EPOCH"];

    if (not timeout_ms.has_value())
    {
        luaL_error(lua_state, "Timeout time point not found in LUA environment "
            "(AVTOMAT_TIMEOUT_MS_SINCE_EPOCH)");
    }
    else
    {
        using std::chrono::milliseconds;
        using std::chrono::round;

        const long long now_ms =
            round<milliseconds>(Clock::now().time_since_epoch()).count();

        if (now_ms > *timeout_ms) // TODO remove debug output
        {
            double seconds = lua["AVTOMAT_TIMEOUT_S"].get_or(-1.0);
            luaL_error(lua_state, cat("Timeout: Script took more than ", seconds,
                                      " s to run").c_str());
        }
    }
}

// Return a time point in milliseconds since the epoch, calculated from the curent time
// plus the given duration. In case of overflow, the maximum representable time point is
// returned.
long long get_future_time_point_in_ms(std::chrono::milliseconds duration_from_now)
{
    using std::chrono::milliseconds;
    using std::chrono::round;

    const long long t0_ms = round<milliseconds>(Clock::now().time_since_epoch()).count();
    const long long max_dt = std::numeric_limits<long long>::max() - t0_ms;
    const long long dt_ms = duration_from_now.count();

    if (dt_ms < max_dt)
        return t0_ms + dt_ms;
    else
        return std::numeric_limits<long long>::max();
}

} // anonymous namespace


bool execute_step(Step& step, Context& context)
{
    sol::state lua;

    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table,
                       sol::lib::utf8);

    auto globals = lua.globals();
    globals["assert"] = sol::nil;
    globals["collectgarbage"] = sol::nil;
    globals["debug"] = sol::nil;
    globals["dofile"] = sol::nil;
    globals["load"] = sol::nil;
    globals["loadfile"] = sol::nil;
    globals["print"] = sol::nil;
    globals["require"] = sol::nil;

    globals["AVTOMAT_TIMEOUT_S"] =
        std::chrono::duration<double>(step.get_timeout()).count();
    globals["AVTOMAT_TIMEOUT_MS_SINCE_EPOCH"] =
        get_future_time_point_in_ms(step.get_timeout());

    // Install a hook that is called after every 10 LUA instructions
    lua_sethook(lua.lua_state(), check_script_timeout, LUA_MASKCOUNT, 10);

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
