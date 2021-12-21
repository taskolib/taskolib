/**
 * \file   Step.cc
 * \author Lars Froehlich
 * \date   Created on December 7, 2021
 * \brief  Implementation of the Step class.
 *
 * \copyright Copyright 2021 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include "avtomat/Step.h"

using namespace std::literals;

namespace avto {


void Step::set_exported_variable_names(const VariableNames& exported_variable_names)
{
    exported_variable_names_ = exported_variable_names;
}

void Step::set_exported_variable_names(VariableNames&& exported_variable_names)
{
    exported_variable_names_ = std::move(exported_variable_names);
}

void Step::set_imported_variable_names(const VariableNames& imported_variable_names)
{
    imported_variable_names_ = imported_variable_names;
}

void Step::set_imported_variable_names(VariableNames&& imported_variable_names)
{
    imported_variable_names_ = std::move(imported_variable_names);
}

void Step::set_label(const std::string& label)
{
    label_ = label;
    set_time_of_last_modification(Clock::now());
}

void Step::set_script(const std::string& script)
{
    script_ = script;
    set_time_of_last_modification(Clock::now());
}

void Step::set_timeout(std::chrono::milliseconds timeout)
{
    if (timeout < 0s)
        timeout_ = 0s;
    else
        timeout_ = timeout;
}

void Step::set_type(Type type)
{
    type_ = type;
    set_time_of_last_modification(Clock::now());
}


} // namespace avto
