/**
 * \file   CommChannel.h
 * \author Lars Froehlich
 * \date   Created on June 8, 2022
 * \brief  Declaration of the CommChannel struct and of the send_message() function.
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

#ifndef TASKOMAT_COMMCHANNEL_H_
#define TASKOMAT_COMMCHANNEL_H_

#include <atomic>
#include "taskomat/LockedQueue.h"
#include "taskomat/Message.h"

namespace task {

/**
 * A struct combining a message queue and several atomic flags.
 *
 * The message queue transports messages from a worker thread to the main thread.
 * The flags are used to send requests for various actions (e.g. termination) from
 * the main thread to the worker thread.
 */
struct CommChannel
{
    LockedQueue<Message> queue_{ 32 };
    std::atomic<bool> immediate_termination_requested_{ false };
};

/**
 * Enqueue a message in the given communication channel.
 *
 * \param comm_channel  Pointer to the communication channel. If this is null, the
 *                   function does nothing.
 * \param type       Message type (see Message::Type)
 * \param text       Message text
 * \param timestamp  Timestamp of the message
 * \param index      Index (of a Step in its parent Sequence)
 *
 * \code
 * send_message(comm_channel, type, text, timestamp, index);
 * // ... is equivalent to:
 * if (comm_channel)
 *     comm_channel->queue_.push(Message(type, text, timestamp, index));
 * \endcode
 */
inline
void send_message(CommChannel* comm_channel, Message::Type type, std::string text,
                  TimePoint timestamp, Message::IndexType index)
{
    if (comm_channel == nullptr)
        return;

    comm_channel->queue_.push(Message(type, std::move(text), timestamp, index));
}

} // namespace task

#endif
