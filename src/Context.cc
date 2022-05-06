/**
 * \file   Context.cc
 * \author Lars Froehlich, Marcus Walla
 * \date   Created on May 6, 2022
 * \brief  Declaration of the Context and VariableValue types.
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

#include "taskomat/Context.h"

namespace task {

std::ostream& operator<<(std::ostream& stream, const Context& context)
{
    stream << "Context {\n";
    if (not context.variables.empty())
    {
        stream << "Variables {\n";
        for(auto [first, second]: context.variables )
           stream << "\"" << first.string() << "\" -> \"" << "Oops!" << "\"\n";
        stream << "}\n";
    }
    // TODO: how do I store std::function?
    stream << "}\n";
    return stream;
}

} // namespace task