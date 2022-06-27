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
#include "sol/sol.hpp"
#include "taskomat/Error.h"
#include "taskomat/Step.h"
#include "lua_details.h"

using namespace std::literals;
using gul14::cat;

namespace {

template <typename>
inline constexpr bool always_false_v = false;

} // anonymous namespace


namespace task {

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

bool Step::execute(Context& context, CommChannel* comm, Message::IndexType index)
{
    const auto now = Clock::now();
    set_time_of_last_execution(now);

    send_message(comm, Message::Type::step_started, "Step started", now, index);

    sol::state lua;

    open_safe_library_subset(lua);
    install_custom_commands(lua, context);

    if (context.lua_init_function)
        context.lua_init_function(lua);

    install_timeout_and_termination_request_hook(lua, now, get_timeout(), index, comm);

    copy_used_variables_from_context_to_lua(context, lua);

    bool result = false;

    try
    {
        auto protected_result = lua.safe_script(get_script(),
                                                sol::script_default_on_error);

        if (!protected_result.valid())
        {
            sol::error err = protected_result;
            throw Error(cat("Error while executing script of step ", index, ": ",
                            err.what()));
        }

        copy_used_variables_from_lua_to_context(lua, context);

        sol::optional<bool> opt_result = protected_result;

        if (opt_result)
            result = *opt_result;
    }
    catch (const sol::error& e)
    {
        std::string msg = cat("Error while executing script of step ", index + 1, ": ",
                              e.what());

        send_message(comm, Message::Type::step_stopped_with_error, msg,
            Clock::now(), index);

        throw Error(msg);
    }

    send_message(comm, Message::Type::step_stopped,
        cat("Step ", index + 1, " finished (logical result: ", result ? "true" : "false",
            ')'),
        Clock::now(), index);

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

} // namespace task
