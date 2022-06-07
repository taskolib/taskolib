/**
 * \file   test_serialize_sequence.cc
 * \author Marcus Walla
 * \date   Created on May 6, 2022
 * \brief  Test suite for the free function serialize_sequence().
 *
 * \copyright Copyright 2021-2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <gul14/gul.h>
#include <gul14/catch.h>
#include <filesystem>
#include "taskomat/serialize_sequence.h"
#include "taskomat/deserialize_sequence.h"

using namespace task;

TEST_CASE("serialize_sequence(): sequence with one step", "[serialize_sequence]")
{
    Sequence seq{"Test a sequence"};
    Step step01{Step::type_action};
    step01.set_label("This is a label"),
    step01.set_script("a = a + 1");
    seq.push_back(step01);

    REQUIRE_NOTHROW(serialize_sequence("test_fs_sequence", seq));

    // TODO: is there a way to return an object from REQUIRE executing an expression:
    // auto deserialize_seq = REQUIRE_NOTHROW(deserialize_sequence(
    //         "test_fs_sequence/sequence_Test"));
    auto deserialize_seq = deserialize_sequence("test_fs_sequence/"
        "sequence_Test_a_sequence");
    REQUIRE(deserialize_seq.get_label() == "Test a sequence");

    REQUIRE(not deserialize_seq.empty());
    REQUIRE(1 == deserialize_seq.size());
    REQUIRE(deserialize_seq[0].get_type() == Step::type_action);
    REQUIRE(deserialize_seq[0].get_label() == "This is a label");

    // TODO: extracted from the file the stored context, sequence, and step(s)  

    REQUIRE(std::filesystem::remove_all("test_fs_sequence"));
}

