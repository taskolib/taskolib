/**
 * \file   lua_details.h
 * \author Lars Froehlich, Marcus Walla
 * \date   Created on June 15, 2022
 * \brief  Declaration of free functions dealing with LUA specifics.
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

#ifndef TASKOLIB_LUA_DETAILS_H_
#define TASKOLIB_LUA_DETAILS_H_

#include <chrono>
#include <functional>
#include <string>
#include <variant>

#include "sol/sol.hpp"
#include "taskolib/CommChannel.h"
#include "taskolib/Context.h"

// Check that the lua lib has been build with the expected types
static_assert(std::is_same<LUA_NUMBER, double>::value, "Unexpected Lua-internal floating point type");
static_assert(std::is_same<LUA_INTEGER, long long>::value, "Unexpected Lua-internal integer type");

namespace task {

// Abort the execution of the script by raising a LUA error with the given error message.
void abort_script_with_error(lua_State* lua_state, const std::string& msg);

// Check if immediate termination has been requested via the CommChannel. If so, raise a
// LUA error.
void check_immediate_termination_request(lua_State* lua_state);

// Check if the step timeout has expired and raise a LUA error if that is the case.
void check_script_timeout(lua_State* lua_state);

/**
 * Retrieve a pointer to the used CommChannel from the LUA registry.
 * The pointer can be null to indicate that no CommChannel is used.
 *
 * \exception Error is thrown if the appropriate registry key is not found.
 */
CommChannel* get_comm_channel_ptr_from_registry(lua_State* lua_state);

/**
 * Get the index of the currently executed Step from the LUA registry.
 *
 * \exception Error is thrown if the appropriate registry key is not found.
 */
StepIndex get_step_idx_from_registry(lua_State* lua_state);

// Return a time point in milliseconds since the epoch, calculated from a time point t0
// plus a duration dt. In case of overflow, the maximum representable time point is
// returned.
long long get_ms_since_epoch(TimePoint t0, std::chrono::milliseconds dt);

// A LUA hook that stops the execution of the script by raising a LUA error.
// This hook reinstalls itself so that it is called immediately if the execution should
// resume. This helps to break out of pcalls.
void hook_abort_with_error(lua_State* lua_state, lua_Debug*);

// Check if the step timeout has expired or if immediate termination has been requested
// via the comm channel. If so, raise a LUA error.
void hook_check_timeout_and_termination_request(lua_State* lua_state, lua_Debug*);

/**
 * Install implementations for some custom functions in the given LUA state.
 * \code
 * print() -- print a string on the (virtual) console; this function calls the
 *            print_function callback from the given context
 * sleep() -- wait for a given number of seconds
 * \endcode
 */
void install_custom_commands(sol::state& lua, const Context& context);

// Install hooks that check for timeouts and immediate termination requests while a LUA
// script is being executed. If one of both occurs, the script terminates with an error
// message that contains the abort marker.
void install_timeout_and_termination_request_hook(sol::state& lua, TimePoint now,
    std::chrono::milliseconds timeout, StepIndex step_idx,
    CommChannel* comm_channel);

/// Create a print() function for LUA that wraps a print callback from the Context.
std::function<void(sol::this_state, sol::variadic_args)>
make_print_fct(std::function<void(const std::string&, StepIndex, CommChannel*)> print_fct);

// Open a safe subset of the LUA standard libraries in the given LUA state.
//
// This opens the math, string, table, and UTF8 libraries. The base library is also
// opened, but the following potentially dangerous commands are removed:
// \code
// collectgarbage, debug, dofile, load, loadfile, print, require
// \endcode
void open_safe_library_subset(sol::state& lua);

// Print a string to the (virtual) console.
void print_fct(const std::string& text, sol::this_state sol);

// Pause execution for the specified time, observing timeouts and termination requests.
void sleep_fct(double seconds, sol::this_state sol);

} // namespace task

#endif
