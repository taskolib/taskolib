/**
 * \file   test_main.cc
 * \author Lars Fröhlich
 * \date   Created on November 26, 2019
 * \brief  Implementation of main() for the Taskolib unit test suite.
 *
 * \copyright Copyright 2021-2025 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <catch2/catch_session.hpp>

#include "internals_unit_test.h"

int main(int argc, char* argv[])
{
    if (std::filesystem::exists(temp_dir))
        std::filesystem::remove_all(temp_dir);

    return Catch::Session().run(argc, argv);
}
