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

#ifndef TASKOMAT_CONTEXT_H_
#define TASKOMAT_CONTEXT_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <variant>

#include "sol/sol.hpp"
#include "taskomat/CommChannel.h"
#include "taskomat/console.h"
#include "taskomat/VariableName.h"

namespace task {

/**
 * A VariableValue is a variant over several data types (long long, double, std::string).
 *
 * Names are associated with these values via a map in the Context type.
 */
using VariableValue = std::variant<long long, double, std::string>;

/**
 * An output function accepts a string and a CommChannel* as parameters.
 * The latter may be null to indicate that there is no associated communication channel.
 */
using OutputCallback = std::function<void(const std::string&, CommChannel*)>;

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
    /// A list of variables that can be im-/exported into steps.
    std::unordered_map<VariableName, VariableValue> variables;

    /// An initialization function that is called on a LUA state before a step is executed.
    std::function<void(sol::state&)> lua_init_function;

    /// A callback that is invoked every time the script uses print().
    OutputCallback print_function = print_to_stdout;

    /// A callback that is invoked for informational log messages.
    OutputCallback log_info_function = print_info_to_stdout;

    /// A callback that is invoked for warning log messages.
    OutputCallback log_warning_function = print_warning_to_stdout;

    /// A callback that is invoked for error log messages.
    OutputCallback log_error_function = print_error_to_stderr;
};

} // namespace task

#endif
