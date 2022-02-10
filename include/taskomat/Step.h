/**
 * \file   Step.h
 * \author Lars Froehlich
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
#include "taskomat/Context.h"

namespace task {

using Clock = std::chrono::system_clock;
using Timestamp = std::chrono::time_point<Clock>;
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


    /// Retrieve the names of the variables that should be exported from the script.
    VariableNames get_exported_variable_names() const { return exported_variable_names_; }

    /// Retrieve the names of the variables that should be imported into the script.
    VariableNames get_imported_variable_names() const { return imported_variable_names_; }

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
     * A default-constructed `Timestamp{}` is returned to indicate that the object was
     * never executed since its creation.
     */
    Timestamp get_time_of_last_execution() const { return time_of_last_execution_; }

    /**
     * Return the timestamp of the last modification of this step's script or label.
     * A default-constructed `Timestamp{}` is returned to indicate that the object was
     * never modified since its creation.
     */
    Timestamp get_time_of_last_modification() const { return time_of_last_modification_; }

    /// Return the timeout duration for executing the script.
    std::chrono::milliseconds get_timeout() const { return timeout_; }

    /// Return the type of this step.
    Type get_type() const noexcept { return type_; }

    /// Set the names of the variables that should be exported from the script.
    void set_exported_variable_names(const VariableNames& exported_variable_names);
    void set_exported_variable_names(VariableNames&& exported_variable_names);

    /// Set the names of the variables that should be imported into the script.
    void set_imported_variable_names(const VariableNames& imported_variable_names);
    void set_imported_variable_names(VariableNames&& imported_variable_names);

    /**
     * Set the label.
     * This call also updates the time of last modification to the current system time.
     */
    void set_label(const std::string& label);

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
    void set_time_of_last_execution(Timestamp t) { time_of_last_execution_ = t; }

    /**
     * Set the timestamp of the last modification of this step's script or label.
     * This function is only useful to restore a step from some serialized form, e.g. from
     * a file.
     */
    void set_time_of_last_modification(Timestamp t) { time_of_last_modification_ = t; }

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

private:
    std::string label_;
    std::string script_;
    VariableNames exported_variable_names_, imported_variable_names_;
    Timestamp time_of_last_modification_, time_of_last_execution_;
    std::chrono::milliseconds timeout_{ infinite_timeout };
    Type type_{ type_action };
};


} // namespace task

#endif