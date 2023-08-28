/**
 * \file   test_deserialize_sequence.cc
 * \author Lars Fröhlich
 * \date   Created on July 26, 2023
 * \brief  Test suite for free functions declared in deserialize_sequence.h.
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

#include "deserialize_sequence.h"

using namespace task;
using namespace task::literals;

TEST_CASE("get_sequence_info_from_filename()", "[deserialize_sequence]")
{
    REQUIRE(get_sequence_info_from_filename("my_sequence[deadbeef]")
        == SequenceInfo{ "", SequenceName{ "my_sequence" }, 0xdeadbeef_uid });
    REQUIRE(get_sequence_info_from_filename("[1234567890abcdef]")
        == SequenceInfo{ "", gul14::nullopt, 0x1234567890abcdef_uid });
    REQUIRE(get_sequence_info_from_filename("A$2f$22sequence$22$24$3cagain$3e [my_uid]")
        == SequenceInfo{ "A/\"sequence\"$<again> [my_uid]", gul14::nullopt, gul14::nullopt });
}
