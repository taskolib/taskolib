/**
 * \file   Step.cc
 * \author Lars Froehlich, Marcus Walla
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

#include <gul14/cat.h>
#include <gul14/finalizer.h>
#include <gul14/trim.h>

#include "internals.h"
#include "lua_details.h"
#include "sol/sol.hpp"
#include "taskolib/exceptions.h"
#include "taskolib/execute_lua_script.h"
#include "taskolib/Step.h"

using namespace std::literals;
using gul14::cat;

namespace {

template <typename>
[[maybe_unused]] inline constexpr bool always_false_v = false;

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
                              std::is_same_v<T, std::string> or std::is_same_v<T, bool>)
                {
                    lua[varname_str] = value;
                }
                else if constexpr (std::is_same_v<T, sol::lua_nil_t>) {
                    lua[varname_str] = sol::lua_nil_t{ };
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
            case sol::type::boolean:
                context.variables[varname] = VariableValue{ var.as<bool>() };
                break;
            case sol::type::lua_nil:
                context.variables.erase(varname);
                break;
            default:
                break;
        }
    }
}

bool Step::execute_impl(Context& context, CommChannel* comm, StepIndex index)
{
    sol::state lua;

    open_safe_library_subset(lua);
    install_custom_commands(lua, context);

    if (context.lua_init_function)
        context.lua_init_function(lua);

    install_timeout_and_termination_request_hook(lua, Clock::now(), get_timeout(), index,
                                                 comm);

    copy_used_variables_from_context_to_lua(context, lua);
    const auto result_or_error = execute_lua_script(lua, get_script());
    copy_used_variables_from_lua_to_context(lua, context);

    if (std::holds_alternative<std::string>(result_or_error))
        throw Error(std::get<std::string>(result_or_error));

    const auto& obj = std::get<sol::object>(result_or_error);

    if (requires_bool_return_value(get_type()))
    {
        if (not obj.is<bool>())
        {
            throw Error(cat("A script in a ", to_string(get_type()),
                " step must return a boolean value (true or false)."));
        }

        return obj.as<bool>();
    }
    else
    {
        if (obj != sol::nil)
        {
            throw Error(cat("A script in a ", to_string(get_type()),
                " step may not return any value."));
        }

        return false;
    }
}

bool Step::execute(Context& context, CommChannel* comm, StepIndex index)
{
    const auto now = Clock::now();
    const auto set_is_running_to_false_after_execution =
        gul14::finally([this]() { set_running(false); });

    set_time_of_last_execution(now);
    set_running(true);
    send_message(comm, Message::Type::step_started, "Step started", now, index);

    try
    {
        const bool result = execute_impl(context, comm, index);

        send_message(comm, Message::Type::step_stopped,
            requires_bool_return_value(get_type())
                ? cat("Step finished (logical result: ", result ? "true" : "false", ')')
                : "Step finished"s,
            Clock::now(), index);

        return result;
    }
    catch(const std::exception& e)
    {
        auto [msg, _] = remove_abort_markers(e.what());
        send_message(comm, Message::Type::step_stopped_with_error, msg, Clock::now(),
                     index);
        throw ErrorAtIndex(e.what(), index);
    }
}

Step& Step::set_disabled(bool disable)
{
    is_disabled_ = disable;
    set_time_of_last_modification(Clock::now());
    return *this;
}

Step& Step::set_indentation_level(short level)
{
    if (level < 0)
        throw Error(cat("Cannot set negative indentation level (", level, ')'));

    if (level > max_indentation_level)
    {
        throw Error(cat("Indentation level exceeds maximum (", level, " > ",
                        max_indentation_level, ')'));
    }

    indentation_level_ = level;
    return *this;
}

Step& Step::set_label(const std::string& label)
{
    label_ = gul14::trim(label);
    set_time_of_last_modification(Clock::now());
    return *this;
}

Step& Step::set_running(bool is_running)
{
    is_running_ = is_running;
    return *this;
}

Step& Step::set_script(const std::string& script)
{
    script_ = script;
    set_time_of_last_modification(Clock::now());
    return *this;
}

Step& Step::set_time_of_last_execution(TimePoint t)
{
    time_of_last_execution_ = t;
    return *this;
}

Step& Step::set_time_of_last_modification(TimePoint t)
{
    time_of_last_modification_ = t;
    return *this;
}

Step& Step::set_timeout(std::chrono::milliseconds timeout)
{
    if (timeout < 0s)
        timeout_ = 0s;
    else
        timeout_ = timeout;
    return *this;
}

Step& Step::set_type(Type type)
{
    type_ = type;
    set_time_of_last_modification(Clock::now());
    return *this;
}

Step& Step::set_used_context_variable_names(const VariableNames& used_context_variable_names)
{
    used_context_variable_names_ = used_context_variable_names;
    return *this;
}

Step& Step::set_used_context_variable_names(VariableNames&& used_context_variable_names)
{
    used_context_variable_names_ = std::move(used_context_variable_names);
    return *this;
}


//
// Free functions
//

std::string to_string(Step::Type type)
{
    switch(type)
    {
        case Step::type_action: return "action";
        case Step::type_if: return "if";
        case Step::type_else: return "else";
        case Step::type_elseif: return "elseif";
        case Step::type_end: return "end";
        case Step::type_while: return "while";
        case Step::type_try: return "try";
        case Step::type_catch: return "catch";
    }

    return "unknown";
}

bool requires_bool_return_value(Step::Type step_type) noexcept
{
    switch (step_type)
    {
        case Step::type_action:
        case Step::type_catch:
        case Step::type_else:
        case Step::type_end:
        case Step::type_try:
            return false;
        case Step::type_elseif:
        case Step::type_if:
        case Step::type_while:
            return true;
    }

    // Never reached as long as the above switch statement covers all cases
    return false;
}

} // namespace task
