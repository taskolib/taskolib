/**
 * \file   test_exceptions.cc
 * \author Lars Froehlich
 * \date   Created on December 10, 2021
 * \brief  Test suite for the Error and ErrorAtIndex exception classes.
 *
 * \copyright Copyright 2021-2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <string>
#include <gul14/catch.h>
#include "taskolib/exceptions.h"

using namespace std::literals;
using namespace task;

TEST_CASE("Error: Constructor", "[exceptions]")
{
    Error e("Test");
}

TEST_CASE("Error: Copy constructor", "[exceptions]")
{
    Error e("Test");
    Error e2(e);

    REQUIRE(e.what() == std::string(e2.what()));
}

TEST_CASE("Error: Copy assignment", "[exceptions]")
{
    Error e("Test");
    Error e2("Test2");

    e2 = e;

    REQUIRE(e2.what() == "Test"s);
}

TEST_CASE("ErrorAtIndex: Constructor", "[exceptions]")
{
    ErrorAtIndex e("Test", 0);
}

TEST_CASE("ErrorAtIndex: Copy constructor", "[exceptions]")
{
    ErrorAtIndex e("Test", 42);
    ErrorAtIndex e2(e);

    REQUIRE(e.what() == std::string(e2.what()));
    REQUIRE(e.get_index() == e2.get_index());
}

TEST_CASE("ErrorAtIndex: Copy assignment", "[exceptions]")
{
    ErrorAtIndex e("Test", 1);
    ErrorAtIndex e2("Test2", 2);

    e2 = e;

    REQUIRE(e2.what() == "Test"s);
    REQUIRE(e2.get_index() == 1);
}
