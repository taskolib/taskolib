/**
 * \file   Executor.cc
 * \author Lars Froehlich
 * \date   Created on May 30, 2022
 * \brief  Implementation of the Executor class.
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

#include <gul14/cat.h>
#include "taskomat/Executor.h"

using gul14::cat;
using namespace std::literals;

namespace task {

Executor::Executor()
    : comm_channel_{ std::make_shared<CommChannel>() }
{
}

void Executor::execute_sequence(Sequence sequence, Context context,
                                std::shared_ptr<CommChannel> comm) noexcept
{
    try
    {
        sequence.execute(context, comm.get());
    }
    catch (const std::exception& e)
    {
        comm->queue_.push(Message(Message::Type::sequence_stopped_with_error,
            cat("Sequence stopped with error: ", e.what()), Clock::now(), 0));
    }
}

bool Executor::is_busy()
{
    if (not future_.valid())
        return false;

    const auto status = future_.wait_for(0s);

    if (status == std::future_status::timeout)
        return true;

    future_.get();
    return false;
}

void Executor::run_asynchronously(Sequence sequence, Context context)
{
    if (future_.valid())
        throw Error("Busy executing another sequence");

    future_ = std::async(std::launch::async, execute_sequence, std::move(sequence),
                         std::move(context), comm_channel_);
}

bool Executor::update(Sequence& sequence)
{
    // Read all messages that are currently in the queue
    while (const auto opt_msg = comm_channel_->queue_.try_pop())
    {
        const Message& msg = *opt_msg;
        const auto step_idx = msg.get_index();
        const auto step_it = sequence.begin() + step_idx;

        switch (msg.get_type())
        {
        case Message::Type::log:
            break;
        case Message::Type::sequence_started:
            break;
        case Message::Type::sequence_stopped:
            break;
        case Message::Type::sequence_stopped_with_error:
            break;
        case Message::Type::step_started:
            sequence.modify(step_it, [](Step& s) { s.set_running(true); });
            break;
        case Message::Type::step_stopped:
            sequence.modify(step_it, [](Step& s) { s.set_running(false); });
            break;
        case Message::Type::step_stopped_with_error:
            sequence.modify(step_it, [](Step& s) { s.set_running(false); });
            break;
        default:
            throw Error(cat("Unknown message type ", static_cast<int>(msg.get_type())));
        }
    }

    return is_busy();
}

} // namespace task
