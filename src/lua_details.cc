/**
 * \file   lua_details.cc
 * \author Lars Froehlich, Marcus Walla
 * \date   Created on June 15, 2022
 * \brief  Implementation of free functions dealing with LUA specifics.
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

#include <limits>

#include <gul14/gul.h>

#include "internals.h"
#include "lua_details.h"
#include "taskolib/CommChannel.h"
#include "taskolib/exceptions.h"

using gul14::cat;

namespace {

static const char step_index_key[] =
    "TASKOLIB_STP_INDEX";
static const char step_timeout_ms_since_epoch_key[] =
    "TASKOLIB_STP_TO_MS";
static const char step_timeout_s_key[] =
    "TASKOLIB_STP_TO_S";
static const char comm_channel_key[] =
    "TASKOLIB_COMM_CH";
static const char abort_error_message_key[] =
    "TASKOLIB_AB_END";

} // anonymous namespace


namespace task {

void abort_script_with_error(lua_State* lua_state, const std::string& msg)
{
    sol::state_view lua(lua_state);
    auto registry = lua.registry();

    // The [ABORT] marker ("ABORT" surrounded by two Unicode stop signs) marks this error
    // as one that can not be caught by CATCH blocks.
    // We store the error message in the registry...
    registry[abort_error_message_key] = gul14::cat(abort_marker, msg, abort_marker);

    // ... and call the abort hook which raises a LUA error with the message from the
    // registry.
    hook_abort_with_error(lua_state, nullptr);
}

void check_immediate_termination_request(lua_State* lua_state)
{
    try
    {
        CommChannel* comm = get_comm_channel_ptr_from_registry(lua_state);

        if (comm)
        {
            if (comm->immediate_termination_requested_)
                abort_script_with_error(lua_state, "Stop on user request");
        }
    }
    catch (const Error& e)
    {
        abort_script_with_error(lua_state, e.what());
    }
}

void check_script_timeout(lua_State* lua_state)
{
    sol::state_view lua(lua_state);

    const auto registry = lua.registry();

    sol::optional<long long> timeout_ms = registry[step_timeout_ms_since_epoch_key];

    if (not timeout_ms.has_value())
    {
        abort_script_with_error(lua_state, cat("Timeout time point not found in LUA "
            "registry (", step_timeout_ms_since_epoch_key, ')'));
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
            abort_script_with_error(lua_state,
                cat("Timeout: Script took more than ", seconds, " s to run"));
        }
    }
}

CommChannel* get_comm_channel_ptr_from_registry(lua_State* lua_state)
{
    sol::state_view lua(lua_state);
    const auto registry = lua.registry();

    sol::optional<CommChannel*> opt_comm_channel_ptr = registry[comm_channel_key];
    if (not opt_comm_channel_ptr.has_value())
        throw Error(cat(comm_channel_key, " not found in LUA registry"));

    return *opt_comm_channel_ptr;
}

StepIndex get_step_idx_from_registry(lua_State* lua_state)
{
    sol::state_view lua(lua_state);
    const auto registry = lua.registry();

    sol::optional<StepIndex> opt_step_idx = registry[step_index_key];
    if (not opt_step_idx.has_value())
        throw Error(cat(step_index_key, " not found in LUA registry"));

    return *opt_step_idx;
}

long long get_ms_since_epoch(TimePoint t0, std::chrono::milliseconds dt)
{
    using std::chrono::milliseconds;
    using std::chrono::round;

    static_assert(std::numeric_limits<long long>::max()
                  >= std::numeric_limits<TimePoint::rep>::max());
    static_assert(std::numeric_limits<long long>::max()
                  >= std::numeric_limits<milliseconds::rep>::max());

    const long long t0_ms = round<milliseconds>(t0.time_since_epoch()).count();
    const long long max_dt = std::numeric_limits<long long>::max() - t0_ms;
    const long long dt_ms = dt.count();

    if (dt_ms < max_dt)
        return t0_ms + dt_ms;
    else
        return std::numeric_limits<long long>::max();
}

void hook_check_timeout_and_termination_request(lua_State* lua_state, lua_Debug*)
{
    // If necessary, these functions raise LUA errors to terminate the execution of the
    // script. As we use a C++ compiled LUA, the error is thrown as an exception that is
    // caught by a LUA-internal handler.
    check_immediate_termination_request(lua_state);
    check_script_timeout(lua_state);
}

void hook_abort_with_error(lua_State* lua_state, lua_Debug*)
{
    sol::state_view lua(lua_state);
    const auto registry = lua.registry();
    const std::string err_msg = registry[abort_error_message_key];

    lua_sethook(lua_state, hook_abort_with_error, LUA_MASKLINE, 0);
    luaL_error(lua_state, err_msg.c_str());
}

void install_custom_commands(sol::state& lua, const Context& context)
{
    auto globals = lua.globals();
    globals["print"] = make_print_fct(context.print_function);
    globals["sleep"] = sleep_fct;
    globals["terminate_sequence"] =
        [](sol::this_state lua){ abort_script_with_error(lua, ""); };
}

void install_timeout_and_termination_request_hook(sol::state& lua, TimePoint now,
    std::chrono::milliseconds timeout, StepIndex step_idx, CommChannel* comm_channel)
{
    auto registry = lua.registry();
    registry[step_timeout_s_key] = std::chrono::duration<double>(timeout).count();
    registry[step_timeout_ms_since_epoch_key] = get_ms_since_epoch(now, timeout);
    registry[step_index_key] = step_idx;
    registry[comm_channel_key] = comm_channel;

    // Install a hook that is called after every 100 LUA instructions
    lua_sethook(lua, hook_check_timeout_and_termination_request, LUA_MASKCOUNT, 100);
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

std::function<void(sol::this_state, sol::variadic_args)>
make_print_fct(std::function<void(const std::string&, StepIndex, CommChannel*)> print_fct)
{
    return
        [print_fct = std::move(print_fct)](sol::this_state sol, sol::variadic_args va)
        {
            sol::state_view state{ sol };
            auto tostring{ state["tostring"] };

            try
            {
                gul14::SmallVector<std::string, 8> stringified_args;
                stringified_args.reserve(va.size());

                for (auto v : va)
                    stringified_args.push_back(tostring(v));

                print_fct(gul14::join(stringified_args, "\t") + "\n",
                          get_step_idx_from_registry(sol),
                          get_comm_channel_ptr_from_registry(sol));
            }
            catch (const Error& e)
            {
                abort_script_with_error(sol, e.what());
            }
        };
}

void sleep_fct(double seconds, sol::this_state sol)
{
    auto t0 = gul14::tic();
    while (gul14::toc(t0) < seconds)
    {
        hook_check_timeout_and_termination_request(sol, nullptr);
        double sec = gul14::clamp(seconds - gul14::toc(t0), 0.0, 0.01);
        gul14::sleep(sec);
    }
}

} // namespace task
