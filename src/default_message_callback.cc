/**
 * \file   console.cc
 * \author Lars Fröhlich
 * \date   Created on June 22, 2022
 * \brief  Implementation of the default_message_callback() function.
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

#include <iostream>
#include "taskolib/default_message_callback.h"

namespace task {

void default_message_callback(const Message& msg)
{
    if (msg.get_type() == Message::Type::output)
        std::cout << msg.get_text();
}

} // namespace task
