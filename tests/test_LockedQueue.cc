/**
 * \file   test_LockedQueue.cc
 * \author Lars Fröhlich
 * \date   Created on April 1, 2022
 * \brief  Test suite for the LockedQueue class template.
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
#include <gul14/time_util.h>
#include "taskolib/exceptions.h"
#include "taskolib/Message.h"
#include "taskolib/LockedQueue.h"

using namespace task;

namespace {

class MyMessage : public Message
{
public:
    int value_;

    MyMessage(int v = 0) : value_{ v }
    {}
};

} // anonymous namespace

TEMPLATE_TEST_CASE("LockedQueue: Constructor", "[LockedQueue]",
    int, std::string, MyMessage, std::unique_ptr<Message>)
{
    static_assert(std::is_constructible<LockedQueue<TestType>, uint32_t>::value,
        "LockedQueue<TestType> is constructible");

    LockedQueue<TestType> queue{ 4 };
}

TEST_CASE("LockedQueue: capacity()", "[LockedQueue]")
{
    SECTION("Default-constructed queue")
    {
        LockedQueue<Message> queue{ 10 };
        REQUIRE(queue.capacity() > 0);
    }

    SECTION("Explicit capacity parameter")
    {
        LockedQueue<MyMessage> queue(42);
        REQUIRE(queue.capacity() == 42);
    }
}

TEMPLATE_TEST_CASE("LockedQueue: empty()", "[LockedQueue]",
    int, std::string, MyMessage, std::unique_ptr<Message>)
{
    LockedQueue<TestType> queue{ 10 };
    REQUIRE(queue.empty() == true);

    queue.push(TestType{});
    REQUIRE(queue.empty() == false);

    queue.pop();
    REQUIRE(queue.empty() == true);
}

TEMPLATE_TEST_CASE("LockedQueue: pop() single-threaded", "[LockedQueue]",
    int, std::string, MyMessage, std::unique_ptr<Message>)
{
    LockedQueue<TestType> queue{ 10 };
    REQUIRE(queue.size() == 0);

    queue.push(TestType{});
    queue.push(TestType{});
    REQUIRE(queue.size() == 2u);

    queue.pop();
    REQUIRE(queue.size() == 1u);

    queue.pop();
    REQUIRE(queue.size() == 0);
}

TEST_CASE("LockedQueue: push() single-threaded", "[LockedQueue]")
{
    LockedQueue<MyMessage> queue{ 10 };
    REQUIRE(queue.size() == 0);

    queue.push(MyMessage(42));
    REQUIRE(queue.size() == 1u);

    queue.push(MyMessage(43));
    REQUIRE(queue.size() == 2u);

    auto msg = queue.pop();
    REQUIRE(queue.size() == 1u);
    REQUIRE(msg.value_ == 42);

    msg = queue.pop();
    REQUIRE(queue.size() == 0);
    REQUIRE(msg.value_ == 43);
}

TEST_CASE("LockedQueue: push() & pop() across threads", "[LockedQueue]")
{
    // Create a queue with only 4 slots
    LockedQueue<MyMessage> queue{ 4 };

    // Start a thread that will push 100 messages into the queue
    std::thread sender([&queue]()
        {
            for (int i = 1; i <= 100; ++i)
                queue.push(MyMessage(i));
        });

    gul14::sleep(0.005);
    // Pull all 100 messages out of the queue from the main thread
    for (int i = 1; i <= 100; ++i)
    {
        auto msg = queue.pop();
        REQUIRE(msg.value_ == i);
    }

    sender.join();
}

TEST_CASE("LockedQueue: size()", "[LockedQueue]")
{
    LockedQueue<Message> queue{ 10 };
    REQUIRE(queue.size() == 0);

    queue.push(MyMessage{});
    REQUIRE(queue.size() == 1u);

    queue.push(MyMessage{});
    REQUIRE(queue.size() == 2u);

    queue.pop();
    REQUIRE(queue.size() == 1u);

    queue.pop();
    REQUIRE(queue.size() == 0);
}

TEST_CASE("LockedQueue: try_pop() single-threaded", "[LockedQueue]")
{
    LockedQueue<int> queue{ 2 };
    REQUIRE(queue.size() == 0);

    REQUIRE(queue.try_pop() == gul14::nullopt);

    queue.push(1);
    queue.push(2);
    REQUIRE(queue.size() == 2u);

    auto opt = queue.try_pop();
    REQUIRE(opt.has_value());
    REQUIRE(*opt == 1);

    opt = queue.try_pop();
    REQUIRE(opt.has_value());
    REQUIRE(*opt == 2);

    REQUIRE(queue.try_pop() == gul14::nullopt);
}

TEST_CASE("LockedQueue: try_push() single-threaded", "[LockedQueue]")
{
    LockedQueue<int> queue{ 2 };
    REQUIRE(queue.size() == 0);

    REQUIRE(queue.try_push(1) == true);
    REQUIRE(queue.size() == 1u);

    REQUIRE(queue.try_push(2) == true);
    REQUIRE(queue.size() == 2u);

    REQUIRE(queue.try_push(3) == false); // queue full
    REQUIRE(queue.size() == 2u);

    int msg = queue.pop();
    REQUIRE(queue.size() == 1u);
    REQUIRE(msg == 1);

    REQUIRE(queue.try_push(3) == true);
    REQUIRE(queue.size() == 2u);

    msg = queue.pop();
    REQUIRE(queue.size() == 1u);
    REQUIRE(msg == 2);

    msg = queue.pop();
    REQUIRE(queue.empty() == true);
    REQUIRE(msg == 3);
}

TEST_CASE("LockedQueue: try_push() & try_pop() across threads", "[LockedQueue]")
{
    // Create a queue with only 4 slots
    LockedQueue<MyMessage> queue{ 4 };

    // Start a thread that will push 100 messages into the queue
    std::thread sender([&queue]()
        {
            for (int i = 1; i <= 100; ++i)
            {
                while (queue.try_push(MyMessage(i)) == false);
            }
        });

    // Pull all 100 messages out of the queue from the main thread
    for (int i = 1; i <= 100; ++i)
    {
        gul14::optional<MyMessage> opt_msg;
        do
        {
            opt_msg = queue.try_pop();
        }
        while (not opt_msg.has_value());

        auto msg = *opt_msg;
        REQUIRE(msg.value_ == i);
    }

    sender.join();
}

TEST_CASE("LockedQueue: back()", "[LockedQueue]")
{
    LockedQueue<MyMessage> queue{ 2 };

    queue.push(MyMessage(1));
    auto msg_1 = queue.back();
    REQUIRE(queue.size() == 1);
    REQUIRE(msg_1.value_ == 1);

    queue.push(MyMessage(2));
    REQUIRE(queue.size() == 2);
    auto msg_2 = queue.back();
    REQUIRE(queue.size() == 2);
    REQUIRE(msg_2.value_ == 2);

    auto msg_pop = queue.pop();
    REQUIRE(queue.size() == 1);
    REQUIRE(msg_pop.value_ == 1);
    auto msg_back = queue.back();
    REQUIRE(queue.size() == 1);
    REQUIRE(msg_back.value_ == 2);
}
