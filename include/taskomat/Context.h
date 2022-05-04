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
#include <unordered_map>
#include <variant>
#include "sol/sol.hpp"
#include "taskomat/VariableName.h"

namespace task {

/**
 * A VariableValue is a variant over several data types (long long, double, std::string).
 *
 * Names are associated with these values via a map in the Context type.
 */
using VariableValue = std::variant<long long, double, std::string>;

/**
 * A context stores information that influences the execution of steps and sequences,
 * namely:
 * - An initialization function that is called on a LUA state before a step is executed.
 * - A list of variables that can be im-/exported into steps.
 */
struct Context
{
    std::function<void(sol::state&)> lua_init_function;
    std::unordered_map<VariableName, VariableValue> variables;
};

} // namespace task


#endif
