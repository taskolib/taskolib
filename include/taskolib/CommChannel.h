/**
 * \file   CommChannel.h
 * \author Lars Fröhlich
 * \date   Created on June 8, 2022
 * \brief  Declaration of the CommChannel struct.
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

#ifndef TASKOLIB_COMMCHANNEL_H_
#define TASKOLIB_COMMCHANNEL_H_

#include <atomic>

#include "taskolib/LockedQueue.h"
#include "taskolib/Message.h"

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

} // namespace task

#endif
