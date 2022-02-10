/**
 * \file   execute_step.cc
 * \author Lars Froehlich
 * \date   Created on December 20, 2021
 * \brief  Implementation of the execute_step() function.
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

#include <limits>
#include <gul14/cat.h>
#include "avtomat/Error.h"
#include "avtomat/execute_step.h"
#include "sol/sol.hpp"

using gul14::cat;


namespace avto {


namespace {

static const char step_timeout_ms_since_epoch_key[] =
    "TASKOMAT_STEP_TIMEOUT_MS_SINCE_EPOCH";
static const char step_timeout_s_key[] =
    "TASKOMAT_STEP_TIMEOUT_S";

template <typename>
inline constexpr bool always_false_v = false;

// Check if the timeout from the LUA state has been reached and raise a LUA error if so.
// As we use a C++ compiled LUA, the error is thrown as an exception that is caught by a
// LUA-internal handler.
void check_script_timeout(lua_State* lua_state, lua_Debug*)
{
    sol::state_view lua(lua_state);

    const auto registry = lua.registry();
    sol::optional<long long> timeout_ms = registry[step_timeout_ms_since_epoch_key];

    if (not timeout_ms.has_value())
    {
        // Throw an error and repeat that when returning to LUA execution (helps break out
        // of pcalls)
        lua_sethook(lua_state, check_script_timeout, LUA_MASKLINE, 0);
        luaL_error(lua_state, "Timeout time point not found in LUA registry (%s)",
            step_timeout_ms_since_epoch_key);
    }
    else
    {
        using std::chrono::milliseconds;
        using std::chrono::round;

        const long long now_ms =
            round<milliseconds>(Clock::now().time_since_epoch()).count();

        if (now_ms > *timeout_ms)
        {
            double seconds = registry[step_timeout_s_key].get_or(-1.0);
            // Throw an error and repeat that when returning to LUA execution (helps break
            // out of pcalls)
            lua_sethook(lua_state, check_script_timeout, LUA_MASKLINE, 0);
            luaL_error(lua_state, cat("Timeout: Script took more than ", seconds,
                                      " s to run").c_str());
        }
    }
}

// Export the variables listed in the step from the LUA state into the context.
void export_variables_to_context(const Step& step, Context& context, sol::state& lua)
{
    const VariableNames export_varnames = step.get_exported_variable_names();

    for (const VariableName& varname : export_varnames)
    {
        sol::object var = lua.get<sol::object>(varname.string());
        switch (var.get_type())
        {
            case sol::type::number:
                // For this check to work, SOL_SAFE_NUMERICS needs to be set to 1
                if (var.is<long long>())
                    context[varname] = VariableValue{ var.as<long long>() };
                else
                    context[varname] = VariableValue{ var.as<double>() };
                break;
            case sol::type::string:
                context[varname] = VariableValue{ var.as<std::string>() };
                break;
            default:
                break;
        }
    }
}

// Return a time point in milliseconds since the epoch, calculated from a time point t0
// plus a duration dt. In case of overflow, the maximum representable time point is
// returned.
long long get_ms_since_epoch(Timestamp t0, std::chrono::milliseconds dt)
{
    using std::chrono::milliseconds;
    using std::chrono::round;

    const long long t0_ms = round<milliseconds>(t0.time_since_epoch()).count();
    const long long max_dt = std::numeric_limits<long long>::max() - t0_ms;
    const long long dt_ms = dt.count();

    if (dt_ms < max_dt)
        return t0_ms + dt_ms;
    else
        return std::numeric_limits<long long>::max();
}

// Import the variables listed in the step from the context into the LUA state.
void import_variables_from_context(const Step& step, const Context& context,
    sol::state& lua)
{
    VariableNames import_varnames = step.get_imported_variable_names();

    for (const VariableName& varname : import_varnames)
    {
        auto it = context.find(varname);
        if (it == context.end())
            continue;

        std::visit(
            [&lua, varname_str = varname.string()](auto&& value)
            {
                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, double> or std::is_same_v<T, long long> or
                              std::is_same_v<T, std::string>)
                {
                    lua[varname_str] = value;
                }
                else
                {
                    static_assert(always_false_v<T>, "Unhandled type in variable import");
                }
            },
            it->second);
    }
}

void install_timeout_hook(sol::state& lua, Timestamp now,
                          std::chrono::milliseconds timeout)
{
    auto registry = lua.registry();
    registry[step_timeout_s_key] = std::chrono::duration<double>(timeout).count();
    registry[step_timeout_ms_since_epoch_key] = get_ms_since_epoch(now, timeout);

    // Install a hook that is called after every 100 LUA instructions
    lua_sethook(lua.lua_state(), check_script_timeout, LUA_MASKCOUNT, 100);
}

void open_safe_library_subset(sol::state& lua)
{
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table,
                       sol::lib::utf8);

    auto globals = lua.globals();
    globals["collectgarbage"] = sol::nil;
    globals["debug"] = sol::nil;
    globals["dofile"] = sol::nil;
    globals["load"] = sol::nil;
    globals["loadfile"] = sol::nil;
    globals["print"] = sol::nil;
    globals["require"] = sol::nil;
}

} // anonymous namespace


bool execute_step(Step& step, Context& context)
{
    const auto now = Clock::now();
    step.set_time_of_last_execution(now);

    sol::state lua;

    open_safe_library_subset(lua);
    install_timeout_hook(lua, now, step.get_timeout());

    import_variables_from_context(step, context, lua);

    try
    {
        sol::optional<bool> result = lua.safe_script(
            step.get_script(), sol::script_default_on_error);

        export_variables_to_context(step, context, lua);

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
