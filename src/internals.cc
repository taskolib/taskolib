/**
 * \file   internals.cc
 * \author Lars Froehlich, Marcus Walla
 * \date   Created on August 30, 2022
 * \brief  Definition of internal constants and functions.
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

#include <gul14/join_split.h>
#include <gul14/SmallVector.h>

#include "internals.h"

namespace task {

const gul14::string_view abort_marker{ u8"\U0001F6D1ABORT\U0001F6D1" };

ErrorCause remove_abort_markers_from_error_message(std::string& msg)
{
    const auto tokens = gul14::split<gul14::SmallVector<gul14::string_view, 3>>(
        msg, abort_marker);

    if (tokens.size() >= 3) // The real error message is between the first 2 abort markers
        msg.assign(tokens[1].begin(), tokens[1].end());
    else
        msg = gul14::join(tokens, "");

    if (tokens.size() > 1) // There was at least one abort marker
    {
        if (msg.empty())
        {
            msg = "Script called terminate_sequence()";
            return ErrorCause::terminated_by_script;
        }

        return ErrorCause::aborted;
    }

    return ErrorCause::uncaught_error;
}

} // namespace task
