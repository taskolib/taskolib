/**
 * \file   lua_details.h
 * \author Lars Froehlich, Marcus Walla
 * \date   Created on June 15, 2022
 * \brief  Declaration of free functions dealing with Lua specifics.
 *
 * \copyright Copyright 2022-2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOLIB_LUA_DETAILS_H_
#define TASKOLIB_LUA_DETAILS_H_

#include <chrono>
#include <functional>
#include <string>
#include <variant>

#include "sol/sol.hpp"
#include "taskolib/CommChannel.h"
#include "taskolib/Context.h"
#include "taskolib/TimeoutTrigger.h"

namespace task {

// Check that the lua lib has been build with the expected types
static_assert(std::is_same<LuaFloat, double>::value, "Unexpected Lua-internal floating point type");
static_assert(std::is_same<LuaInteger, long long>::value, "Unexpected Lua-internal integer type");

// Abort the execution of the script by raising a Lua error with the given error message.
void abort_script_with_error(lua_State* lua_state, const std::string& msg);

// Check if immediate termination has been requested via the CommChannel. If so, raise a
// Lua error.
void check_immediate_termination_request(lua_State* lua_state);

// Check if the step timeout has expired and raise a Lua error if that is the case.
void check_script_timeout(lua_State* lua_state);

/**
 * Retrieve a pointer to the used CommChannel from the Lua registry.
 * The pointer can be null to indicate that no CommChannel is used.
 *
 * \exception Error is thrown if the appropriate registry key is not found.
 */
CommChannel* get_comm_channel_ptr_from_registry(lua_State* lua_state);

/**
 * Retrieve a reference to the used Context from the Lua registry.
 *
 * \exception Error is thrown if the appropriate registry key is not found or if it
 *            contains a null pointer.
 */
const Context& get_context_from_registry(lua_State* lua_state);

/**
 * Get the index of the currently executed Step from the Lua registry.
 *
 * This function has three different outcomes:
 * - If the appropriate registry key is not found, an exception is thrown.
 * - If a negative step index is found in the registry, this indicates that no step index
 *   information is available. nullopt is returned.
 * - Otherwise, the step index is returned.
 *
 * \exception Error is thrown if the appropriate registry key is not found.
 */
OptionalStepIndex get_step_idx_from_registry(lua_State* lua_state);

// Return a time point in milliseconds since the epoch, calculated from a time point t0
// plus a duration dt. In case of overflow, the maximum representable time point is
// returned.
LuaInteger get_ms_since_epoch(TimePoint t0, std::chrono::milliseconds dt);

// A Lua hook that stops the execution of the script by raising a Lua error.
// This hook reinstalls itself so that it is called immediately if the execution should
// resume. This helps to break out of pcalls.
void hook_abort_with_error(lua_State* lua_state, lua_Debug*);

// Check if the step timeout has expired or if immediate termination has been requested
// via the comm channel. If so, raise a Lua error.
void hook_check_timeout_and_termination_request(lua_State* lua_state, lua_Debug*);

/**
 * Install implementations for some custom functions in the given Lua state.
 * \code
 * print() -- print a string on the (virtual) console; this function calls the
 *            print_function callback from the given context
 * sleep() -- wait for a given number of seconds
 * \endcode
 */
void install_custom_commands(sol::state& lua);

// Install hooks that check for timeouts and immediate termination requests while a Lua
// script is being executed. If one of both occurs, the script terminates with an error
// message that contains the abort marker.
void install_timeout_and_termination_request_hook(sol::state& lua, TimePoint now,
    std::chrono::milliseconds timeout, OptionalStepIndex step_idx, const Context& context,
    CommChannel* comm_channel, TimeoutTrigger* sequence_timeout);

// Open a safe subset of the Lua standard libraries in the given Lua state.
//
// This opens the math, string, table, and UTF8 libraries. The base library is also
// opened, but the following potentially dangerous commands are removed:
// \code
// collectgarbage, debug, dofile, load, loadfile, print, require
// \endcode
void open_safe_library_subset(sol::state& lua);

// An equivalent to Lua's print() function that stringifies and concatenates its arguments
// and finally sends a message of type Message::Type::output with the result.
void print_fct(sol::this_state, sol::variadic_args);

// Pause execution for the specified time, observing timeouts and termination requests.
void sleep_fct(double seconds, sol::this_state sol);

} // namespace task

#endif
