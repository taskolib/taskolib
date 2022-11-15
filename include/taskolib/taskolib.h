/**
 * \file   taskolib.h
 * \author Lars Froehlich
 * \date   Created on November 26, 2021
 * \brief  Main include file for Taskolib.
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

#ifndef TASKOLIB_TASKOLIB_H_
#define TASKOLIB_TASKOLIB_H_

#include "taskolib/Context.h"
#include "taskolib/exceptions.h"
#include "taskolib/execute_lua_script.h"
#include "taskolib/Executor.h"
#include "taskolib/Sequence.h"
#include "taskolib/SequenceManager.h"
#include "taskolib/Step.h"
#include "taskolib/time_types.h"
#include "taskolib/VariableName.h"

/// Namespace task contains all Taskolib functions and classes.
namespace task { }

#endif
