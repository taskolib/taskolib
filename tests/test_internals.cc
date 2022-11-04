/**
 * \file   test_internals.cc
 * \author Lars Froehlich
 * \date   Created on November 4, 2022
 * \brief  Test suite for internal functions.
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

#include <gul14/cat.h>
#include <gul14/catch.h>

#include "../src/internals.h"

using gul14::cat;
using namespace task;
using namespace Catch::Matchers;

TEST_CASE("remove_abort_markers_from_error_message()", "[internals]")
{
    std::string msg;

    SECTION("Empty message")
    {
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::uncaught_error);
        REQUIRE(msg.empty());
    }

    SECTION("Normal error message")
    {
        msg = "This is an error message";
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::uncaught_error);
        REQUIRE(msg == "This is an error message");
    }

    SECTION("Script termination (abort marker(s) without message)")
    {
        msg = std::string(abort_marker);
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::terminated_by_script);
        REQUIRE(msg == "Script called terminate_sequence()");

        msg = gul14::cat(abort_marker, abort_marker);
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::terminated_by_script);
        REQUIRE(msg == "Script called terminate_sequence()");

        msg = gul14::cat("lorem ipsum", abort_marker, abort_marker, "dolor sit");
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::terminated_by_script);
        REQUIRE(msg == "Script called terminate_sequence()");

        msg = gul14::cat("lorem ipsum", abort_marker, abort_marker, "dolor sit", abort_marker, "amet");
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::terminated_by_script);
        REQUIRE(msg == "Script called terminate_sequence()");
    }

    SECTION("Abort with error message")
    {
        msg = cat(abort_marker, "hydrogen", abort_marker);
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::aborted);
        REQUIRE(msg == "hydrogen");

        msg = cat(abort_marker, "helium");
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::aborted);
        REQUIRE(msg == "helium");

        msg = cat("lithium", abort_marker);
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::aborted);
        REQUIRE(msg == "lithium");

        msg = cat("beryll", abort_marker, "ium");
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::aborted);
        REQUIRE(msg == "beryllium");

        msg = cat("waste of ", abort_marker, "boron", abort_marker, " sucks");
        REQUIRE(remove_abort_markers_from_error_message(msg) == ErrorCause::aborted);
        REQUIRE(msg == "boron");
    }
}
