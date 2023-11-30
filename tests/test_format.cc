/**
 * \file   test_format.cc
 * \author Fini Jastrow
 * \date   Created on June 22, 2022
 * \brief  Test suite for the fmt{} library helpers.
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

#include <fmt/format.h>
#include <gul14/catch.h>
#include <gul14/trim.h>

#include "taskolib/Message.h"
#include "taskolib/time_types.h"
#include "taskolib/format.h"

using namespace task;

TEST_CASE("format: Message", "[format]")
{
    Message msg;

    msg.set_type(Message::Type::step_started);
    msg.set_timestamp(TimePoint{});
    msg.set_text("Beware of the foxes");
    msg.set_index(32);

    auto x = fmt::format("{}", msg);
    REQUIRE(gul14::trim(x) == "Message{ 32: step_started \"Beware of the foxes\" 1970-01-01 00:00:00 UTC }");
}

TEST_CASE("format: Timeout", "[format]")
{
    auto x = fmt::format("{}", Timeout{ 0s });
    REQUIRE(x == "0");
    x = fmt::format("{}", Timeout::infinity());
    REQUIRE(x == "infinite");
}

TEST_CASE("format: TimePoint", "[format]")
{
    auto x = fmt::format("{}", TimePoint{ });
    REQUIRE(x == "1970-01-01 00:00:00 UTC");
}
