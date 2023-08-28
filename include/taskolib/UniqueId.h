/**
 * \file   UniqueId.h
 * \author Lars Fröhlich
 * \date   Created on July 26, 2023
 * \brief  Declaration of the UniqueId class.
 *
 * \copyright Copyright 2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOLIB_UNIQUEID_H_
#define TASKOLIB_UNIQUEID_H_

#include <random>
#include <string>

#include <gul14/optional.h>
#include <gul14/string_view.h>

namespace task {

/**
 * An unsigned 64-bit integer for use as a unique ID.
 */
class UniqueId
{
public:
    using ValueType = std::uint64_t;

    /// Default-construct a random unique ID.
    UniqueId();

    /// Construct a unique ID from a given integer.
    explicit UniqueId(ValueType id);

    /**
     * Create a unique ID from the given string, returning an empty optional if the string
     * does not represent a valid hexadecimal number.
     *
     * The string must represent a hexadecimal number with a maximum of 16 characters.
     * Neither leading whitespace nor a "0x" prefix are allowed.
     */
    static gul14::optional<UniqueId> from_string(gul14::string_view str);

    /// Determine if two unique IDs are equal.
    friend bool operator==(UniqueId a, UniqueId b)
    {
        return a.id_ == b.id_;
    }

    /// Determine if two unique IDs are different.
    friend bool operator!=(UniqueId a, UniqueId b)
    {
        return a.id_ != b.id_;
    }

    /// Return a hexadecimal string representation of the given unique ID.
    friend std::string to_string(UniqueId uid);

private:
    static thread_local std::mt19937_64 random_number_generator_;

    ValueType id_;
};


namespace literals {

/// User-defined literal for generating a UniqueId.
inline ::task::UniqueId operator""_uid(unsigned long long id)
{
    return ::task::UniqueId{ id };
}

} // namespace literals

} // namespace task

#endif
