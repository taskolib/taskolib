/**
 * \file   test_LockedQueue.cc
 * \author Lars Froehlich
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
#include "taskomat/Error.h"
#include "taskomat/Message.h"
#include "taskomat/LockedQueue.h"

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

TEMPLATE_TEST_CASE("LockedQueue: Constructor", "[Error]",
    int, std::string, MyMessage, std::unique_ptr<Message>)
{
    static_assert(std::is_default_constructible<TestType>::value,
        "LockedQueue is default-constructible");

    LockedQueue<TestType> queue;
}

TEST_CASE("LockedQueue: capacity()", "[Error]")
{
    SECTION("Default-constructed queue")
    {
        LockedQueue<std::unique_ptr<Message>> queue;
        REQUIRE(queue.capacity() > 0);
    }

    SECTION("Explicit capacity parameter")
    {
        LockedQueue<MyMessage> queue(42);
        REQUIRE(queue.capacity() == 42);
    }
}

TEMPLATE_TEST_CASE("LockedQueue: empty()", "[Error]",
    int, std::string, MyMessage, std::unique_ptr<Message>)
{
    LockedQueue<TestType> queue;
    REQUIRE(queue.empty() == true);

    queue.push(TestType{});
    REQUIRE(queue.empty() == false);

    queue.pop();
    REQUIRE(queue.empty() == true);
}

TEMPLATE_TEST_CASE("LockedQueue: pop() single-threaded", "[Error]",
    int, std::string, MyMessage, std::unique_ptr<Message>)
{
    LockedQueue<TestType> queue;
    REQUIRE(queue.size() == 0);

    queue.push(TestType{});
    queue.push(TestType{});
    REQUIRE(queue.size() == 2u);

    queue.pop();
    REQUIRE(queue.size() == 1u);

    queue.pop();
    REQUIRE(queue.size() == 0);
}

TEST_CASE("LockedQueue: push() single-threaded", "[Error]")
{
    LockedQueue<std::unique_ptr<Message>> queue;
    REQUIRE(queue.size() == 0);

    queue.push(std::make_unique<MyMessage>(42));
    REQUIRE(queue.size() == 1u);

    queue.push(std::make_unique<MyMessage>(43));
    REQUIRE(queue.size() == 2u);

    auto msg_ptr = queue.pop();
    REQUIRE(queue.size() == 1u);
    REQUIRE(msg_ptr != nullptr);
    auto mymsg_ptr = dynamic_cast<MyMessage*>(msg_ptr.get());
    REQUIRE(mymsg_ptr != nullptr);
    REQUIRE(mymsg_ptr->value_ == 42);

    msg_ptr = queue.pop();
    REQUIRE(queue.size() == 0);
    REQUIRE(msg_ptr != nullptr);
    mymsg_ptr = dynamic_cast<MyMessage*>(msg_ptr.get());
    REQUIRE(mymsg_ptr != nullptr);
    REQUIRE(mymsg_ptr->value_ == 43);
}

TEST_CASE("LockedQueue: push() & pop() across threads", "[Error]")
{
    LockedQueue<std::unique_ptr<Message>> queue(4);

    std::thread sender([&queue]()
        {
            for (int i = 1; i <= 100; ++i)
                queue.push(std::make_unique<MyMessage>(i));
        });

    for (int i = 1; i <= 100; ++i)
    {
        auto msg_ptr = queue.pop();
        auto mymsg_ptr = dynamic_cast<MyMessage*>(msg_ptr.get());
        REQUIRE(mymsg_ptr != nullptr);
        REQUIRE(mymsg_ptr->value_ == i);
    }

    sender.join();
}

TEST_CASE("LockedQueue: size()", "[Error]")
{
    LockedQueue<std::unique_ptr<Message>> queue;
    REQUIRE(queue.size() == 0);

    queue.push(std::make_unique<MyMessage>());
    REQUIRE(queue.size() == 1u);

    queue.push(std::make_unique<MyMessage>());
    REQUIRE(queue.size() == 2u);

    queue.pop();
    REQUIRE(queue.size() == 1u);

    queue.pop();
    REQUIRE(queue.size() == 0);
}
