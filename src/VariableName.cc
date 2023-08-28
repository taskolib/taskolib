/**
 * \file   VariableName.cc
 * \author Lars Fröhlich
 * \date   Created on January 6, 2022
 * \brief  Implementation of the VariableName class.
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

#include <algorithm>
#include <cctype>
#include <gul14/cat.h>
#include "taskolib/exceptions.h"
#include "taskolib/VariableName.h"

using gul14::cat;

namespace task {


namespace {

// Check that the given name is a valid variable name or throw a task::Error.
void check_name(gul14::string_view name)
{
    if (name.empty())
        throw Error("A variable name may not be empty");

    if (name.size() > 64)
        throw Error(cat("Variable name \"", name, "\" is too long (>64 characters)"));

    if (!std::isalpha(static_cast<unsigned char>(name[0]))) // cast is important
        throw Error(cat("Variable name \"", name, "\"does not start with a letter"));

    if (not std::all_of(name.begin() + 1, name.end(),
                        [](unsigned char c) { return c == '_' || std::isalnum(c); }))
    {
        throw Error(cat("Variable name \"", name, "\" contains illegal characters"));
    }
}

} // anonymous namespace


VariableName::VariableName(const char* name)
{
    if (name == nullptr)
        throw Error("A null pointer is not a valid variable name");

    check_name(name);
    name_ = name;
}

VariableName::VariableName(const std::string& name)
{
    check_name(name);
    name_ = name;
}

VariableName::VariableName(std::string&& name)
{
    check_name(name);
    name_ = std::move(name);
}

VariableName& VariableName::operator+=(gul14::string_view suffix)
{
    std::string new_name = name_ + suffix;

    check_name(new_name);

    name_ = std::move(new_name);
    return *this;
}


} // namespace task
