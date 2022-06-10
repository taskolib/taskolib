/**
 * \file   VariableName.h
 * \author Lars Froehlich
 * \date   Created on January 6, 2022
 * \brief  Declaration of the VariableName class and of an associated specialization of
 *         std::hash.
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

#ifndef TASKOMAT_VARIABLENAME_H_
#define TASKOMAT_VARIABLENAME_H_

#include <functional>
#include <string>
#include <gul14/string_view.h>

namespace task {

/**
 * A variable name is a string with limited functionality and some limitations on the
 * allowed characters.
 *
 * Basically, a variable name may only contain alphanumeric characters plus the underscore
 * ("_"). It must start with a letter. Variable names are case sensitive and may not be
 * more than 64 characters long.
 */
class VariableName
{
public:
    /**
     * Construct a variable name from a C string.
     *
     * This constructor can be called implicitly to make the assignment of literal
     * variable names easier.
     *
     * \exception Error is thrown if the name is not a valid variable name.
     */
    VariableName(const char* name);

    /**
     * Construct a variable name from a string.
     *
     * \exception Error is thrown if the name is not a valid variable name.
     */
    explicit VariableName(const std::string& name);
    explicit VariableName(std::string&& name);

    friend bool operator==(const VariableName& a, const VariableName& b) noexcept
    {
        return a.string() == b.string();
    }

    friend bool operator!=(const VariableName& a, const VariableName& b) noexcept
    {
        return a.string() != b.string();
    }

    friend bool operator<(const VariableName& a, const VariableName& b) noexcept
    {
        return a.string() < b.string();
    }

    friend bool operator>(const VariableName& a, const VariableName& b) noexcept
    {
        return a.string() > b.string();
    }

    friend bool operator<=(const VariableName& a, const VariableName& b) noexcept
    {
        return a.string() <= b.string();
    }

    friend bool operator>=(const VariableName& a, const VariableName& b) noexcept
    {
        return a.string() >= b.string();
    }

    /// Return a const reference to the internal string member.
    const std::string& string() const noexcept { return name_; }

    /// Append a VariableName to a std::string.
    friend std::string& operator+=(std::string& lhs, const VariableName& rhs)
    {
        lhs += rhs.name_;
        return lhs;
    }

    /// Concatenate a VariableName and a string_view.
    friend std::string operator+(const VariableName& lhs, gul14::string_view rhs)
    {
        return lhs.name_ + rhs;
    }

    /// Concatenate a string_view and a VariableName.
    friend std::string operator+(gul14::string_view lhs, const VariableName& rhs)
    {
        return lhs + rhs.name_;
    }

private:
    std::string name_;
};

} // namespace task


namespace std {

/// Custom specialization of std::hash for VariableName
template<>
struct hash<task::VariableName>
{
    std::size_t operator()(const task::VariableName& name) const noexcept
    {
        return std::hash<std::string>{}(name.string());
    }
};

} // namespace std

#endif
