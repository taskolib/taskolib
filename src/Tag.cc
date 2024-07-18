/**
 * \file   Tag.cc
 * \author Lars Fröhlich
 * \date   Created on July 17, 2024
 * \brief  Implementation of the Tag class.
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

#include <ostream>

#include <gul14/cat.h>
#include <gul14/escape.h>
#include <gul14/substring_checks.h>

#include "taskolib/exceptions.h"
#include "taskolib/Tag.h"

using gul14::cat;

namespace task {

Tag::Tag(gul14::string_view name)
    : name_{ check_validity(gul14::lowercase_ascii(name)) }
{}

gul14::string_view Tag::check_validity(gul14::string_view name)
{
    if (name.empty())
        throw Error("Tag must not be empty");

    if (name.size() > max_length)
    {
        throw Error(cat("Tag '", name, "' is too long: ", name.size(), " bytes > ",
            max_length, " bytes"));
    }

    if (name.find_first_not_of(valid_characters) != gul14::string_view::npos)
        throw Error(cat("Tag '", gul14::escape(name), "' contains invalid characters"));

    return name;
}

const gul14::string_view Tag::valid_characters{ "abcdefghijklmnopqrstuvwxyz0123456789-" };

std::ostream& operator<<(std::ostream& stream, const Tag& tag)
{
    return stream << tag.name_;
}

} // namespace task
