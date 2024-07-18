/**
 * \file   test_deserialize_sequence.cc
 * \author Lars Fröhlich
 * \date   Created on July 26, 2023
 * \brief  Test suite for free functions declared in deserialize_sequence.h.
 *
 * \copyright Copyright 2023-2024 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include "deserialize_sequence.h"

using namespace task;

TEST_CASE("parse_tags()", "[deserialize_sequence]")
{
    REQUIRE(parse_tags("").empty());
    REQUIRE(parse_tags(" tag1 tag2\ttag3\n")
        == std::vector{ Tag{ "tag1" }, Tag{ "tag2" }, Tag{ "tag3" } });
    REQUIRE(parse_tags("c    Yet-Another-Tag")
        == std::vector{ Tag{ "c" }, Tag{ "yet-another-tag" } });
    REQUIRE_THROWS_AS(parse_tags("tag*"), Error);
    REQUIRE_THROWS_AS(parse_tags("tag2 tag3 Yet-Another-Tag NOT_ALLOWED"), Error);
}
