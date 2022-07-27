/**
 * \file   test_SequenceManager.cc
 * \author Marcus Walla
 * \date   Created on July 22, 2022
 * \brief  Test suite for the SequenceManager class.
 *
 * \copyright Copyright 2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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
#include <type_traits>
#include "taskomat/SequenceManager.h"

using namespace task;

TEST_CASE("Construct empty SequenceManager", "[SequenceManager]")
{
    SequenceManager::SequenceManagerRef sm = SequenceManager::get();
    REQUIRE(not sm.get_path().empty());
    REQUIRE(sm.get_path() == ".");
}

TEST_CASE("Construct SequenceManager with path", "[SequenceManager]")
{
    SequenceManager::SequenceManagerRef sm = SequenceManager::get();
    sm.set_path("./some/path/to/sequences");
    REQUIRE(not sm.get_path().empty());
    REQUIRE(sm.get_path() == "./some/path/to/sequences");
}
