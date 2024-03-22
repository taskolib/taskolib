/**
 * \file   execute_lua_script.h
 * \author Lars Fröhlich
 * \date   Created on November 15, 2022
 * \brief  Declaration of execute_lua_script() and load_lua_script().
 *
 * \copyright Copyright 2022-2024 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOLIB_EXECUTE_LUA_SCRIPT_H_
#define TASKOLIB_EXECUTE_LUA_SCRIPT_H_

#include <string>

#include <gul14/expected.h>

#include "sol/sol.hpp"

namespace task {

/**
 * Execute a Lua script safely, intercepting all possible exceptions that may occur during
 * its execution.
 *
 * This function returns a gul14::expected object: If the Lua script finishes without
 * error, it contains a sol::object representing the return value of the script. If a Lua
 * error or C++ exception occurs, the returned object contains a string with an error message. The
 * error message is pre-processed to a certain degree: Unhelpful parts like the chunk name
 * of the script (`[string "..."]:`) are removed, and a few known special messages are
 * replaced by more readable explanations.
 */
gul14::expected<sol::object, std::string>
execute_lua_script(sol::state& lua, sol::string_view script);

/**
 * Load a Lua script into the given Lua state and check its syntax without running it.
 *
 * This function returns a gul14::expected object: If the syntax of the Lua script is OK,
 * it contains a sol::load_result proxy object that can be called to run the script or
 * cast to a sol::function/sol::protected_function. If the syntax check fails, a string
 * with an error message is returned instead. This error message is pre-processed to a
 * certain degree: Unhelpful parts like the chunk name of the script (`[string "..."]:`)
 * are removed, and a few known special messages are replaced by more readable
 * explanations.
 */
gul14::expected<sol::load_result, std::string>
load_lua_script(sol::state& lua, sol::string_view script);


} // namespace task

#endif
