/**
 * \file   internals.h
 * \author Lars Froehlich
 * \date   Created on August 30, 2022
 * \brief  Declaration of internal constants.
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

#ifndef TASKOMAT_INTERNALS_H_
#define TASKOMAT_INTERNALS_H_

#include <gul14/string_view.h>

namespace task {

/**
 * A marker string (the word "ABORT" surrounded by Unicode stop signs) whose presence
 * anywhere in an error message signals that the execution of a script should be stopped.
 */
extern const gul14::string_view abort_marker;

} // namespace task

#endif
