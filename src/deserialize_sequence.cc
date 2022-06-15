/**
 * \file   deserialize_sequence.cc
 * \author Marcus Walla
 * \date   Created on May 24, 2022
 * \brief  Deserialize Sequence and Steps from storage hardware.
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

#include <chrono>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <ctime>
#include <regex>
#include <gul14/gul.h>
#include "taskomat/hash_string.h"
#include "taskomat/deserialize_sequence.h"

namespace task {

namespace {

/// Extracted from High Level Controls Utility Library (DESY), file string_util.h/cc
int hex2dec(const char c)
{
    static constexpr gul14::string_view table { "0123456789abcdef" };
    auto pos = table.find(c);
    if (pos == table.npos)
        return -1;
    else
        return static_cast<int>(pos);
}

/// Extracted from High Level Controls Utility Library (DESY), file string_util.h/cc
// str must be at least 2 chars long; returns negative if conversion failed.
int hex2dec_2chars(gul14::string_view str)
{
    const int a = hex2dec(str[0]);
    const int b = hex2dec(str[1]);
    if (a < 0 or b < 0)
        return -1;
    return (a << 4) | b;
}

/// Extracted from High Level Controls Utility Library (DESY), file string_util.h/cc
std::string unescape_filename_characters(gul14::string_view str)
{
    std::string out;
    out.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i)
    {
        const char c = str[i];
        if (c != '$' or (i + 2) >= str.size())
        {
            out.push_back(c);
            continue;
        }

        // Decode $ sequence
        int val = hex2dec_2chars(str.substr(i+1, 2));
        if (val < 32)
        {
            out.push_back('$');
        }
        else
        {
            out.push_back(val);
            i += 2;
        }
    }

    return out;
}

const std::string extract_keyword(const std::string& extract)
{
    auto found_value = extract.find(":");
    if (gul14::starts_with(extract, "-- ") && found_value != std::string::npos)
        return gul14::trim(extract.substr(3, found_value - 3)); // 3: length of "-- "
    return "";
}

void extract_type(const std::string& extract, Step& step)
{
    auto pos = extract.find(": ");
    if (pos == std::string::npos)
        throw Error("type: cannot find leading ': '");

    auto keyword = gul14::trim(extract.substr(pos + 2));

    switch(hash_djb2a(keyword))
    {
        case "action"_sh:
            step.set_type(Step::type_action); break;
        case "if"_sh:
            step.set_type(Step::type_if); break;
        case "elseif"_sh:
            step.set_type(Step::type_elseif); break;
        case "else"_sh:
            step.set_type(Step::type_else); break;
        case "while"_sh:
            step.set_type(Step::type_while); break;
        case "try"_sh:
            step.set_type(Step::type_try); break;
        case "catch"_sh:
            step.set_type(Step::type_catch); break;
        case "end"_sh:
            step.set_type(Step::type_end); break;
        default:
            throw Error(gul14::cat("type: unable to parse ('", keyword, "')"));
    }
}

void extract_label(const std::string& extract, Step& step)
{
    auto start = extract.find(": ");
    if (start == std::string::npos)
        throw Error("label: cannot find leading ': '");

    start += 2;

    auto end = extract.size();

    if (start + 1 != end)
        step.set_label(gul14::unescape(gul14::trim(extract.substr(start, end - start))));
}

void extract_context_variable_names(const std::string& extract, Step& step)
{
    auto start = extract.find(": [");
    if (start == std::string::npos)
        throw Error("context variable names: cannot find leading ': ['");

    start += 3;

    auto end = extract.find("]", start);
    if (end == std::string::npos)
        throw Error("context variable names: cannot find trailing ']'");
    else if (start < end)
    {
        VariableNames variableNames{};
        for(auto variable: gul14::split(extract.substr(start, end - start), std::regex{"[ \t]*,[ \t]*"}))
            variableNames.emplace(gul14::trim(variable));
        step.set_used_context_variable_names(variableNames);
    }
}

TimePoint extract_time(const std::string& issue, const std::string& extract)
{
    std::tm t;

    auto start = extract.find(": ");
    if (start == std::string::npos)
        throw Error(gul14::cat(issue, ": cannot find leading ': '"));

    if (strptime(extract.substr(start + 2).c_str(), "%Y-%m-%d %H:%M:%S",&t) == nullptr)
        throw Error(gul14::cat(issue, ": unable to parse time ('", extract, "')"));
    t.tm_isdst = -1; // Daylight Saving Time (DST) is unknown -> use local time zone
    auto convert = std::mktime(&t);

    return TimePoint::clock::from_time_t(convert);
}

void extract_time_of_last_execution(const std::string& extract, Step& step)
{
    step.set_time_of_last_execution(extract_time("time of last execution", extract));
}

void extract_timeout(const std::string& extract, Step& step)
{
    const auto start = extract.find(": ");
    if (start == std::string::npos)
        throw Error("timeout: cannot find leading ': '");

    auto start_timeout = extract.find_first_not_of(" \t", start + 2);
    if (start_timeout == std::string::npos)
        throw Error(gul14::cat("timeout: unable to parse ('", extract.substr(start), "')"));

    auto timeout = gul14::trim(extract.substr(start_timeout));
    if ("infinite" == timeout)
        step.set_timeout(Step::infinite_timeout);
    else
    {
        try
        {
            step.set_timeout(std::chrono::milliseconds(std::stoull(timeout)));
        }
        catch(...) // catch any exception from std::stoull (Step::set_timeout is nothrow).
        {
            throw Error(gul14::cat("timeout: unable to parse number ('", timeout, "')"));
        }
    }
}

} // namespace anonymous

std::istream& operator>>(std::istream& stream, Step& step)
{
    TimePoint last_modification{}; // since any manipulation on step sets a new time point
    std::string extract;
    std::stringstream script;
    bool load_script = false; // flag to store non Step properties to the script
    bool has_type = false; // sanity check: must have type
    bool has_label = false; // sanity check: must have label
    Step step_internal; // temporary Step. Will be move to step after loading

    while(std::getline(stream, extract, '\n'))
    {
        auto internal = gul14::trim(extract);
        auto keyword = extract_keyword(internal);

        if (load_script)
        {
            script << extract << '\n';
            continue;
        }
        else if (internal.empty()) // load_script is false -> nothing useful
            continue;

        // Using switch cases with string hashes (see operator"" _sh in case_string.h)
        // DJB2A hash algorithm: http://www.cse.yorku.ca/%7Eoz/hash.html
        // Interesting hashing algorithm comparison:
        // https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed/145633#145633
        switch(hash_djb2a(keyword))
        {
            case "type"_sh:
                extract_type(internal, step_internal);
                has_type = true;
                break;
            case "label"_sh:
                extract_label(internal, step_internal);
                has_label = true;
                break;
            case "use context variable names"_sh:
                extract_context_variable_names(internal, step_internal);
                break;
            case "time of last modification"_sh:
                last_modification = extract_time("time of last modification", internal);
                break;
            case "time of last execution"_sh:
                extract_time_of_last_execution(internal, step_internal);
                break;
            case "timeout"_sh:
                extract_timeout(internal, step_internal);
                break;
            default:
                script << extract << '\n';
                load_script = true;
        }
    }

    if (stream.bad())
        throw Error(gul14::cat("I/O error: serious error on file system (bad flag is set)"));
    // need to get !eof (eof indicates that also the fail bit is set)
    else if (not stream.eof() and stream.fail())
        throw Error(gul14::cat("I/O error: failure on loading step"));

    if (not has_type) // sanity check: missing type
        throw Error("Step must have type declaration");
    else if (not has_label) // sanity check: missing label
        throw Error("Step must have label declaration");

    if (not script.str().empty())
    {
        auto temp = script.str();
        step_internal.set_script(temp.substr(0, temp.size() - 1)); // remove last cr
    }

    // finally set time points ...
    if (last_modification.time_since_epoch().count() != 0LL)
        step_internal.set_time_of_last_modification(last_modification);
    else // sanity check: if no time is provided set it to current time
        step_internal.set_time_of_last_modification(TimePoint::clock::now());

    step = std::move(step_internal);

    return stream;
}

Step deserialize_step(const std::filesystem::path& path)
{
    Step step{};
    std::ifstream stream(path);

    if (not stream.is_open())
        throw Error(gul14::cat("I/O error: unable to open file '", path.string(), "'"));

    stream >> step; // RAII closes the stream (let the destructor do the job)

    return step;
}

Sequence deserialize_sequence(const std::filesystem::path& path)
{
    if (path.empty())
        throw Error("Must specify a valid path. Currently it is empty.");
    else if (not std::filesystem::exists(path))
        throw Error(gul14::cat("Path does not exist: '", path.string(), "'"));

    auto label = unescape_filename_characters(path.filename().string());
    Sequence seq{label};

    std::vector<std::filesystem::path> steps;
    for (auto const& entry : std::filesystem::directory_iterator{path})
        if (entry.is_regular_file())
            steps.push_back(entry.path());

    if (steps.empty())
        throw Error(gul14::cat("No steps found: ", path.string()));

    std::sort(std::begin(steps), std::end(steps),
        [](const auto& lhs, const auto& rhs) -> bool
        { return lhs.filename() < rhs.filename(); });

    for(auto entry: steps)
        seq.push_back(deserialize_step(entry));

    return seq;
}

} // namespace task