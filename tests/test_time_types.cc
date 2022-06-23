/**
 * \file   test_time_types.cc
 * \author Fini Jastrow
 * \date   Created on June 22, 2022
 * \brief  Test suite for the TimePoint class (i.e. alias).
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
#include <sstream>

#include "taskomat/time_types.h"

TEST_CASE("TimePoint: Dump to stream", "[TimePoint]")
{
    std::stringstream ss{ };
    ss << task::TimePoint{ };
    REQUIRE(ss.str() == "1970-01-01 00:00:00 UTC");
}
