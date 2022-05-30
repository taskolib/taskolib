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

namespace {

    /**
     * Convert \a Type to string equivalent.
     * 
     * @param step \a Step
     * @return transformed type
     */
    std::string type_to_string(const Step& step)
    {
        switch(step.get_type())
        {
            case Step::type_action: return "action";
            case Step::type_if: return "if";
            case Step::type_elseif: return "elseif";
            case Step::type_else: return "else";
            case Step::type_while: return "while";
            case Step::type_try: return "try";
            case Step::type_catch: return "catch";
            case Step::type_end: return "end";
            default: return "unknown"; // TODO: maybe include unknown enum Type?
        }
    }

} // namespace anonymous

std::ostream& operator<<(std::ostream& stream, const Step& step)
{
    // TODO: need to fetch taskomat, lua, and sol2 version
    //stream << "-- Taskomat version: " << TASKOMAT_VERSION_STRING << ", Lua version: "
    //    << LUA_VERSION_MAJOR << ", Sol2 version: " << SOL_VERSION_STRING << '\n';

    stream << "-- type: " << type_to_string(step) << '\n';
    stream << "-- label: \"" << step.get_label() << "\"\n";
    stream << "-- use context variable names: [";
    for(auto variable: step.get_used_context_variable_names())
    {
        // TODO: need to fix separator for last entity        
        stream << "\"" << variable.string() << "\", ";
    }
    stream << "]\n";

    const std::time_t last_modification = 
        std::chrono::system_clock::to_time_t(step.get_time_of_last_modification());
    stream << "-- time of last modification: \"" << std::put_time(std::localtime(
        &last_modification), "%F %T") << "\"\n";
    
    const std::time_t last_execution =
        std::chrono::system_clock::to_time_t(step.get_time_of_last_execution());
    stream << "-- time of last execution: \"" << std::put_time(
        std::localtime(&last_execution), "%F %T") << "\"\n";
    
    stream << "-- timeout: ";
    if ( step.get_timeout() == Step::infinite_timeout )
        stream << "infinite\n";
    else
        stream << std::to_string(step.get_timeout().count()) << "ms\n";

    stream << '\n' << step.get_script() << '\n';

    return stream;
}

namespace {

/**
 * Serialize \a Step to the file system.
 * 
 * @param step step to serialize
 * @param path for the \a Step
 */
void serialize_step(const std::filesystem::path& path, const Step& step)
{
    try
    {
        if (std::filesystem::exists(path))
            std::filesystem::remove(path); // remove previous stored step
        std::ofstream stream(path);
        stream << step;
        stream.close();
    }
    catch(const std::exception& e)
    {
        // TODO: provide more information about failure
        throw Error(e.what());
    }
}

} // namespace anonymous

void serialize_sequence(const std::filesystem::path& path, const Sequence& sequence)
{
    unsigned int idx = 0;
    auto seq_path = path / ("sequence_" + gul14::replace(sequence.get_label(), " ", "_"));
    try
    {
        if (not std::filesystem::exists(seq_path))
            std::filesystem::create_directories(seq_path);
    }
    catch(const std::exception& e)
    {
        // TODO: provide more information about failure
        throw Error(e.what());
    }
    for(const auto step: sequence)
        serialize_step(seq_path / ( "step_" + std::to_string(++idx) + '_' +
            type_to_string(step) + ".lua" ), step);
}

} // namespace task