/**
 * \file   serialize_sequence.cc
 * \author Marcus Walla
 * \date   Created on May 06, 2022
 * \brief  Implementation of the store_sequence() free function.
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

#include <fstream>
#include <sstream>

#include <gul14/gul.h>

#include "internals.h"
#include "serialize_sequence.h"

using gul14::cat;

namespace task {

namespace {

void check_stream(std::ostream& stream)
{
    if (stream.bad())
        throw Error(cat("I/O error: serious error on file system (bad flag is set)"));
    else if (stream.fail())
        throw Error(cat("I/O error: failure on storing step"));
}

} // anonymous namespace

std::string make_sequence_filename(SequenceName sequence_name, UniqueId unique_id)
{
    return cat(sequence_name.string(), '[', to_string(unique_id), ']');
}

std::ostream& operator<<(std::ostream& stream, const Step& step)
{
    // TODO: need to fetch taskolib, lua, and sol2 version
    //stream << "-- Taskolib version: " << TASKOLIB_VERSION_STRING << ", Lua version: "
    //    << LUA_VERSION_MAJOR << ", Sol2 version: " << SOL_VERSION_STRING << '\n';

    stream << "-- type: " << to_string(step.get_type()) << '\n';
    stream << "-- label: " << gul14::escape(step.get_label()) << '\n';

    stream << "-- use context variable names: [";
    stream << gul14::join(step.get_used_context_variable_names(), ", ") << "]\n";

    auto modify = TimePoint::clock::to_time_t(step.get_time_of_last_modification());
    stream << "-- time of last modification: "
        << std::put_time(std::localtime(&modify), "%Y-%m-%d %H:%M:%S") << '\n';

    auto execution = TimePoint::clock::to_time_t(step.get_time_of_last_execution());
    stream << "-- time of last execution: "
        << std::put_time(std::localtime(&execution), "%Y-%m-%d %H:%M:%S") << '\n';

    stream << "-- timeout: " << step.get_timeout() << '\n';
    stream << "-- disabled: " << std::boolalpha << step.is_disabled() << '\n';
    stream << step.get_script() << '\n'; // (Marcus) good practice to add a cr at the end

    check_stream(stream);

    return stream;
}

void store_step(const std::filesystem::path& lua_file, const Step& step)
{
    if (std::filesystem::exists(lua_file))
    {
        std::error_code err{};
        std::filesystem::remove(lua_file, err);
        if (err)
        {
            throw Error(cat("I/O error: Unable to remove file '", lua_file.string(),
                "': ", err.message()));
        }
    }

    std::ofstream stream(lua_file);

    if (not stream.is_open())
        throw Error(gul14::cat("I/O error: unable to open file (", lua_file.string(), ")"));

    stream << step; // RAII closes the stream (let the destructor do the job)
}

std::ostream& operator<<(std::ostream& stream, const Sequence& sequence)
{
    stream << sequence.get_step_setup_script();

    check_stream(stream);

    return stream;
}


} // namespace task
