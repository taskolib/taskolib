/**
 * \file   test_exceptions.cc
 * \author Lars Fröhlich
 * \date   Created on December 10, 2021
 * \brief  Test suite for the Error exception class.
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
    SECTION("Single argument")
    {
        Error e("Test");
        REQUIRE(e.what() == "Test"s);
        REQUIRE(e.get_index().has_value() == false);
    }

    SECTION("Two arguments")
    {
        Error e("tesT", 42);
        REQUIRE(e.what() == "tesT"s);
        REQUIRE(e.get_index().has_value() == true);
        REQUIRE(e.get_index().value() == 42);
    }
}

TEST_CASE("Error: Copy constructor", "[exceptions]")
{
    Error e("Test", 42);
    Error e2(e);

    REQUIRE(e.what() == std::string(e2.what()));
    REQUIRE(e.get_index() == e2.get_index());
}

TEST_CASE("Error: Copy assignment", "[exceptions]")
{
    const Error e("Test", 1);
    Error e2("Test2", 2);

    e2 = e;

    REQUIRE(e2.what() == "Test"s);
    REQUIRE(e2.get_index() == e.get_index());
}

TEST_CASE("Error: operator==", "[exceptions]")
{
    REQUIRE(Error("Test", 42) == Error("Test", 42));
    REQUIRE(Error("Test", gul14::nullopt) == Error("Test", gul14::nullopt));
    REQUIRE(Error("", 42) == Error("", 42));

    REQUIRE_FALSE(Error("test", 42) == Error("TEST", 42));
    REQUIRE_FALSE(Error("test", 42) == Error("test", 23));
    REQUIRE_FALSE(Error("Test", 13) == Error("Test", gul14::nullopt));
    REQUIRE_FALSE(Error(" ", gul14::nullopt) == Error("", gul14::nullopt));
}

TEST_CASE("Error: operator!=", "[exceptions]")
{
    REQUIRE_FALSE(Error("Test", 42) != Error("Test", 42));
    REQUIRE_FALSE(Error("Test", gul14::nullopt) != Error("Test", gul14::nullopt));
    REQUIRE_FALSE(Error("", 42) != Error("", 42));

    REQUIRE(Error("test", 42) != Error("TEST", 42));
    REQUIRE(Error("test", 42) != Error("test", 23));
    REQUIRE(Error("Test", 13) != Error("Test", gul14::nullopt));
    REQUIRE(Error(" ", gul14::nullopt) != Error("", gul14::nullopt));
}
