/**
 * \file   Message.h
 * \author Lars Froehlich
 * \date   Created on April 1, 2022
 * \brief  Declaration of the Message class.
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

#ifndef TASKOMAT_MESSAGE_H_
#define TASKOMAT_MESSAGE_H_

namespace task {

/**
 * Base class for messages to be used with the LockedQueue.
 */
class Message
{
public:
    /// Virtual destructor.
    virtual ~Message() {};

protected:
    /// Protected default constructor (the Message class cannot be instantiated directly).
    Message() = default;
};

} // namespace task

#endif
