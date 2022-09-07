/**
 * \file   Step.h
 * \author Lars Froehlich, Marcus Walla
 * \date   Created on November 26, 2021
 * \brief  Declaration of the Step class.
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

#ifndef TASKOMAT_STEP_H_
#define TASKOMAT_STEP_H_

#include <chrono>
#include <set>
#include <string>
#include "taskomat/CommChannel.h"
#include "taskomat/Context.h"
#include "taskomat/time_types.h"
#include "taskomat/VariableName.h"

namespace task {

using VariableNames = std::set<VariableName>;

/**
 * A step is the main building block of a sequence.
 *
 * Each step consists of a label, a script, and various other attributes.
 * Steps can be of different types (see Step::Type) - for instance, "action" steps hold a
 * script that can simply be executed, "if" steps hold a script that can be evaluated to
 * determine if a condition is fulfilled, "end" steps mark the closing of a block in a
 * sequence.
 */
class Step
{
public:
    /// An enum for differentiating the different types of step.
    enum Type
    {
        type_action, type_if, type_else, type_elseif, type_end, type_while, type_try,
        type_catch
    };

    /// A constant to use for "infinite" timeout durations
    static constexpr std::chrono::milliseconds
        infinite_timeout{ std::chrono::milliseconds::max() };

    /// Maximum allowed level of indentation (or nesting of steps)
    static constexpr short max_indentation_level{ 20 };


    /// Construct a Step of a certain type.
    explicit Step(Type type = type_action)
        : type_{ type }
    {}

    /**
     * Execute the step script within the given context, sending status information to a
     * message queue.
     *
     * This function performs the following steps:
     * 1. A fresh script runtime environment is prepared and safe library components are
     *    loaded into it.
     * 2. The lua_init_function from the context is run if it is defined (non-null).
     * 3. Selected variables are imported from the context into the runtime environment.
     * 4. The script from the step is loaded into the runtime environment and executed.
     * 5. Selected variables are exported from the runtime environment back into the
     *    context.
     *
     * \param context       The context to be used for executing the step
     * \param comm_channel  Pointer to a communication channel; If this is null, messaging
     *                      is disabled and there is no way to stop the execution.
     *                      Otherwise, termination requests are honored and the queue
     *                      receives the following messages:
     *                      - A message of type step_started when the step is started
     *                      - A message of type step_stopped when the step has finished
     *                        successfully
     *                      - A message of type step_stopped_with_error when the step has
     *                        been stopped due to an error condition
     * \param index         Index of the step in its parent Sequence.
     *
     * Note: when the Lua script explicitly terminates by a call to the custom Lua
     * function \a terminate_sequence() it is set to not running and an Error exception is
     * thrown carrying a special message for termination. When a sequence is run with
     * such a step it is catch by the \a Sequence::execute() member and a special
     * handling is performed.
     *
     * \returns true if the script returns a value that evaluates as true in the scripting
     *          language, or false otherwise (even in the case that the script returns no
     *          value at all).
     *
     * \exception Error is thrown if the script cannot be started, if there is a LUA error
     *            during execution, if a timeout is encountered, or if termination has
     *            been requested via the communication channel.
     */
    bool execute(Context& context, CommChannel* comm_channel, Message::IndexType index);

    /**
     * Execute the step script within the given context (without messaging).
     *
     * This function performs the following steps:
     * 1. A fresh script runtime environment is prepared and safe library components are
     *    loaded into it.
     * 2. The lua_init_function from the context is run if it is defined (non-null).
     * 3. Selected variables are imported from the context into the runtime environment.
     * 4. The script from the step is loaded into the runtime environment and executed.
     * 5. Selected variables are exported from the runtime environment back into the
     *    context.
     *
     * Note: when the Lua script explicitly terminates by a call to the custom Lua
     * function \a terminate_sequence() it is set to not running and an Error exception is
     * thrown carrying a special message for termination. When a sequence is run with
     * such a step it is catch by the \a Sequence::execute() member and a special
     * handling is performed.
     *
     * \param context  The context to be used for executing the step
     *
     * \returns true if the script returns a value that evaluates as true in the scripting
     *          language, or false otherwise (even in the case that the script returns no
     *          value at all).
     *
     * \exception Error is thrown if the script cannot be started or if it raises an error
     *            during execution.
     */
    bool execute(Context& context)
    {
        return execute(context, nullptr, 0);
    }

    /**
     * Retrieve the names of the variables that should be im-/exported to and from the
     * context.
     *
     * The variables with the listed names are imported from the context into the script
     * environment before running the script and are afterwards exported back into it.
     *
     * \note
     * This function returns a reference to an internal member variable, so be aware of
     * lifetime implications.
     */
    const VariableNames& get_used_context_variable_names() const
    {
        return used_context_variable_names_;
    }

