/**
 * \file   test_CommChannel.cc
 * \author Lars Fröhlich
 * \date   Created on June 8, 2022
 * \brief  Test suite for the CommChannel struct.
 *
 * \copyright Copyright 2022-2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <type_traits>

#include <gul14/catch.h>

#include "taskolib/CommChannel.h"

using namespace task;

TEST_CASE("CommChannel: Constructor", "[CommChannel]")
{
    static_assert(std::is_default_constructible_v<CommChannel>,
        "CommChannel is default-constructible");

    CommChannel c;
}
