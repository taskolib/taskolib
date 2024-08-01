/**
 * \file   Tag.h
 * \author Lars Fröhlich
 * \date   Created on July 17, 2024
 * \brief  Declaration of the Tag class.
 *
 * \copyright Copyright 2024 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOLIB_TAG_H_
#define TASKOLIB_TAG_H_

#include <iosfwd>
#include <string>

#include <gul14/string_view.h>

namespace task {

/**
 * A tag used for categorizing sequences.
 *
 * A tag consists of lowercase ASCII letters, digits, and hyphen characters. It must be at
 * least 1 character long and has a maximum length given by the max_length member
 * constant. Uppercase letters are automatically converted to lowercase when comparing or
 * creating tags.
 */
class Tag
{
public:
    /// Maximum number of bytes of a tag name.
    static constexpr std::size_t max_length = 32;

    /// A string containing all of the valid characters for a tag name.
    static const gul14::string_view valid_characters;


    /// Default-construct a tag with the name "-".
    Tag() : name_{ "-" }
    {}

    /**
     * Construct a tag with the specified name
     *
     * Uppercase ASCII characters are automatically converted to lowercase.
     *
     * \exception Error is thrown if the string is too long or if it contains invalid
     *            characters.
     */
    explicit Tag(gul14::string_view name);

    /// Determine if two tags are equal.
    friend bool operator==(const Tag& a, const Tag& b)
    {
        return a.name_ == b.name_;
    }

    /// Determine if two tags are different.
    friend bool operator!=(const Tag& a, const Tag& b)
    {
        return a.name_ != b.name_;
    }

    /// Determine if tag a comes before tag b in a lexicographical comparison.
    friend bool operator<(const Tag& a, const Tag& b)
    {
        return a.name_ < b.name_;
    }

    /**
     * Determine if tag a comes before or is equal to tag b in a lexicographical
     * comparison.
     */
    friend bool operator<=(const Tag& a, const Tag& b)
    {
        return a.name_ <= b.name_;
    }

    /// Determine if tag a comes after tag b in a lexicographical comparison.
    friend bool operator>(const Tag& a, const Tag& b)
    {
        return a.name_ > b.name_;
    }

    /**
     * Determine if tag a comes after or is equal to tag b in a lexicographical
     * comparison.
     */
    friend bool operator>=(const Tag& a, const Tag& b)
    {
        return a.name_ >= b.name_;
    }

    /// Concatenate a string and a tag.
    friend std::string operator+(const std::string& lhs, const Tag& rhs)
    {
        return lhs + rhs.name_;
    }

    /// Concatenate a tag and a string.
    friend std::string operator+(const Tag& lhs, const std::string& rhs)
    {
        return lhs.name_ + rhs;
    }

    /// Concatenate a string and a tag.
    friend std::string& operator+=(std::string& lhs, const Tag& rhs)
    {
        lhs += rhs.name_;
        return lhs;
    }

    /// Output the tag name to the given stream.
    friend std::ostream& operator<<(std::ostream& stream, const Tag& tag);

    /// Return the length of the tag name.
    std::size_t size() const noexcept { return name_.size(); }

    /// Return the name of the tag as a string.
    const std::string& string() const noexcept { return name_; }

private:
    std::string name_;

    /**
     * Throw an exception if the given string violates the length or character constraints
     * of a tag name; otherwise, return the unmodified string view. Uppercase ASCII
     * characters are rejected.
     */
    static gul14::string_view check_validity(gul14::string_view);
};

} // namespace task

#endif
