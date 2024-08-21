/**
 * \file   test_deserialize_sequence.cc
 * \author Lars Fröhlich, Marcus Walla
 * \date   Created on July 26, 2023
 * \brief  Test suite for free functions declared in deserialize_sequence.h.
 *
 * \copyright Copyright 2023-2024 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <filesystem>
#include <fstream>

#include <gul14/catch.h>

#include "deserialize_sequence.h"
#include "internals_unit_test.h"
#include "internals.h"

using namespace task;
using namespace std::literals;

TEST_CASE("parse_tags()", "[deserialize_sequence]")
{
    REQUIRE(parse_tags("").empty());
    REQUIRE(parse_tags(" tag1 tag2\ttag3\n")
        == std::vector{ Tag{ "tag1" }, Tag{ "tag2" }, Tag{ "tag3" } });
    REQUIRE(parse_tags("c    Yet-Another-Tag")
        == std::vector{ Tag{ "c" }, Tag{ "yet-another-tag" } });
    REQUIRE_THROWS_AS(parse_tags("tag*"), Error);
    REQUIRE_THROWS_AS(parse_tags("tag2 tag3 Yet-Another-Tag NOT_ALLOWED"), Error);
}

TEST_CASE("parse_timeout()", "[deserialize_sequence]")
{
    REQUIRE(parse_timeout("0") == Timeout{ 0ms });
    REQUIRE(parse_timeout("10") == Timeout{ 10ms });
    REQUIRE(parse_timeout("30000") == Timeout{ 30s });
    REQUIRE(parse_timeout(" 42 \t\n") == Timeout{ 42ms });
    REQUIRE(parse_timeout("86400000") == Timeout{ 24h });
    REQUIRE(parse_timeout("infinite") == Timeout::infinity());
    REQUIRE(parse_timeout("Infinite") == Timeout::infinity());
    REQUIRE(parse_timeout("INFINITE") == Timeout::infinity());
    REQUIRE(parse_timeout("\ninfinite \t") == Timeout::infinity());

    REQUIRE_THROWS_AS(parse_timeout(""), Error);
    REQUIRE_THROWS_AS(parse_timeout("hello"), Error);
    REQUIRE_THROWS_AS(parse_timeout("10s"), Error);
    REQUIRE_THROWS_AS(parse_timeout("10 us"), Error);
    REQUIRE_THROWS_AS(parse_timeout("-10"), Error);

}

TEST_CASE("parse_bool()", "[deserialize_sequence]")
{
    REQUIRE(parse_bool("true"));
    REQUIRE_FALSE(parse_bool("false"));

    REQUIRE_THROWS_AS(parse_bool("TRUE"), Error);
    REQUIRE_THROWS_AS(parse_bool(""), Error);
    REQUIRE_THROWS_AS(parse_bool("1"), Error);
}

TEST_CASE("Ancient sequence", "[deserialize_sequence]")
{
    std::filesystem::path path = temp_dir / sequence_lua_filename;
    Sequence seq;

    if (not std::filesystem::exists(temp_dir))
        std::filesystem::create_directory(temp_dir);

    SECTION("Ancient sequence without parameter autorun and disable")
    {
        std::ofstream stream(path);

        if (stream.fail())
            FAIL(gul14::cat("Could not create file: " + path.string()));

        stream << "-- label: some label\n";
        stream.close();

        load_sequence_parameters(temp_dir, seq);
        REQUIRE_FALSE(seq.get_autorun());
        REQUIRE_FALSE(seq.is_disabled());
    }

    SECTION("Ancient sequence without parameter disable")
    {
        std::ofstream stream(path);

        if (stream.fail())
            FAIL(gul14::cat("Could not create file: " + path.string()));

        stream << "-- label: some label\n";
        stream << "-- autorun: true\n";
        stream.close();

        load_sequence_parameters(temp_dir, seq);
        REQUIRE(seq.get_autorun());
        REQUIRE_FALSE(seq.is_disabled());
    }

    SECTION("Ancient sequence without parameter autorun")
    {
        std::ofstream stream(path);

        if (stream.fail())
            FAIL(gul14::cat("Could not create file: " + path.string()));

        stream << "-- label: some label\n";
        stream << "-- disabled: true\n";
        stream.close();

        load_sequence_parameters(temp_dir, seq);
        REQUIRE_FALSE(seq.get_autorun());
        REQUIRE(seq.is_disabled());
    }
}
