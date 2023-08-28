/**
 * \file   UniqueId.cc
 * \author Lars Fröhlich
 * \date   Created on July 26, 2023
 * \brief  Implementation of the UniqueId class.
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

#include <charconv>

#include <gul14/cat.h>
#include <gul14/escape.h>
#include <gul14/string_util.h>

#include "taskolib/exceptions.h"
#include "taskolib/UniqueId.h"

namespace task {

UniqueId::UniqueId()
{
    std::uniform_int_distribution<std::uint64_t> dist{
        0, std::numeric_limits<std::uint64_t>::max() };

    id_ = dist(random_number_generator_);
}

UniqueId::UniqueId(ValueType id)
    : id_{ id }
{}

gul14::optional<UniqueId> UniqueId::from_string(gul14::string_view str)
{
    if (str.empty() || str.size() > sizeof(ValueType) * 2)
        return {};

    ValueType result{};
    auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), result, 16);

    if (err != std::errc{})
        return {};

    return UniqueId{ result };
}

thread_local std::mt19937_64 UniqueId::random_number_generator_{ std::random_device{}() };

std::string to_string(UniqueId uid)
{
    return gul14::hex_string(uid.id_);
}

} // namespace task
