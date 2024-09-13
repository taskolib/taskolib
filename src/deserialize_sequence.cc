/**
 * \file    deserialize_sequence.cc
 * \authors Marcus Walla, Lars Fröhlich
 * \date    Created on May 24, 2022
 * \brief   Deserialize Sequence and Steps from storage hardware.
 *
 * \copyright Copyright 2022-2024 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <gul14/gul.h>

#include "deserialize_sequence.h"
#include "internals.h"
#include "taskolib/hash_string.h"

using gul14::cat;
using gul14::escape;

namespace task {

namespace {

/**
 * Separate a comment line from a Lua script into a keyword and a remainder.
 *
 * "-- label: Hello" -> ["label", " Hello"]
 * "   -- label:Pippo " -> ["label", "Pippo "]
 * " -- Comment" -> ["", " -- Comment"]
 * "  something" -> ["", "  something"]
 */
std::pair<gul14::string_view, gul14::string_view>
extract_keyword(const gul14::string_view line)
{
    auto remainder = gul14::trim_left_sv(line);

    if (not gul14::starts_with(remainder, "-- "))
        return {{}, line};

    auto found_value = remainder.find(":");
    if (found_value == gul14::string_view::npos)
        return {{}, line};

    auto keyword = gul14::trim_sv(remainder.substr(3, found_value - 3)); // 3: length of "-- "
    remainder.remove_prefix(found_value + 1); // incl the ":"

    return { keyword, remainder };
}

void extract_type(gul14::string_view extract, Step& step)
{
    auto keyword = gul14::trim_sv(extract);

    switch(hash_djb2a({keyword.data(), keyword.size()}))
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
            throw Error(gul14::cat("type: unable to parse (\"", escape(keyword), "\")"));
    }
}

void extract_label(gul14::string_view extract, Step& step)
{
    step.set_label(gul14::unescape(gul14::trim_sv(extract)));
}

void extract_context_variable_names(gul14::string_view extract, Step& step)
{
    extract = gul14::trim_sv(extract);
    if (not gul14::starts_with(extract, "["))
        throw Error("context variable names: cannot find leading '['");
    extract.remove_prefix(1);

    auto end = extract.find("]");
    if (end == gul14::string_view::npos)
        throw Error("context variable names: cannot find trailing ']'");
    VariableNames variableNames{};
    for (auto variable: gul14::split(extract.substr(0, end), std::regex{"[ \t]*,[ \t]*"})) {
        variable = gul14::trim(variable);
        if (not variable.empty())
            variableNames.emplace(variable);
    }
    if (not variableNames.empty())
        step.set_used_context_variable_names(variableNames);
}

TimePoint extract_time(const std::string& issue, gul14::string_view extract)
{
    std::tm t;

    if (strptime(std::string{ extract }.c_str(), "%Y-%m-%d %H:%M:%S",&t) == nullptr)
        throw Error(gul14::cat(issue, ": unable to parse time (\"", escape(extract), "\")"));
    t.tm_isdst = -1; // Daylight Saving Time (DST) is unknown -> use local time zone
    auto convert = std::mktime(&t);

    return TimePoint::clock::from_time_t(convert);
}

void extract_time_of_last_execution(gul14::string_view extract, Step& step)
{
    step.set_time_of_last_execution(extract_time("time of last execution", extract));
}

} // anonymous namespace

