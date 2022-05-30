/**
 * \file   deserialize_sequence.cc
 * \author Marcus Walla
 * \date   Created on May 24, 2022
 * \brief  Deserialize Sequence and Steps from storage hardware.
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
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <gul14/gul.h>
#include "taskomat/deserialize_sequence.h"

namespace task {

namespace {

static const char task[] = "task";

// Using switch statement with string comparision:
// https://learnmoderncpp.com/2020/06/01/strings-as-switch-case-labels/
constexpr auto hash(const gul14::string_view sv)
{
    unsigned long hash{ 5381 };
    for (unsigned char c : sv) {
        hash = ((hash << 5) + hash) ^ c;
    }
    return hash;
}
 
// Using switch statement with string comparision:
// https://learnmoderncpp.com/2020/06/01/strings-as-switch-case-labels/
constexpr auto operator"" _sh(const char *str, size_t len)
{
    return hash(gul14::string_view{ str, len });
}

const std::string extract_keyword(const std::string& extract)
{
    auto found_value = extract.find(":");
    if (extract.rfind("-- ", 0) == 0 && found_value != std::string::npos)
        return extract.substr(3, found_value - 3);
    return "";
}

void extract_type(const std::string& extract, Step& step)
{
    auto pos = extract.find(": ");
    auto keyword = extract.substr(pos + 2, extract.size() - pos);

    switch(hash(keyword))
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
            throw Error(gul14::cat("Step type mismatch: \"", keyword, "\""));
    }
}

void extract_label(const std::string& extract, Step& step)
{
    auto pos = extract.rfind(": \"") + 3;
    if (pos != extract.size() - 1 /* last quote */)
        step.set_label(extract.substr(pos, extract.size() - pos - 1 /* last quote */));
}

void extract_context_variable_names(const std::string& extract, Step& step)
{
    // TODO
}

void extract_time_of_last_modification(const std::string& extract, Step& step)
{
    // TODO
}

void extract_time_of_last_execution(const std::string& extract, Step& step)
{
    // TODO
}

void extract_timeout(const std::string& extract, Step& step)
{
    // TODO
}

} // namespace anonymous

std::istream& operator>>(std::istream& stream, Step& step)
{
    std::string extract;
    std::stringstream script;
    while(!stream.eof())
    {
        std::getline(stream, extract, '\n');

        auto keyword = extract_keyword(extract);

        // Using switch statement with string comparision:
        // https://learnmoderncpp.com/2020/06/01/strings-as-switch-case-labels/
        switch(hash(keyword))
        {
            case "type"_sh:
                extract_type(extract, step); break;
            case "label"_sh:
                extract_label(extract, step); break;
            case "use context variable names"_sh:
                extract_context_variable_names(extract, step); break;
            case "time of last modification"_sh:
                extract_time_of_last_modification(extract, step); break;
            case "time of last execution"_sh:
                extract_time_of_last_execution(extract, step); break;
            case "timeout"_sh:
                extract_timeout(extract, step); break;
            default:
                if (not extract.empty() or not script.str().empty())
                    script << extract << '\n';
        }
    }

    if (not script.str().empty())
        step.set_script(script.str());

    return stream;
}

namespace {

static const char sequence[] = "sequence_";

std::string extract_sequence_label(std::string extract)
{
    std::size_t pos = extract.find(sequence);
    if (pos == std::string::npos)
        throw Error(gul14::cat("sequence folder must start with '", sequence, "'"));
    else if (extract == sequence)
        throw Error("sequence folder needs label description");
    extract.erase(0, extract.find("_") + 1);

    std::string label;

    while ((pos = extract.find("_")) != std::string::npos) // delimiter: "_"
    {
        label += extract.substr(0, pos) + ' ';
        extract.erase(0, pos + 1 /* delimiter length */);
    }

    return label.empty() ? extract : label + extract;
}

void load_step(const std::filesystem::path& step_filename, Step& step)
{
    std::ifstream stream(step_filename);
    stream >> step;
    stream.close();
}

} // namespace anonymous

std::unique_ptr<Sequence> deserialize_sequence(const std::filesystem::path& path)
{
    if (path.empty())
        throw Error("Must specify a valid path. Currently it is empty.");
    else if (path.filename().empty())
        throw Error(gul14::cat("Must specify a valid path: '", path.string(), "'"));

    auto label = extract_sequence_label(path.filename().string());
    Sequence seq{label};

    std::vector<std::filesystem::path> steps;
    for (auto const& entry : std::filesystem::directory_iterator{path}) 
        if (entry.is_regular_file())
            steps.push_back(entry.path());
    std::sort(std::begin(steps), std::end(steps), 
        [](const auto& lhs, const auto& rhs) -> bool
        { return lhs.filename() < rhs.filename(); });

    for(auto entry: steps)
    {
        Step step{};
        load_step(entry, step);
        seq.push_back(step);
    }

    return std::make_unique<Sequence>(seq);
}

} // namespace task