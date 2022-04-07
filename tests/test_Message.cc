/**
 * \file   test_Message.cc
 * \author Lars Froehlich
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

#include <thread>
#include <type_traits>
#include <gul14/catch.h>
#include "taskomat/Message.h"

using namespace task;
using namespace std::literals;

TEST_CASE("Message: Constructors", "[Message]")
{
    static_assert(std::is_default_constructible_v<Message>,
        "Message is default constructible");
    static_assert(std::is_constructible_v<Message, Message::Type, std::string, TimePoint>,
        "Message can be constructed with init values");

    const TimePoint t0 = Clock::now();

    Message a;
    Message b(Message::Type::log, "Test", t0);

    REQUIRE(b.get_type() == Message::Type::log);
    REQUIRE(b.get_text() == "Test");
    REQUIRE(b.get_timestamp() == t0);
}

TEST_CASE("StoppedMessage: get_text()", "[Message]")
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

TEST_CASE("StoppedMessage: get_type()", "[Message]")
{
    Message msg;
    REQUIRE(msg.get_type() == Message::Type::log);

    msg.set_type(Message::Type::step_started);
    REQUIRE(msg.get_type() == Message::Type::step_started);
}

TEST_CASE("StoppedMessage: set_text()", "[Message]")
{
    Message msg;

    msg.set_text("Test");
    REQUIRE(msg.get_text() == "Test");

    msg.set_text("");
    REQUIRE(msg.get_text() == "");
}

TEST_CASE("Message: set_timestamp()", "[Message]")
{
    const auto t0 = Clock::now();

    Message msg;
    msg.set_timestamp(t0);
    REQUIRE(msg.get_timestamp() == t0);

    msg.set_timestamp(TimePoint{});
    REQUIRE(msg.get_timestamp() == TimePoint{});
}

TEST_CASE("StoppedMessage: set_type()", "[Message]")
{
    Message msg;

    msg.set_type(Message::Type::step_started);
    REQUIRE(msg.get_type() == Message::Type::step_started);

    msg.set_type(Message::Type::log);
    REQUIRE(msg.get_type() == Message::Type::log);
}


/// send_message()

TEST_CASE("send_message() across threads", "[Message]")
{
    const auto timestamp = Clock::now();

    MessageQueue queue{ 4 };

    // Does nothing
    send_message(nullptr, Message::Type::log, "Test", timestamp);

    std::thread sender([=,&queue]()
        {
            for (int i = 1; i <= 100; ++i)
            {
                send_message(&queue, Message::Type::step_started, "start",
                    timestamp + std::chrono::seconds{ i });
                send_message(&queue, Message::Type::step_stopped, "stop",
                    timestamp + std::chrono::seconds{ i + 1 });
            }
        });

    for (int i = 1; i <= 100; ++i)
    {
        Message msg = queue.pop();
        REQUIRE(msg.get_type() == Message::Type::step_started);
        REQUIRE(msg.get_text() == "start");
        REQUIRE(msg.get_timestamp() == timestamp + std::chrono::seconds{ i });

        msg = queue.pop();
        REQUIRE(msg.get_type() == Message::Type::step_stopped);
        REQUIRE(msg.get_text() == "stop");
        REQUIRE(msg.get_timestamp() == timestamp + std::chrono::seconds{ i + 1 });
    }

    sender.join();
}
