/**
 * \file   serialize_sequence.cc
 * \author Marcus Walla
 * \date   Created on May 06, 2022
 * \brief  Implementation of the serialize_sequence() free function.
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

#include <fstream>
#include <sstream>
#include <gul14/gul.h>
#include "taskolib/serialize_sequence.h"

namespace task {

namespace {

/// Remove Step from the file system.
void remove_step_path(const std::filesystem::path& path)
{
    try
    {
        if (std::filesystem::exists(path))
            std::filesystem::remove(path); // remove previous stored step
    }
    catch(const std::exception& e)
    {
        auto err = errno;
        throw Error(gul14::cat("I/O error: ", e.what(), ", error=", std::strerror(err)));
    }
}

/// Extracted from High Level Controls Utility Library (DESY), file string_util.h/cc
std::string escape_filename_characters(gul14::string_view str)
{
    static const std::string bad_characters("/\\:?*\"\'<>|$&");

    std::ostringstream out;
    out.fill('0');

    for (const char c : str)
    {
        const unsigned char u = static_cast<unsigned char>(c);

        if (u <= 32)
            out << ' ';
        else if (u > 127 or bad_characters.find(c) != bad_characters.npos)
            out << '$' << std::setw(2) << std::hex << static_cast<unsigned int>(u);
        else
            out << c;
    }

    return out.str();
}

/// Create the filename. Push the extra leading zero to the step numberings (ie. leading
/// zeros) to order them alphabetically.
std::string extract_filename(const int number, int max_digits, const Step& step)
{
    std::ostringstream ss;
    ss << "step_" << std::setw(max_digits) << std::setfill('0') << number << '_'
       << to_string(step.get_type()) << ".lua";
    return ss.str();
}

void check_stream(std::ostream& stream)
{
    if (stream.bad())
        throw Error(gul14::cat("I/O error: serious error on file system (bad flag is set)"));
    else if (stream.fail())
        throw Error(gul14::cat("I/O error: failure on storing step"));
}

} // namespace anonymous

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

    stream << "-- timeout: ";
    if ( step.get_timeout() == Step::infinite_timeout )
        stream << "infinite\n";
    else
        stream << step.get_timeout().count() << '\n';

    stream << "-- disabled: " << std::boolalpha << step.is_disabled() << '\n';

    stream << step.get_script() << '\n'; // (Marcus) good practice to add a cr at the end

    check_stream(stream);

    return stream;
}

void serialize_step(const std::filesystem::path& path, const Step& step)
{
    remove_step_path(path);

    std::ofstream stream(path);

    if (not stream.is_open())
        throw Error(gul14::cat("I/O error: unable to open file (", path.string(), ")"));

    stream << step; // RAII closes the stream (let the destructor do the job)
}

void serialize_sequence(const std::filesystem::path& path, const Sequence& sequence)
{
    unsigned int idx = 0;
    const int max_digits = int( sequence.size() / 10 ) + 1;
    auto seq_path = path / escape_filename_characters(sequence.get_label());
    try
    {
        if (std::filesystem::exists(seq_path))
            std::filesystem::remove_all(seq_path); // remove previous storage
        std::filesystem::create_directories(seq_path);
    }
    catch(const std::exception& e)
    {
        auto err = errno;
        throw Error(gul14::cat("I/O error: ", e.what(), ", error=", std::strerror(err)));
    }
    for(const auto& step: sequence)
        serialize_step(seq_path / extract_filename(++idx, max_digits, step), step);
}

} // namespace task
