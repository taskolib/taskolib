/**
 * \file   test_execute_step.cc
 * \author Lars Froehlich
 * \date   Created on December 20, 2021
 * \brief  Test suite for the execute_step() function.
 *
 * \copyright Copyright 2021 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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
#include "../include/avtomat/Error.h"
#include "../include/avtomat/execute_step.h"

using namespace avto;

TEST_CASE("execute_step(): Boolean return value from simple scripts", "[execute_step]")
{
    Context context;
    Step step;

    SECTION("Empty step returns false")
    {
        REQUIRE(execute_step(step, context) == false);
    }

    SECTION("'return true' returns true")
    {
        step.set_script("return true");
        REQUIRE(execute_step(step, context) == true);
    }

    SECTION("'return false' returns false")
    {
        step.set_script("return true");
        REQUIRE(execute_step(step, context) == true);
    }

    SECTION("'return nil' returns false")
    {
        step.set_script("return nil");
        REQUIRE(execute_step(step, context) == false);
    }

    SECTION("'return 42' returns true")
    {
        step.set_script("return true");
        REQUIRE(execute_step(step, context) == true);
    }
}

TEST_CASE("execute_step(): Exceptions", "[execute_step]")
{
    Context context;
    Step step;

    SECTION("Syntax error")
    {
        step.set_script("not a lua program");
        REQUIRE_THROWS_AS(execute_step(step, context), Error);
    }

    SECTION("Runtime error")
    {
        step.set_script("b = nil; b()");
        REQUIRE_THROWS_AS(execute_step(step, context), Error);
    }
}
