/**
 * \file   execute_lua_script.cc
 * \author Lars Fröhlich, Marcus Walla
 * \date   Created on November 15, 2022
 * \brief  Implementation of execute_lua_script() and load_lua_script().
 *
 * \copyright Copyright 2022-2025 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <stdexcept>
#include <string_view>

#include <gul17/replace.h>

#include "taskolib/exceptions.h"
#include "taskolib/execute_lua_script.h"

namespace task {

namespace {

const std::string anchor = u8"\u2693";
constexpr std::string_view chunk_prefix{ u8"[string \"\u2693\"]:" };

std::string process_msg(std::string_view msg)
{
    // If C++ code is called by Lua and throws an exception that is not derived
    // from std::exception, the exception is not intercepted by the Sol
    // trampoline, but caught directly by Lua. Lua would expect this exception
    // to come from lua_error() and therefore looks for an error message on its
    // stack, which is not there. Depending on build type and Sol2 configuration,
    // this can generate a stack error message or not. We try to convert this into
    // a concise error message.
    if (msg.empty() ||
        msg == "lua: error: stack index 1, expected string, received function")
    {
        return "Unknown exception";
    }

    return gul17::replace(msg, chunk_prefix, "");
}

} // anonymous namespace

gul17::expected<sol::object, std::string>
execute_lua_script(sol::state& lua, sol::string_view script)
{
    try
    {
        auto protected_result = lua.safe_script(
            script, sol::script_pass_on_error, anchor);

        if (!protected_result.valid())
        {
            sol::error err = protected_result;
            return gul17::unexpected(process_msg(err.what()));
        }

        return static_cast<sol::object>(protected_result);
    }
    catch(const std::exception& e)
    {
        return gul17::unexpected(process_msg(e.what()));
    }
    catch(...)
    {
        return gul17::unexpected("Unknown C++ exception");
    }
}

gul17::expected<sol::load_result, std::string>
load_lua_script(sol::state& lua, sol::string_view script)
{
    sol::load_result result = lua.load(script, anchor);
    if (result.valid())
        return result;

    return gul17::unexpected(static_cast<sol::error>(result).what());
}

} // namespace task
