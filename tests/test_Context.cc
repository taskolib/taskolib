/**
 * \file   test_Context.cc
 * \author Lars Froehlich
 * \date   Created on December 20, 2021
 * \brief  Test suite for the Context class.
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

#include <gul14/catch.h>
#include "taskolib/Context.h"

using namespace std::literals;
using namespace task;

TEST_CASE("Context: Constructor", "[Context]")
{
    static_assert(std::is_default_constructible<Context>::value,
                  "Context is_default_constructible");

    Context c;
}

TEST_CASE("Context: Copy constructor", "[Context]")
{
    static_assert(std::is_copy_constructible<Context>::value,
                  "Context is_copy_constructible");

    Context c;
    Context c2{ c };
}

TEST_CASE("Context: Move constructor", "[Context]")
{
    static_assert(std::is_nothrow_move_constructible<Context>::value,
                  "Context is_nothrow_move_constructible");

    Context c;
    Context c2{ std::move(c) };
}

TEST_CASE("Context: Copy assignment", "[Context]")
{
    static_assert(std::is_copy_assignable<Context>::value, "Context is_copy_assignable");

    Context c;
    Context c2;

    c2 = c;
}

TEST_CASE("Context: Move assignment", "[Context]")
{
    static_assert(std::is_nothrow_move_assignable<Context>::value,
                  "Context is_nothrow_move_assignable");

    Context c;
    Context c2;

    c2 = std::move(c);
}

TEST_CASE("Check context variable assignment", "[Context]")
{
    Context c;
    // Never assign a char* like this: c.variables["b"] = "BooleanTest";
    // Some compilers fill this into the bool alternative!
    // See PR #33 for details.
    c.variables["b"] = "BooleanTest"s; // assigns std::string
    REQUIRE(std::holds_alternative<bool>(c.variables["b"]) == false);
    REQUIRE(std::holds_alternative<std::string>(c.variables["b"]) == true);
    REQUIRE(std::get<std::string>(c.variables["b"]) == "BooleanTest");
}
