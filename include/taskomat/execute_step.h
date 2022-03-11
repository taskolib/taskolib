/**
 * \file   execute_step.h
 * \author Lars Froehlich
 * \date   Created on December 20, 2021
 * \brief  Declaration of the execute_step() function.
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

#ifndef TASKOMAT_EXECUTE_STEP_H_
#define TASKOMAT_EXECUTE_STEP_H_

#include "taskomat/Context.h"
#include "taskomat/Step.h"

namespace task {

/**
 * Execute the script from the given step within the given context.
 *
 * This function performs the following steps:
 * 1. A fresh script runtime environment is prepared and safe library components are
 *    loaded into it.
 * 2. The lua_init_function from the context is run if it is defined (non-null).
 * 3. Selected variables are imported from the context into the runtime environment.
 * 4. The script from the step is loaded into the runtime environment and executed.
 * 5. Selected variables are exported from the runtime environment back into the context.
 *
 * \returns true if the script returns a value that evaluates as true in the scripting
 *          language, or false otherwise (even in the case that the script returns no
 *          value at all).
 *
 * \exception Error is thrown if the script cannot be started or if it raises an error
 *            during execution.
 */
bool execute_step(Step& step, Context& context);

} // namespace task

#endif
