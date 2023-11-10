/**
 * \file   SequenceName.cc
 * \author Lars Fröhlich
 * \date   Created on July 26, 2023
 * \brief  Implementation of the SequenceName class.
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

#include <gul14/cat.h>
#include <gul14/escape.h>
#include <gul14/substring_checks.h>

#include "taskolib/exceptions.h"
#include "taskolib/SequenceName.h"

using gul14::cat;

namespace task {

SequenceName::SequenceName(gul14::string_view str)
    : str_{ check_validity(str) }
{}

gul14::string_view SequenceName::check_validity(gul14::string_view str)
{
    if (str.size() > max_length)
    {
        throw Error(cat("Sequence name '", str, "' is too long: ", str.size(),
            " bytes > ", max_length, " bytes"));
    }

    if (str.find_first_not_of(valid_characters) != gul14::string_view::npos)
    {
        throw Error(cat("Sequence name '", gul14::escape(str),
            "' contains invalid characters"));
    }

    if (gul14::starts_with(str, '.'))
        throw Error(cat("A sequence name may not start with a period ('", str, "'"));

    return str;
}

gul14::optional<SequenceName> SequenceName::from_string(gul14::string_view str)
{
    try
    {
        return SequenceName{ str };
    }
    catch (const Error&)
    {
        return {};
    }
}

const gul14::string_view SequenceName::valid_characters{
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_." };

} // namespace task
