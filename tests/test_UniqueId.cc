/**
 * \file   test_UniqueId.cc
 * \author Lars Fröhlich
 * \date   Created on July 27, 2023
 * \brief  Test suite for the UniqueId class.
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

#include <gul14/catch.h>

#include "taskolib/exceptions.h"
#include "taskolib/UniqueId.h"

using namespace task;
using namespace task::literals;

TEST_CASE("UniqueId: Default constructor", "[UniqueId]")
{
    // Default-construct 100 random UIDs
    std::array<UniqueId, 100> ids;

    // Check that all IDs are unique.
    auto it = std::unique(ids.begin(), ids.end());
    REQUIRE(it == ids.end());
}

TEST_CASE("UniqueId: Constructor from integer", "[UniqueId]")
{
    UniqueId a{ 1234 };
    UniqueId b{ 0xdeadbeef };
    UniqueId c{ 0 };
}

TEST_CASE("UniqueId: User-defined literal", "[UniqueId]")
{
    REQUIRE(UniqueId{ 1234 } == 1234_uid);
    REQUIRE(UniqueId{ 0xdeadbeef } == 0xdeadbeef_uid);
    REQUIRE(UniqueId{ 0 } == 0_uid);
}

TEST_CASE("UniqueId: from_string()", "[UniqueId]")
{
    REQUIRE(UniqueId::from_string("1234") == UniqueId{ 0x1234 });
    REQUIRE(UniqueId::from_string("deadbeef") == UniqueId{ 0xdeadbeef });
    REQUIRE(UniqueId::from_string("123456789abcdef0") == UniqueId{ 0x123456789abcdef0 });
    REQUIRE(UniqueId::from_string("0") == UniqueId{ 0 });
    REQUIRE(UniqueId::from_string("").has_value() == false);
    REQUIRE(UniqueId::from_string("gabbagabba").has_value() == false);
    REQUIRE(UniqueId::from_string("0123456789abcdef0").has_value() == false);
}

TEST_CASE("UniqueId: operator==()", "[UniqueId]")
{
    REQUIRE(UniqueId{ 1234 } == UniqueId{ 1234 });
    REQUIRE_FALSE(UniqueId{ 1235 } == UniqueId{ 1234 });
    REQUIRE_FALSE(UniqueId{ 0 } == UniqueId{ 1 });
}

TEST_CASE("UniqueId: operator!=()", "[UniqueId]")
{
    REQUIRE_FALSE(UniqueId{ 1234 } != UniqueId{ 1234 });
    REQUIRE(UniqueId{ 1235 } != UniqueId{ 1234 });
    REQUIRE(UniqueId{ 0 } != UniqueId{ 1 });
}

TEST_CASE("to_string(UniqueId)", "[UniqueId]")
{
    REQUIRE(to_string(32_uid) == "0000000000000020");
    REQUIRE(to_string(0xdeadbeef_uid) == "00000000deadbeef");
    REQUIRE(to_string(0x123456789abcdef0_uid) == "123456789abcdef0");
    REQUIRE(to_string(0_uid) == "0000000000000000");
}
