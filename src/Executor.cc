/**
 * \file   Executor.cc
 * \author Lars Froehlich, Ulf Fini Jastrow, Marcus Walla
 * \date   Created on May 30, 2022
 * \brief  Implementation of the Executor class.
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

#include <gul14/cat.h>

#include "lua_details.h"
#include "sol/sol.hpp"
#include "taskolib/Executor.h"

using gul14::cat;
using namespace std::literals;

namespace task {

namespace {

// The sequence/single-step execution function to be started with launch_async_execution().
//
// If step_index is nullopt, this function calls Sequence::execute() to run the entire
// sequence. If step_index contains a step index, it calls
// Sequence::execute_single_step(). In both cases, all exceptions are silently
//
// \param sequence        The Sequence to be started
// \param context         The Context under which the sequence should run
// \param comm            Shared pointer to a CommChannel for communication (can be null)
// \param opt_step_index  The index of the step to be started in isolation or nullopt to
//                        start the entire sequence
VariableTable execute_sequence(Sequence sequence, Context context,
    std::shared_ptr<CommChannel> comm, OptionalStepIndex opt_step_index) noexcept
{
    // Ignore any returned errors - the sequence already takes care of sending the
    // appropriate messages.
    (void)sequence.execute(context, comm.get(), opt_step_index);

    return context.variables;
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
    while (comm_channel_->queue_.try_pop());
    context_.variables = future_.get(); // Wait for thread to join
    comm_channel_->immediate_termination_requested_ = false;
}

void Executor::cancel(Sequence& sequence) {
    if (not future_.valid())
        return;
    comm_channel_->immediate_termination_requested_ = true;
    while(update(sequence));
    if (future_.valid())
        context_.variables = future_.get();
    comm_channel_->immediate_termination_requested_ = false; // Successfully terminated, rearm comm_channel
}

bool Executor::is_busy()
{
    if (not future_.valid())
        return false;

    const auto status = future_.wait_for(0s);

    if (status == std::future_status::timeout)
        return true;

    context_.variables = future_.get();
    return false;
}

void Executor::launch_async_execution(Sequence& sequence, Context context,
                                      OptionalStepIndex step_index)
{
    if (future_.valid())
        throw Error("Busy executing another sequence");

    // Store a copy of the context for its local print and logging functions
    context_ = context;

    // Disable any message callbacks in the worker thread
    context.message_callback_function = nullptr;

    future_ = std::async(std::launch::async, execute_sequence, sequence,
                         std::move(context), comm_channel_, step_index);

    sequence.set_running(true);
    sequence.set_error(gul14::nullopt);
}

void Executor::run_asynchronously(Sequence& sequence, Context context)
{
    launch_async_execution(sequence, context, gul14::nullopt);
}

void Executor::run_single_step_asynchronously(Sequence& sequence, Context context,
                                              StepIndex step_index)
{
    if (step_index >= sequence.size())
        throw Error(cat("Invalid step index ", step_index));

    launch_async_execution(sequence, context, step_index);
}

bool Executor::update(Sequence& sequence)
{
    // Read all messages that are currently in the queue
    while (const auto opt_msg = comm_channel_->queue_.try_pop())
    {
        const Message& msg = *opt_msg;
        const OptionalStepIndex step_idx = msg.get_index();

        const auto modify_step =
            [&sequence, step_idx](auto fct)
            {
                if (!step_idx)
                    throw Error("Missing step index");

                const bool was_running = sequence.is_running();
                sequence.set_running(false); // temporarily allow modification
                sequence.modify(sequence.begin() + *step_idx, fct);
                sequence.set_running(was_running);
            };

        if (context_.message_callback_function)
            context_.message_callback_function(msg);

        switch (msg.get_type())
        {
        case Message::Type::output:
            break; // only triggers callback
        case Message::Type::sequence_started:
            break; // only triggers callback
        case Message::Type::sequence_stopped:
            sequence.set_running(false);
            break;
        case Message::Type::sequence_stopped_with_error:
            sequence.set_running(false);
            sequence.set_error(Error{ msg.get_text(), msg.get_index() });
            break;
        case Message::Type::step_started:
            modify_step([ts = msg.get_timestamp()](Step& s)
                {
                    s.set_running(true);
                    s.set_time_of_last_execution(ts);
                });
            break;
        case Message::Type::step_stopped:
            modify_step([](Step& s) { s.set_running(false); });
            break;
        case Message::Type::step_stopped_with_error:
            modify_step([](Step& s) { s.set_running(false); });
            break;
        default:
            throw Error(cat("Unknown message type ", static_cast<int>(msg.get_type())));
        }
    }

    return is_busy();
}

} // namespace task
