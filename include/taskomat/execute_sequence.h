/**
 * \file   execute_sequence.h
 * \author Marcus Walla
 * \date   Created on February 16, 2022
 * \brief  Implementation of the execute_sequence() free function.
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

#ifndef TASKOMAT_EXECUTE_SEQUENCE_H_
#define TASKOMAT_EXECUTE_SEQUENCE_H_

#include "taskomat/Context.h"
#include "taskomat/Sequence.h"

namespace task {

/**
 * \brief Execute a sequence within a given context.
 *
 * This function performs the following steps:
 * - iterates throw the steps
 * - executes each step
 *  
 * The variables are copied for step type \a task::Step::Type::type_action from
 * step to step with their intermediate changed values from the previous step.
 *
 * \par sequence [IN/OUT] sequence to execute
 * \par context [IN/OUT] context with given variable declaration
 * \exception Error is thrown if the script cannot be execute due to syntax error
 * or if it raises an error during execution.
 */
void execute_sequence(Sequence& sequence, Context& context);

} // namespace task

#endif