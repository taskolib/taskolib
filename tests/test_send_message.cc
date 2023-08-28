/**
 * \file   test_send_message.cc
 * \author Lars Fröhlich
 * \date   Created on January 30, 2023, based on older code
 * \brief  Test suite for the send_message() function.
 *
 * \copyright Copyright 2022-2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <gul14/catch.h>

#include "send_message.h"
#include "taskolib/CommChannel.h"
#include "taskolib/Context.h"

using namespace task;
using namespace std::literals;

TEST_CASE("send_message() across threads", "[CommChannel][send_message]")
{
    const auto timestamp = Clock::now();

    CommChannel comm;
    Context context;
    context.message_callback_function = nullptr;

    // Does nothing
    send_message(Message::Type::output, "Test", timestamp, 0, context);

    std::thread sender([=,&comm,&context]()
        {
            for (int i = 1; i <= 100; ++i)
            {
                send_message(Message::Type::step_started, "start",
                    timestamp + std::chrono::seconds{ i }, i, context, &comm);
                send_message(Message::Type::step_stopped, "stop",
                    timestamp + std::chrono::seconds{ i + 1 }, i, context, &comm);
            }
        });

    for (int i = 1; i <= 100; ++i)
    {
        Message msg = comm.queue_.pop();
        REQUIRE(msg.get_type() == Message::Type::step_started);
        REQUIRE(msg.get_text() == "start");
        REQUIRE(msg.get_timestamp() == timestamp + std::chrono::seconds{ i });
        REQUIRE(msg.get_index().has_value());
        REQUIRE(*(msg.get_index()) == i);

        msg = comm.queue_.pop();
        REQUIRE(msg.get_type() == Message::Type::step_stopped);
        REQUIRE(msg.get_text() == "stop");
        REQUIRE(msg.get_timestamp() == timestamp + std::chrono::seconds{ i + 1 });
        REQUIRE(msg.get_index().has_value());
        REQUIRE(*(msg.get_index()) == i);
    }

    sender.join();
}
