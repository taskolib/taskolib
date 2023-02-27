/**
 * \file   Step.h
 * \author Lars Froehlich, Marcus Walla
 * \date   Created on November 26, 2021
 * \brief  Declaration of the Step class.
 *
 * \copyright Copyright 2021-2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOLIB_STEP_H_
#define TASKOLIB_STEP_H_

#include <chrono>
#include <set>
#include <string>

#include "taskolib/CommChannel.h"
#include "taskolib/Context.h"
#include "taskolib/time_types.h"
#include "taskolib/Timeout.h"
#include "taskolib/TimeoutTrigger.h"
#include "taskolib/VariableName.h"

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
     * 2. The step_setup_function from the context is run if it is defined (non-null).
     * 3. The step setup script is run.
     * 4. Selected variables are imported from the context into the runtime environment.
     * 5. The script from the step is loaded into the runtime environment and executed.
     * 6. Selected variables are exported from the runtime environment back into the
     *    context.
     *
     * Certain step types (IF, ELSEIF, WHILE) require the script to return a boolean
     * value. Not returning a value or returning a different type is considered an error.
     * Conversely, the other step types (ACTION etc.) do not allow returning values from
     * the script, with the exception of nil.
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
     * \param opt_step_index  Optional index of the step in its parent Sequence (to be
     *                        used in exceptions and messages)
     * \param sequence_timeout Pointer to a sequence timeout to determine a timeout during
     *                      executing a step. If this is null the corresponding check for
     *                      timeout is omitted.
     *
     * \return If the step type requires a boolean return value (IF, ELSEIF, WHILE), this
     *         function returns the return value of the script. For other step types
     *         (ACTION etc.), it returns false.
     *
     * \exception Error is thrown if the script cannot be started, if
     *            there is a Lua error during execution, if the script has an
     *            inappropriate return value for the step type (see above), if a timeout
     *            is encountered, or if termination has been requested via the
     *            communication channel or explicitly by the script.
     *
     * \see For more information about step setup scripts see at Sequence.
     */
    bool execute(Context& context, CommChannel* comm_channel = nullptr,
                 OptionalStepIndex opt_step_index = gul14::nullopt,
                 TimeoutTrigger* sequence_timeout = nullptr);

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
    Timeout get_timeout() const { return timeout_; }

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
    Step& set_disabled(bool disable);

    /**
     * Set the indentation level of this step.
     *
     * \param level  New indentation level; Zero indicates a top-level step and each
     *               additional level stands for one level of nesting inside a block
     *               statement such as IF, WHILE, CATCH, and so on.
     *
     * \exception Error is thrown if level < 0 or level > max_indentation_level.
     */
    Step& set_indentation_level(short level);

    /**
     * Set the label.
     * This call also updates the time of last modification to the current system time.
     *
     * Labels must not start or end with whitespace; existing whitespace is
     * silently removed.
     */
    Step& set_label(const std::string& label);

    /**
     * Set whether the step should be marked as "currently running".
     *
     * This is normally done by an Executor.
     */
    Step& set_running(bool is_running);

    /**
     * Set the script that should be executed when this step is run.
     * Syntax or semantics of the script are not checked.
     */
    Step& set_script(const std::string& script);

    /**
     * Set the timestamp of the last execution of this step's script.
     * This function should be called when an external execution engine starts the
     * embedded script or when the Step has been restored from serialized form.
     */
    Step& set_time_of_last_execution(TimePoint t);

    /**
     * Set the timestamp of the last modification of this step's script or label.
     * This function is only useful to restore a step from some serialized form, e.g. from
     * a file.
     */
    Step& set_time_of_last_modification(TimePoint t);

    /// Set the timeout duration for executing the script.
    Step& set_timeout(Timeout timeout);

    /**
     * Set the type of this step.
     * This call also updates the time of last modification to the current system time.
     */
    Step& set_type(Type type);

    /// Set the names of the variables that should be im-/exported from/to the script.
    Step& set_used_context_variable_names(const VariableNames& used_context_variable_names);
    Step& set_used_context_variable_names(VariableNames&& used_context_variable_names);

private:
    std::string label_;
    std::string script_;
    VariableNames used_context_variable_names_;
    TimePoint time_of_last_modification_{ Clock::now() };
    TimePoint time_of_last_execution_;
    Timeout timeout_;
    Type type_{ type_action };
    short indentation_level_{ 0 };
    bool is_running_{ false };
    bool is_disabled_{ false };

    /**
     * Copy the variables listed in used_context_variable_names_ from the given Context
     * into a Lua state.
     */
    void copy_used_variables_from_context_to_lua(const Context& context, sol::state& lua);

    /**
     * Copy the variables listed in used_context_variable_names_ from a Lua state into the
     * given Context.
     */
    void copy_used_variables_from_lua_to_context(const sol::state& lua, Context& context);

    /**
     * Execute the Lua script, throwing an exception if anything goes wrong.
     * \see execute(Context&, CommChannel*, OptionalStepIndex, TimeoutTrigger*)
     */
    bool execute_impl(Context& context, CommChannel* comm_channel
        , OptionalStepIndex index, TimeoutTrigger* sequence_timeout);
};

/// Alias for a step type collection that executes a script.
using ExecutionSteps = std::set<Step::Type>;

/// Step types that execute a script.
const ExecutionSteps execution_steps{
    Step::type_action,
    Step::type_if,
    Step::type_elseif,
    Step::type_while
    };

/// Return a lower-case name for a step type ("action", "if", "end").
std::string to_string(Step::Type type);

/**
 * Determine if a script is executed.
 *
 * \param step_type step type to check
 * \return true for executing a script otherwise false.
 */
bool executes_script(Step::Type step_type);

/// Determine if a certain step type requires a boolean return value from the script.
bool requires_bool_return_value(Step::Type step_type) noexcept;

} // namespace task

#endif
