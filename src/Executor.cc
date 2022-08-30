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
#include "sol/sol.hpp"
#include "taskomat/Executor.h"
#include "lua_details.h"

using gul14::cat;
using namespace std::literals;

namespace task {

namespace {

void print_to_message_queue(const std::string& text, Message::IndexType idx, CommChannel* comm_channel)
{
    send_message(comm_channel, Message::Type::output, text, Clock::now(), idx);
}

void log_info_to_message_queue(const std::string& text, Message::IndexType idx, CommChannel* comm_channel)
{
    send_message(comm_channel, Message::Type::log_info, text, Clock::now(), idx);
}

void log_warning_to_message_queue(const std::string& text, Message::IndexType idx, CommChannel* comm_channel)
{
    send_message(comm_channel, Message::Type::log_warning, text, Clock::now(), idx);
}

void log_error_to_message_queue(const std::string& text, Message::IndexType idx, CommChannel* comm_channel)
{
    send_message(comm_channel, Message::Type::log_error, text, Clock::now(), idx);
}

} // anonymous namespace


Executor::Executor()
    : comm_channel_{ std::make_shared<CommChannel>() }
{
}

void Executor::cancel()
{
    if (not future_.valid())
        return;

    comm_channel_->immediate_termination_requested_ = true;
    future_.get(); // Wait for thread to join
    comm_channel_->immediate_termination_requested_ = false;
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
        comm->queue_.push(Message(Message::Type::sequence_stopped_with_error, e.what(),
                                  Clock::now(), 0));
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

void Executor::run_asynchronously(Sequence& sequence, Context context)
{
    if (future_.valid())
        throw Error("Busy executing another sequence");

    // Store a copy of the context for its local print and logging functions
    context_ = context;

    // Redirect the output functions used by the parallel thread to the message queue
    context.print_function = print_to_message_queue;
    context.log_info_function = log_info_to_message_queue;
    context.log_warning_function = log_warning_to_message_queue;
    context.log_error_function = log_error_to_message_queue;

    future_ = std::async(std::launch::async, execute_sequence, sequence,
                         std::move(context), comm_channel_);

    sequence.set_running(true);
    sequence.set_error_message("");
}

bool Executor::update(Sequence& sequence)
{
    // Read all messages that are currently in the queue
    while (const auto opt_msg = comm_channel_->queue_.try_pop())
    {
        const Message& msg = *opt_msg;
        const auto step_idx = msg.get_index();
        const auto step_it = sequence.begin() + step_idx;

        const bool was_running = sequence.is_running();

        switch (msg.get_type())
        {
        case Message::Type::output:
            if (context_.print_function)
                context_.print_function(msg.get_text(), step_idx, nullptr);
            break;
        case Message::Type::log_info:
            if (context_.log_info_function)
                context_.log_info_function(msg.get_text(), step_idx, nullptr);
            break;
        case Message::Type::log_warning:
            if (context_.log_warning_function)
                context_.log_warning_function(msg.get_text(), step_idx, nullptr);
            break;
        case Message::Type::log_error:
            if (context_.log_error_function)
                context_.log_error_function(msg.get_text(), step_idx, nullptr);
            break;
        case Message::Type::sequence_started:
            break;
        case Message::Type::sequence_stopped:
            sequence.set_running(false);
            break;
        case Message::Type::sequence_stopped_with_error:
            sequence.set_running(false);
            sequence.set_error_message(msg.get_text());
            if (context_.log_error_function)
                context_.log_error_function(msg.get_text(), step_idx, nullptr);
            break;
        case Message::Type::step_started:
            sequence.set_running(false); // temporarily allow modification
            sequence.modify(step_it, [ts = msg.get_timestamp()](Step& s) {
                s.set_running(true);
                s.set_time_of_last_execution(ts);
            });
            sequence.set_running(was_running);
            break;
        case Message::Type::step_stopped:
            sequence.set_running(false); // temporarily allow modification
            sequence.modify(step_it, [](Step& s) { s.set_running(false); });
            sequence.set_running(was_running);
            break;
        case Message::Type::step_stopped_with_error:
            sequence.set_running(false); // temporarily allow modification
            sequence.modify(step_it, [](Step& s) { s.set_running(false); });
            sequence.set_running(was_running);
            if (context_.log_error_function)
                context_.log_error_function(msg.get_text(), step_idx, nullptr);
            break;
        default:
            throw Error(cat("Unknown message type ", static_cast<int>(msg.get_type())));
        }
    }

    return is_busy();
}

} // namespace task
