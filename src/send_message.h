/**
 * \file   send_message.h
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

#ifndef TASKOLIB_SEND_MESSAGE_H_
#define TASKOLIB_SEND_MESSAGE_H_

#include <gul14/string_view.h>

#include "taskolib/CommChannel.h"
#include "taskolib/Context.h"
#include "taskolib/Message.h"
#include "taskolib/StepIndex.h"

namespace task {

/**
 * Call the message callback and enqueue the message in the given communication channel,
 * if any.
 *
 * \param type          Message type (see Message::Type)
 * \param text          Message text
 * \param timestamp     Timestamp of the message
 * \param index         An optional index (of a Step in its parent Sequence)
 * \param context       Context with a message callback function
 * \param comm_channel  Pointer to the communication channel. If this is null, the
 *                      function does not attempt to push the message into any message
 *                      queue.
 */
void send_message(Message::Type type, gul14::string_view text, TimePoint timestamp,
                  OptionalStepIndex index, const Context& context,
                  CommChannel* comm_channel = nullptr);

} // namespace task

#endif
