/**
 * \file   send_message.cc
 * \author Lars Fröhlich
 * \date   Created on January 30, 2023, based on older code
 * \brief  Declaration of the send_message() function.
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

#include "send_message.h"

namespace task {

void send_message(Message::Type type, gul14::string_view text, TimePoint timestamp,
                  OptionalStepIndex index, const Context& context,
                  CommChannel* comm_channel)
{
    Message msg{ type, std::string(text), timestamp, index };

    if (context.message_callback_function)
        context.message_callback_function(msg);

    if (comm_channel == nullptr)
        return;

    comm_channel->queue_.push(std::move(msg));
}

} // namespace task
