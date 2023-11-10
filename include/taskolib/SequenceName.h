/**
 * \file   SequenceName.h
 * \author Lars Fröhlich
 * \date   Created on August 25, 2023, based on older code
 * \brief  Declaration of the SequenceName class.
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

#ifndef TASKOLIB_SEQUENCENAME_H_
#define TASKOLIB_SEQUENCENAME_H_

#include <string>

#include <gul14/optional.h>
#include <gul14/string_view.h>

namespace task {

/**
 * The machine-readable name of a Sequence.
 *
 * A sequence name has constraints on its length and the contained characters. It may be
 * at most max_length bytes long and it may only contain upper- and lowercase letters,
 * digits, the minus and underscore characters, and periods. It may not start with a
 * period.
 */
class SequenceName
{
public:
    /// Maximum number of bytes of a sequence name.
    static constexpr std::size_t max_length = 64;

    /// A string containing all of the valid characters of a sequence name.
    static const gul14::string_view valid_characters;


    /// Default-construct an empty sequence name.
    SequenceName() = default;

    /**
     * Construct a sequence name from a string.
     * \exception Error is thrown if the string is too long or if it contains invalid
     *            characters.
     */
    explicit SequenceName(gul14::string_view str);

    /**
     * Create a sequence name from the given string, returning an empty optional if the
     * string violates the length or character constraints of a sequence name.
     */
    static gul14::optional<SequenceName> from_string(gul14::string_view str);

    /// Determine if two sequence names are equal.
    friend bool operator==(const SequenceName& a, const SequenceName& b)
    {
        return a.str_ == b.str_;
    }

    /// Determine if two sequence names are different.
    friend bool operator!=(const SequenceName& a, const SequenceName& b)
    {
        return a.str_ != b.str_;
    }

    /// Return the sequence name as a string.
    const std::string& string() const noexcept { return str_; }

private:
    std::string str_;

    /**
     * Throw an exception if the given string violates the length or character constraints
     * of a sequence name; otherwise, return the unmodified string view.
     */
    static gul14::string_view check_validity(gul14::string_view);
};

} // namespace task

#endif
