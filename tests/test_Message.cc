/**
 * \file   test_Message.cc
 * \author Lars Fröhlich
 * \date   Created on April 1, 2022
 * \brief  Test suite for the Message class.
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

#include <sstream>
#include <thread>
#include <type_traits>

#include <gul14/catch.h>
#include <gul14/trim.h>

#include "taskolib/Message.h"
#include "taskolib/Sequence.h"

using namespace task;
using namespace std::literals;

TEST_CASE("Message: Check IndexType", "[Message]")
{
    REQUIRE(std::is_same_v<StepIndex, Sequence::SizeType> == true);
}

TEST_CASE("Message: Constructors", "[Message]")
{
    static_assert(std::is_default_constructible_v<Message>,
        "Message is default constructible");
    static_assert(std::is_constructible_v<Message, Message::Type, std::string, TimePoint,
                                          StepIndex>,
        "Message can be constructed with init values");

    const TimePoint t0 = Clock::now();

    Message a;
    Message b(Message::Type::output, "Test", t0, 42);

    REQUIRE(b.get_type() == Message::Type::output);
    REQUIRE(b.get_text() == "Test");
    REQUIRE(b.get_timestamp() == t0);
    REQUIRE(b.get_index().has_value());
    REQUIRE(*(b.get_index()) == 42);
}

TEST_CASE("Message: get_index()", "[Message]")
{
    Message msg;
    REQUIRE(msg.get_index().has_value() == false);

    msg.set_index(42);
    REQUIRE(msg.get_index().has_value());
    REQUIRE(*(msg.get_index()) == 42);
}

TEST_CASE("Message: get_text()", "[Message]")
{
    Message msg;
    REQUIRE(msg.get_text() == "");

    msg.set_text("Test");
    REQUIRE(msg.get_text() == "Test");
}

TEST_CASE("Message: get_timestamp()", "[Message]")
{
    const auto t0 = Clock::now();

    Message msg;
    REQUIRE(msg.get_timestamp() == TimePoint{});

    msg.set_timestamp(t0);
    REQUIRE(msg.get_timestamp() == t0);
}

TEST_CASE("Message: get_type()", "[Message]")
{
    Message msg;
    REQUIRE(msg.get_type() == Message::Type::output);

    msg.set_type(Message::Type::step_started);
    REQUIRE(msg.get_type() == Message::Type::step_started);
}

TEST_CASE("Message: set_index()", "[Message]")
{
    Message msg;

    REQUIRE(&msg.set_index(42) == &msg);
    REQUIRE(msg.get_index().has_value() == true);
    REQUIRE(*(msg.get_index()) == 42);

    msg.set_index(0);
    REQUIRE(msg.get_index().has_value() == true);
    REQUIRE(*(msg.get_index()) == 0);

    msg.set_index(gul14::nullopt);
    REQUIRE(msg.get_index().has_value() == false);
}

TEST_CASE("Message: set_text()", "[Message]")
{
    Message msg;

    REQUIRE(&msg.set_text("Test") == &msg);
    REQUIRE(msg.get_text() == "Test");

    msg.set_text("");
    REQUIRE(msg.get_text() == "");
}

TEST_CASE("Message: set_timestamp()", "[Message]")
{
    const auto t0 = Clock::now();

    Message msg;
    REQUIRE(&msg.set_timestamp(t0) == &msg);
    REQUIRE(msg.get_timestamp() == t0);

    msg.set_timestamp(TimePoint{});
    REQUIRE(msg.get_timestamp() == TimePoint{});
}

TEST_CASE("Message: set_type()", "[Message]")
{
    Message msg;

    REQUIRE(&msg.set_type(Message::Type::step_started) == &msg);
    REQUIRE(msg.get_type() == Message::Type::step_started);

    msg.set_type(Message::Type::output);
    REQUIRE(msg.get_type() == Message::Type::output);
}

TEST_CASE("Message: Dump to stream", "[Message]")
{
    Message msg;

    msg.set_type(Message::Type::step_started)
       .set_timestamp(TimePoint{})
       .set_text("Beware of the foxes")
       .set_index(32);

    std::stringstream ss{ };
    ss << msg;
    REQUIRE(gul14::trim(ss.str()) == "Message{ 32: step_started \"Beware of the foxes\" 1970-01-01 00:00:00 UTC }");

    // msg.set_type(Message::Type::output); We don't have that yet
    msg.set_text("Lua print() has been called\n");
    ss.str(""s);
    ss << msg;
    REQUIRE(gul14::trim(ss.str()) == "Message{ 32: step_started \"Lua print() has been called\\n\" 1970-01-01 00:00:00 UTC }");
}
