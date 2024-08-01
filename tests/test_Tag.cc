/**
 * \file   test_Tag.cc
 * \author Lars Fröhlich
 * \date   Created on July 17, 2024
 * \brief  Test suite for the Tag class.
 *
 * \copyright Copyright 2024 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <sstream>

#include <gul14/catch.h>
#include <gul14/join_split.h>

#include "taskolib/exceptions.h"
#include "taskolib/Tag.h"

using namespace std::literals;
using namespace task;

TEST_CASE("Tag: Default constructor", "[Tag]")
{
    Tag name;
    REQUIRE(name.string() == "-");
}

TEST_CASE("Tag: Constructor from string", "[Tag]")
{
    Tag a{ "1234" };
    Tag b{ "extremely-weird-Combination" };
    Tag c{ "-1-a-B-C-" };

    REQUIRE_THROWS_AS(Tag{ "" }, Error);
    REQUIRE_THROWS_AS(Tag{ std::string(Tag::max_length + 1, 'a') }, Error);
    REQUIRE_THROWS_AS(Tag{ "string with whitespace" }, Error);
    REQUIRE_THROWS_AS(Tag{ "abcd#e" }, Error);
    REQUIRE_THROWS_AS(Tag{ "abcd(e)" }, Error);
    REQUIRE_THROWS_AS(Tag{ "abcd[e]" }, Error);
    REQUIRE_THROWS_AS(Tag{ ".abcd" }, Error);
}

TEST_CASE("Tag: operator==()", "[Tag]")
{
    REQUIRE(Tag{ "1234" } == Tag{ "1234" });
    REQUIRE(Tag{ "Gulag" } == Tag{ "gulag" });
    REQUIRE_FALSE(Tag{ "hallo" } == Tag{ "hello" });
}

TEST_CASE("Tag: operator!=()", "[Tag]")
{
    REQUIRE_FALSE(Tag{ "1234" } != Tag{ "1234" });
    REQUIRE_FALSE(Tag{ "Gulag" } != Tag{ "gulag" });
    REQUIRE(Tag{ "hallo" } != Tag{ "hello" });
}

TEST_CASE("Tag: operator>()", "[Tag]")
{
    REQUIRE(Tag{ "12" } > Tag{ "11" });
    REQUIRE(Tag{ "banana" } > Tag{ "apple" });
    REQUIRE_FALSE(Tag{ "12" } > Tag{ "21" });
    REQUIRE_FALSE(Tag{ "banana" } > Tag{ "cherry" });
    REQUIRE_FALSE(Tag{ "apple" } > Tag{ "apple" });
}

TEST_CASE("Tag: operator>=()", "[Tag]")
{
    REQUIRE(Tag{ "12" } >= Tag{ "11" });
    REQUIRE(Tag{ "banana" } >= Tag{ "apple" });
    REQUIRE_FALSE(Tag{ "12" } >= Tag{ "21" });
    REQUIRE_FALSE(Tag{ "banana" } >= Tag{ "cherry" });
    REQUIRE(Tag{ "apple" } >= Tag{ "apple" });
}

TEST_CASE("Tag: operator<()", "[Tag]")
{
    REQUIRE_FALSE(Tag{ "12" } < Tag{ "11" });
    REQUIRE_FALSE(Tag{ "banana" } < Tag{ "apple" });
    REQUIRE(Tag{ "12" } < Tag{ "21" });
    REQUIRE(Tag{ "banana" } < Tag{ "cherry" });
    REQUIRE_FALSE(Tag{ "apple" } < Tag{ "apple" });
}

TEST_CASE("Tag: operator<=()", "[Tag]")
{
    REQUIRE_FALSE(Tag{ "12" } <= Tag{ "11" });
    REQUIRE_FALSE(Tag{ "banana" } <= Tag{ "apple" });
    REQUIRE(Tag{ "12" } <= Tag{ "21" });
    REQUIRE(Tag{ "banana" } <= Tag{ "cherry" });
    REQUIRE(Tag{ "apple" } <= Tag{ "apple" });
}

TEST_CASE("operator+(string, Tag)", "[Tag]")
{
    REQUIRE("Hello "s + Tag{ "world" } == "Hello world");
}

TEST_CASE("operator+(Tag, string)", "[Tag]")
{
    REQUIRE(Tag{ "gung" } + "-ho"s == "gung-ho");
}

TEST_CASE("operator+=(string, Tag)", "[Tag]")
{
    std::string str = "Hello ";
    str += Tag{ "world" };
    REQUIRE(str == "Hello world");
}

TEST_CASE("operator<<(std::ostream&, Tag)", "[Tag]")
{
    std::ostringstream stream;
    stream << Tag{ "bloody" } << " " << Tag{ "sunday" };
    REQUIRE(stream.str() == "bloody sunday");
}

TEST_CASE("Tag: size()", "[Tag]")
{
    REQUIRE(Tag{ "1234" }.size() == 4);
    REQUIRE(Tag{ "Gulag" }.size() == 5);
    REQUIRE(Tag{ "hallo-welt" }.size() == 10);
}

TEST_CASE("Tag: string()", "[Tag]")
{
    REQUIRE(Tag{ "1234" }.string() == "1234");
    REQUIRE(Tag{ "Extremely-Weird-Combination" }.string()
            == "extremely-weird-combination");
    REQUIRE(Tag{ "-1-a-B-C-" }.string() == "-1-a-b-c-");
}

TEST_CASE("Tag: Usability with gul14::join()", "[Tag]")
{
    std::vector tags{ Tag{ "a" }, Tag{ "b" }, Tag{ "c" } };
    REQUIRE(gul14::join(tags, ", ") == "a, b, c");
}
