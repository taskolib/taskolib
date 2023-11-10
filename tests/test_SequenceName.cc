/**
 * \file   test_SequenceName.cc
 * \author Lars Fröhlich
 * \date   Created on July 27, 2023
 * \brief  Test suite for the SequenceName class.
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
#include "taskolib/SequenceName.h"

using namespace task;

TEST_CASE("SequenceName: Default constructor", "[SequenceName]")
{
    SequenceName name;
    REQUIRE(name.string() == "");
}

TEST_CASE("SequenceName: Constructor from string", "[SequenceName]")
{
    SequenceName a{ "1234" };
    SequenceName b{ "extremely-weird-combination" };
    SequenceName c{ "a.b-C_D" };

    REQUIRE_THROWS_AS(SequenceName{ std::string(SequenceName::max_length + 1, 'a') }, Error);
    REQUIRE_THROWS_AS(SequenceName{ "string with whitespace" }, Error);
    REQUIRE_THROWS_AS(SequenceName{ "abcd#e" }, Error);
    REQUIRE_THROWS_AS(SequenceName{ "abcd(e)" }, Error);
    REQUIRE_THROWS_AS(SequenceName{ "abcd[e]" }, Error);
    REQUIRE_THROWS_AS(SequenceName{ ".abcd" }, Error);
}

TEST_CASE("SequenceName: from_string()", "[SequenceName]")
{
    REQUIRE(SequenceName::from_string("1234") == SequenceName{ "1234" });
    REQUIRE(SequenceName::from_string("extremely-weird-combination")
            == SequenceName{ "extremely-weird-combination" });
    REQUIRE(SequenceName::from_string("a.b-C_D") == SequenceName{ "a.b-C_D" });

    REQUIRE(SequenceName::from_string(std::string(SequenceName::max_length + 1, 'a')) == gul14::nullopt);
    REQUIRE(SequenceName::from_string("string with whitespace") == gul14::nullopt);
    REQUIRE(SequenceName::from_string("abcd#e") == gul14::nullopt);
    REQUIRE(SequenceName::from_string("abcd(e)") == gul14::nullopt);
    REQUIRE(SequenceName::from_string("abcd[e]") == gul14::nullopt);
    REQUIRE(SequenceName::from_string(".abcd") == gul14::nullopt);
}

TEST_CASE("SequenceName: operator==()", "[SequenceName]")
{
    REQUIRE(SequenceName{ "1234" } == SequenceName{ "1234" });
    REQUIRE_FALSE(SequenceName{ "Gulag" } == SequenceName{ "gulag" });
    REQUIRE_FALSE(SequenceName{ "a.b.c" } == SequenceName{ "a-b-c" });
}

TEST_CASE("SequenceName: operator!=()", "[SequenceName]")
{
    REQUIRE_FALSE(SequenceName{ "1234" } != SequenceName{ "1234" });
    REQUIRE(SequenceName{ "Gulag" } != SequenceName{ "gulag" });
    REQUIRE(SequenceName{ "a.b.c" } != SequenceName{ "a-b-c" });
}

TEST_CASE("SequenceName: string()", "[SequenceName]")
{
    REQUIRE(SequenceName{ "1234" }.string() == "1234");
    REQUIRE(SequenceName{ "extremely-weird-combination" }.string()
            == "extremely-weird-combination");
    REQUIRE(SequenceName{ "a.b-C_D" }.string() == "a.b-C_D");
}
