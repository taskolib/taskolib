/**
 * \file   internals_unit_test.g
 * \author Marcus Walla
 * \date   Created on August 20, 2024
 * \brief  Internal settings for the unit test.
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

#ifndef INTERNALS_UNIT_TEST_H

#include <filesystem>

namespace {

    static const std::filesystem::path temp_dir{ "unit_test_files" };

} // namespace

#endif // INTERNALS_UNIT_TEST_H