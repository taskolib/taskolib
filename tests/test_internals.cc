/**
 * \file   test_internals.cc
 * \author Lars Fröhlich
 * \date   Created on November 4, 2022
 * \brief  Test suite for internal functions.
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

#include <gul14/cat.h>
#include <gul14/catch.h>

#include "internals.h"

using gul14::cat;
using namespace task;
using namespace Catch::Matchers;

TEST_CASE("remove_abort_markers()", "[internals]")
{
    SECTION("Empty message")
    {
        auto [msg, cause] = remove_abort_markers("");
        REQUIRE(cause == ErrorCause::uncaught_error);
        REQUIRE(msg.empty());
    }

    SECTION("Normal error message")
    {
        auto [msg, cause] = remove_abort_markers("This is an error message");
        REQUIRE(cause == ErrorCause::uncaught_error);
        REQUIRE(msg == "This is an error message");
    }

    SECTION("Script termination (abort marker(s) without message)")
    {
        SECTION("Single marker")
        {
            auto [msg, cause] = remove_abort_markers(abort_marker);
            REQUIRE(cause == ErrorCause::terminated_by_script);
            REQUIRE(msg == "Script called terminate_sequence()");
        }

        SECTION("Two markers")
        {
            auto [msg, cause] = remove_abort_markers(gul14::cat(abort_marker, abort_marker));
            REQUIRE(cause == ErrorCause::terminated_by_script);
            REQUIRE(msg == "Script called terminate_sequence()");
        }

        SECTION("Two markers surrounded by text")
        {
            auto [msg, cause] = remove_abort_markers(
                gul14::cat("lorem ipsum", abort_marker, abort_marker, "dolor sit"));
            REQUIRE(cause == ErrorCause::terminated_by_script);
            REQUIRE(msg == "Script called terminate_sequence()");
        }

        SECTION("Three markers surrounded by text")
        {
            auto [msg, cause] = remove_abort_markers(
                gul14::cat("lorem ipsum", abort_marker, abort_marker, "dolor sit", abort_marker, "amet"));
            REQUIRE(cause == ErrorCause::terminated_by_script);
            REQUIRE(msg == "Script called terminate_sequence()");
        }
    }

    SECTION("Abort with error message")
    {
        SECTION("Two markers")
        {
            auto [msg, cause] = remove_abort_markers(
                cat(abort_marker, "hydrogen", abort_marker));
            REQUIRE(cause == ErrorCause::aborted);
            REQUIRE(msg == "hydrogen");
        }

        SECTION("One marker at string start")
        {
            auto [msg, cause] = remove_abort_markers(
                cat(abort_marker, "helium"));
            REQUIRE(cause == ErrorCause::aborted);
            REQUIRE(msg == "helium");
        }

        SECTION("One marker at string end")
        {
            auto [msg, cause] = remove_abort_markers(
                cat("lithium", abort_marker));
            REQUIRE(cause == ErrorCause::aborted);
            REQUIRE(msg == "lithium");
        }

        SECTION("One marker embedded in string")
        {
            auto [msg, cause] = remove_abort_markers(
                cat("beryll", abort_marker, "ium"));
            REQUIRE(cause == ErrorCause::aborted);
            REQUIRE(msg == "beryllium");
        }

        SECTION("Two markers embedded in string")
        {
            auto [msg, cause] = remove_abort_markers(
                cat("waste of ", abort_marker, "boron", abort_marker, " sucks"));
            REQUIRE(cause == ErrorCause::aborted);
            REQUIRE(msg == "boron");
        }
    }
}
