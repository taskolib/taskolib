/**
 * \file   Context.h
 * \author Lars Froehlich
 * \date   Created on December 20, 2021
 * \brief  Declaration of the Context and VariableValue types.
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

#ifndef TASKOLIB_CONTEXT_H_
#define TASKOLIB_CONTEXT_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <variant>

#include "sol/sol.hpp"
#include "taskolib/CommChannel.h"
#include "taskolib/console.h"
#include "taskolib/Message.h"
#include "taskolib/StepIndex.h"
#include "taskolib/VariableName.h"

namespace task {

/**
 * A VariableValue is a variant over several data types Lua can understand (long long, double, std::string, bool).
 *
 * Variable names are associated with these values via a map in the Context class.
 *
 * Be careful when assigning a string to a VariableValue:
 * Do not use a char* to pass the string, it might be converted to bool instead
 * of the expected std::string. The conversion depends on the used compiler (version).
 */
using VariableValue = std::variant<long long, double, std::string, bool>;

/**
 * Associative table that holds Lua variable names and their value.
 *
 * The keys are of type \a VariableName and the values \a VariableValue.
 */
using VariableTable = std::unordered_map<VariableName, VariableValue>;

/**
 * An output function accepts a string, an optional step index, and a CommChannel* as
 * parameters. The latter may be null to indicate that there is no associated
 * communication channel.
 */
using OutputCallback =
    std::function<void(const std::string&, OptionalStepIndex, CommChannel*)>;

/**
 * A context stores information that influences the execution of steps and sequences,
 * namely:
 * - A list of variables that can be im-/exported into steps.
 * - An initialization function that is called on a LUA state before a step is executed.
 * - Several callbacks that are invoked when a script calls print() or when the engine
 *   produces log output.
 */
struct Context
{
    /// A map of variables (names and values) that can be im-/exported into steps.
    VariableTable variables;

    /// An initialization function that is called on a LUA state before a step is executed.
    std::function<void(sol::state&)> lua_init_function;

    /// A callback that is invoked every time the script uses print().
    OutputCallback print_function = print_to_stdout;

    /// A callback that is invoked for informational log messages.
    OutputCallback log_info_function = print_info_to_stdout;

    /// A callback that is invoked for warning log messages.
    OutputCallback log_warning_function = print_warning_to_stdout;

    /// A callback that is invoked for error log messages.
    OutputCallback log_error_function = print_error_to_stdout;
};

} // namespace task

#endif
