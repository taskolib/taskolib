/**
 * \file   console.h
 * \author Lars Froehlich
 * \date   Created on June 22, 2022
 * \brief  Declaration of several functions for console output.
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

#ifndef TASKOMAT_CONSOLE_H_
#define TASKOMAT_CONSOLE_H_

#include <string>

namespace task {

class CommChannel;

/// Print a string to stdout.
void print_to_stdout(const std::string& str, CommChannel* comm_channel);

/// Print a string to stdout with an "INFO: " prefix.
void print_info_to_stdout(const std::string& str, CommChannel* comm_channel);

/// Print a string to stdout with a "WARNING: " prefix.
void print_warning_to_stdout(const std::string& str, CommChannel* comm_channel);

/// Print a string to stdout with an "ERROR: " prefix.
void print_error_to_stdout(const std::string& str, CommChannel* comm_channel);

} // namespace task

#endif
