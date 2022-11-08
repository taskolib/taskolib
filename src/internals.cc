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

#include <gul14/cat.h>
#include <gul14/join_split.h>
#include <gul14/SmallVector.h>

#include "internals.h"

namespace task {

const gul14::string_view abort_marker{ u8"\U0001F6D1ABORT\U0001F6D1" };

std::pair<std::string, ErrorCause> remove_abort_markers(gul14::string_view error_message)
{
    const auto tokens = gul14::split<gul14::SmallVector<gul14::string_view, 3>>(
        error_message, abort_marker);

    std::string msg;

    switch (tokens.size())
    {
    case 0: // impossible
    case 1: // no marker
        msg = std::string(error_message);
        return std::make_pair(msg, ErrorCause::uncaught_error);
    case 2: // one marker
        msg = gul14::cat(tokens[0], tokens[1]);
        break;
    case 3: // The real error message is between the first 2 abort markers.
    default:
        msg = std::string(tokens[1]);
    }

    if (msg.empty()) {
        msg = "Script called terminate_sequence()";
        return std::make_pair(msg, ErrorCause::terminated_by_script);
    }

    return std::make_pair(msg, ErrorCause::aborted);
}

} // namespace task