    /**
     * Return the indentation level of this step.
     *
     * Zero indicates a top-level step and each additional level stands for one level of
     * nesting inside a block statement such as IF, WHILE, CATCH, and so on.
     */
    short get_indentation_level() const noexcept { return indentation_level_; }

    /**
     * Return the label of the step.
     *
     * \note
     * This function returns a reference to an internal member variable, so be aware of
     * lifetime implications.
     */
    const std::string& get_label() const { return label_; }

    /**
     * Return the script.
     *
     * \note
     * This function returns a reference to an internal member variable, so be aware of
     * lifetime implications:
     * \code
     * Step my_step;
     *
     * // Safe, string is copied
     * std::string str = my_step.get_script();
     *
     * // Fast, but str_ref will dangle if my_step goes out of scope
     * const std::string& str_ref = my_step.get_script();
     * \endcode
     */
    const std::string& get_script() const { return script_; }

    /**
     * Return the timestamp of the last execution of this step's script.
     * A default-constructed `TimePoint{}` is returned to indicate that the object was
     * never executed since its creation.
     */
    TimePoint get_time_of_last_execution() const { return time_of_last_execution_; }

    /**
     * Return the timestamp of the last modification of this step's script or label.
     * A default-constructed `TimePoint{}` is returned to indicate that the object was
     * never modified since its creation.
     */
    TimePoint get_time_of_last_modification() const { return time_of_last_modification_; }

    /// Return the timeout duration for executing the script.
    std::chrono::milliseconds get_timeout() const { return timeout_; }

    /// Return the type of this step.
    Type get_type() const noexcept { return type_; }

    /**
     * Return whether this step is currently being executed.
     *
     * This flag is normally set by an Executor.
     */
    bool is_running() const noexcept { return is_running_; }

    /// Return whether this step is currently disabled.
    bool is_disabled() const noexcept { return is_disabled_; }

    /// Set whether the step should be disabled (or possibly executed).
    void set_disabled(bool disable);

    /**
     * Set the indentation level of this step.
     *
     * \param level  New indentation level; Zero indicates a top-level step and each
     *               additional level stands for one level of nesting inside a block
     *               statement such as IF, WHILE, CATCH, and so on.
     *
     * \exception Error is thrown if level < 0 or level > max_indentation_level.
     */
    void set_indentation_level(short level);

    /**
     * Set the label.
     * This call also updates the time of last modification to the current system time.
     */
    void set_label(const std::string& label);

    /**
     * Set whether the step should be marked as "currently running".
     *
     * This is normally done by an Executor.
     */
    void set_running(bool is_running) { is_running_ = is_running; }

    /**
     * Set the script that should be executed when this step is run.
     * Syntax or semantics of the script are not checked.
     */
    void set_script(const std::string& script);

    /**
     * Set the timestamp of the last execution of this step's script.
     * This function should be called when an external execution engine starts the
     * embedded script or when the Step has been restored from serialized form.
     */
    void set_time_of_last_execution(TimePoint t) { time_of_last_execution_ = t; }

    /**
     * Set the timestamp of the last modification of this step's script or label.
     * This function is only useful to restore a step from some serialized form, e.g. from
     * a file.
     */
    void set_time_of_last_modification(TimePoint t) { time_of_last_modification_ = t; }

    /**
     * Set the timeout duration for executing the script.
     * Negative values set the timeout to zero.
     */
    void set_timeout(std::chrono::milliseconds timeout);

    /**
     * Set the type of this step.
     * This call also updates the time of last modification to the current system time.
     */
    void set_type(Type type);

    /// Set the names of the variables that should be im-/exported from/to the script.
    void set_used_context_variable_names(const VariableNames& used_context_variable_names);
    void set_used_context_variable_names(VariableNames&& used_context_variable_names);

private:
    std::string label_;
    std::string script_;
    VariableNames used_context_variable_names_;
    TimePoint time_of_last_modification_{ Clock::now() };
    TimePoint time_of_last_execution_;
    std::chrono::milliseconds timeout_{ infinite_timeout };
    Type type_{ type_action };
    short indentation_level_{ 0 };
    bool is_running_{ false };
    bool is_disabled_{ false };

    /**
     * Copy the variables listed in used_context_variable_names_ from the given Context
     * into a LUA state.
     */
    void copy_used_variables_from_context_to_lua(const Context& context, sol::state& lua);

    /**
     * Copy the variables listed in used_context_variable_names_ from a LUA state into the
     * given Context.
     */
    void copy_used_variables_from_lua_to_context(const sol::state& lua, Context& context);
};

/// Return a lower-case name for a step type ("action", "if", "end").
std::string to_string(Step::Type type);

} // namespace task

#endif
