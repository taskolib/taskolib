/**
 * \file   serialize_sequence.cc
 * \author Marcus Walla
 * \date   Created on May 06, 2022
 * \brief  Implementation of the serialize_sequence() free function.
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

#include <fstream>
#include "taskomat/serialize_sequence.h"

namespace task {

void serialize_sequence(const std::string& path, const Sequence& sequence,
    const Context& context)
{
    try
    {
        // TODO: needs more improvement
        std::ofstream stream(path);
        stream << context << '\n' << sequence << '\n';
        stream.close();
    }
    catch(const std::exception& e)
    {
        // TODO: provide more information about failure
        throw Error(e.what());
    }
}

} // namespace task