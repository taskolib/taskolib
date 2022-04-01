/**
 * \file   test_Message.cc
 * \author Lars Froehlich
 * \date   Created on April 1, 2022
 * \brief  Test suite for the Message class.
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

#include <type_traits>
#include <gul14/catch.h>
#include "taskomat/Message.h"

using namespace task;

TEST_CASE("Message: Constructor", "[Error]")
{
    static_assert(not std::is_default_constructible<Message>::value,
        "Message is default-constructible although it should not be");
}