std::istream& operator>>(std::istream& stream, Step& step)
{
    TimePoint last_modification{}; // since any manipulation on step sets a new time point
    std::string line;
    std::stringstream script;
    bool load_script = false; // flag to store non Step properties to the script
    bool has_type = false; // sanity check: must have type
    bool has_label = false; // sanity check: must have label
    std::set<unsigned long> encountered_keywords; // validate multiple keyword definition
    Step step_internal; // temporary Step. Will be move to step after loading

    while(std::getline(stream, line, '\n'))
    {
        if (load_script)
        {
            script << line << '\n';
            continue;
        }
        auto [keyword, remaining_line] = extract_keyword(line);
        if (keyword.empty() && gul14::trim_sv(remaining_line).empty()) // load_script is false -> nothing useful
            continue;

        const auto keyword_hash = hash_djb2a(keyword);

        // validate multiple keyword declaration
        if (encountered_keywords.count(keyword_hash))
        {
            throw Error(gul14::cat("Syntax error: Encountered keyword '", keyword,
                                   "' multiple times"));
        }
        encountered_keywords.insert(keyword_hash);

        // Using switch cases with string hashes (see operator"" _sh in case_string.h)
        // DJB2A hash algorithm: http://www.cse.yorku.ca/%7Eoz/hash.html
        // Interesting hashing algorithm comparison:
        // https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed/145633#145633
        switch(keyword_hash)
        {
            case "type"_sh:
                extract_type(remaining_line, step_internal);
                has_type = true;
                break;

            case "label"_sh:
                extract_label(remaining_line, step_internal);
                has_label = true;
                break;

            case "use context variable names"_sh:
                extract_context_variable_names(remaining_line, step_internal);
                break;

            case "time of last modification"_sh:
                last_modification = extract_time("time of last modification", remaining_line);
                break;

            case "time of last execution"_sh:
                extract_time_of_last_execution(remaining_line, step_internal);
                break;

            case "timeout"_sh:
                step_internal.set_timeout(parse_timeout(remaining_line));
                break;

            case "disabled"_sh:
                step_internal.set_disabled(parse_bool(remaining_line));
                break;

            default:
                script << line << '\n';
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

Step load_step(const std::filesystem::path& lua_file)
{
    Step step{};
    std::ifstream stream(lua_file);

    if (not stream.is_open())
        throw Error(gul14::cat("I/O error: unable to open file \"", escape(lua_file.string()), '"'));

    stream >> step; // RAII closes the stream (let the destructor do the job)

    return step;
}

void load_sequence_parameters(const std::filesystem::path& folder, Sequence& sequence)
{
    if (not std::filesystem::exists(folder))
        throw Error(gul14::cat("Folder does not exist: \"", escape(folder.string()), '"'));

    auto stream = std::ifstream(folder / sequence_lua_filename);

    std::string step_setup_script;

    std::string line;
    if (stream.good())
    {
        while(std::getline(stream, line, '\n'))
        {
            auto keyword = gul14::trim_left_sv(line);

            if (gul14::starts_with(keyword, "-- maintainers:"))
                sequence.set_maintainers(keyword.substr(15));
            else if (gul14::starts_with(keyword, "-- label:"))
                sequence.set_label(gul14::trim_sv(keyword.substr(9)));
            else if (gul14::starts_with(keyword, "-- timeout:"))
                sequence.set_timeout(parse_timeout(keyword.substr(11)));
            else if (gul14::starts_with(keyword, "-- tags:"))
                sequence.set_tags(parse_tags(keyword.substr(8)));
            else if (gul14::starts_with(keyword, "-- autorun:"))
                sequence.set_autorun(parse_bool(keyword.substr(11)));
            else if (gul14::starts_with(keyword, "-- disabled:"))
                sequence.set_disabled(parse_bool(keyword.substr(12)));
            else
                step_setup_script += (line + '\n');
        }

        sequence.set_step_setup_script(step_setup_script);
    }
}

std::vector<Tag> parse_tags(gul14::string_view str)
{
    const auto tokens = gul14::tokenize_sv(str);

    std::vector<Tag> tags(tokens.size());
    std::transform(tokens.begin(), tokens.end(), tags.begin(),
                   [](gul14::string_view token) { return Tag{ token }; });
    return tags;
}

bool parse_bool(gul14::string_view str)
{
    auto bool_expression{gul14::trim_sv(str)};
    if (bool_expression == "true" )
        return true;
    else if (bool_expression == "false")
        return false;
    throw Error(gul14::cat("Cannot parse bool expression from \"", escape(str), '"'));
}

Timeout parse_timeout(gul14::string_view str)
{
    str = gul14::trim_sv(str);

    if (gul14::equals_nocase(str, "infinite"))
        return Timeout::infinity();

    const auto maybe_msec = gul14::to_number<unsigned long long>(str);
    if (!maybe_msec)
        throw Error(gul14::cat("Cannot parse timeout from \"", escape(str), '"'));

    return Timeout{ std::chrono::milliseconds{ *maybe_msec } };
}

} // namespace task
