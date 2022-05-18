/**
 * \file   Step.cc
 * \author Lars Froehlich
 * \date   Created on December 7, 2021
 * \brief  Implementation of the Step class.
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

#include <ctime>
#include <iomanip> // for std::put_time
#include <gul14/cat.h>
#include "taskomat/Error.h"
#include "taskomat/Step.h"

using namespace std::literals;
using gul14::cat;

namespace task {


// Anonymous namespace for implementation details
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

// Return a time point in milliseconds since the epoch, calculated from a time point t0
// plus a duration dt. In case of overflow, the maximum representable time point is
// returned.
long long get_ms_since_epoch(TimePoint t0, std::chrono::milliseconds dt)
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

void install_timeout_hook(sol::state& lua, TimePoint now,
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


void Step::copy_used_variables_from_context_to_lua(const Context& context, sol::state& lua)
{
    VariableNames import_varnames = get_used_context_variable_names();

    for (const VariableName& varname : import_varnames)
    {
        auto it = context.variables.find(varname);
        if (it == context.variables.end())
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

void Step::copy_used_variables_from_lua_to_context(const sol::state& lua, Context& context)
{
    const VariableNames export_varnames = get_used_context_variable_names();

    for (const VariableName& varname : export_varnames)
    {
        sol::object var = lua.get<sol::object>(varname.string());
        switch (var.get_type())
        {
            case sol::type::number:
                // For this check to work, SOL_SAFE_NUMERICS needs to be set to 1
                if (var.is<long long>())
                    context.variables[varname] = VariableValue{ var.as<long long>() };
                else
                    context.variables[varname] = VariableValue{ var.as<double>() };
                break;
            case sol::type::string:
                context.variables[varname] = VariableValue{ var.as<std::string>() };
                break;
            default:
                break;
        }
    }
}

bool Step::execute(Context& context, MessageQueue* queue)
{
    const auto now = Clock::now();
    set_time_of_last_execution(now);

    send_message(queue, Message::Type::step_started, "Step started", now);

    sol::state lua;

    open_safe_library_subset(lua);

    if (context.lua_init_function)
        context.lua_init_function(lua);

    install_timeout_hook(lua, now, get_timeout());

    copy_used_variables_from_context_to_lua(context, lua);

    bool result = false;

    try
    {
        auto protected_result = lua.safe_script(get_script(),
                                                sol::script_default_on_error);

        if (!protected_result.valid())
        {
            sol::error err = protected_result;
            throw Error(cat("Error while executing script: ", err.what()));
        }

        copy_used_variables_from_lua_to_context(lua, context);

        sol::optional<bool> result = protected_result;

        if (result)
            return *result;
    }
    catch (const sol::error& e)
    {
        std::string msg = cat("Error while executing script: ", e.what());

        send_message(queue, Message::Type::step_stopped_with_error, msg,
            Clock::now());

        throw Error(msg);
    }

    send_message(queue, Message::Type::step_stopped,
        cat("Step finished (logical result: ", result ? "true" : "false", ')'),
        Clock::now());

    return result;
}

void Step::set_used_context_variable_names(const VariableNames& used_context_variable_names)
{
    used_context_variable_names_ = used_context_variable_names;
}

void Step::set_used_context_variable_names(VariableNames&& used_context_variable_names)
{
    used_context_variable_names_ = std::move(used_context_variable_names);
}

void Step::set_indentation_level(short level)
{
    if (level < 0)
        throw Error(cat("Cannot set negative indentation level (", level, ')'));

    if (level > max_indentation_level)
    {
        throw Error(cat("Indentation level exceeds maximum (", level, " > ",
                        max_indentation_level, ')'));
    }

    indentation_level_ = level;
}

void Step::set_label(const std::string& label)
{
    label_ = label;
    set_time_of_last_modification(Clock::now());
}

void Step::set_script(const std::string& script)
{
    script_ = script;
    set_time_of_last_modification(Clock::now());
}

void Step::set_timeout(std::chrono::milliseconds timeout)
{
    if (timeout < 0s)
        timeout_ = 0s;
    else
        timeout_ = timeout;
}

void Step::set_type(Type type)
{
    type_ = type;
    set_time_of_last_modification(Clock::now());
}

std::ostream& operator<<(std::ostream& stream, const Step& step)
{
    // TODO: need to fetch taskomat, lua, and sol2 version
    //stream << "-- Taskomat version: " << TASKOMAT_VERSION_STRING << ", Lua version: "
    //    << LUA_VERSION_MAJOR << ", Sol2 version: " << SOL_VERSION_STRING << '\n';

    stream << "-- type: " << type_to_string(step.get_type()) << '\n';
    stream << "-- label: \"" << step.get_label() << "\"\n";
    stream << "-- use context variable names: [";
    for(auto variable: step.get_used_context_variable_names())
    {
        // TODO: need to fix separator for last entity        
        stream << "\"" << variable.string() << "\", ";
    }
    stream << "]\n";

    const std::time_t last_modification = 
        std::chrono::system_clock::to_time_t(step.get_time_of_last_modification());
    stream << "-- time of last modification: \"" << std::put_time(std::localtime(
        &last_modification), "%F %T") << "\"\n";
    
    const std::time_t last_execution =
        std::chrono::system_clock::to_time_t(step.get_time_of_last_execution());
    stream << "-- time of last execution: \"" << std::put_time(
        std::localtime(&last_execution), "%F %T") << "\"\n";
    
    stream << "-- timeout: ";
    if ( step.get_timeout() == Step::infinite_timeout )
        stream << "infinite\n";
    else
        stream << std::to_string(step.get_timeout().count()) << "ms\n";

    stream << '\n' << step.get_script() << '\n';

    return stream;
}

} // namespace task
