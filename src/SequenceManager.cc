/**
 * \file    SequenceManager.cc
 * \authors Marcus Walla, Lars Fröhlich
 * \date    Created on July 22, 2022
 * \brief   Manage and control sequences.
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
#include <fstream>

#include <gul14/gul.h>

#include "deserialize_sequence.h"
#include "internals.h"
#include "serialize_sequence.h"
#include "taskolib/SequenceManager.h"

#include <libgit4cpp/Error.h>
#include <libgit4cpp/Repository.h>

using namespace std::literals::string_literals;
using gul14::cat;

namespace task {

namespace {

bool contains_id(const std::vector<SequenceManager::SequenceOnDisk>& sequences,
    const UniqueId& uid)
{
    return std::any_of(sequences.begin(), sequences.end(),
        [&uid](const auto& seq) { return seq.unique_id == uid; });
}

/// Create the filename. Push the extra leading zero to the step numberings (ie. leading
/// zeros) to order them alphabetically.
std::string extract_filename_step(const int number, int max_digits, const Step& step)
{
    std::ostringstream ss;
    ss << "step_" << std::setw(max_digits) << std::setfill('0') << number << '_'
       << to_string(step.get_type()) << ".lua";
    return ss.str();
}

void store_sequence_parameters(const std::filesystem::path& lua_file, const Sequence& seq)
{
    std::error_code error;
    std::filesystem::remove(lua_file, error);
    if (error)
        throw Error{ cat("I/O error: ", error.message()) };

    std::ofstream stream(lua_file);

    if (not stream.is_open())
        throw Error{ cat("I/O error: unable to open file (", lua_file.string(), ")") };

    if (not seq.get_maintainers().empty())
        stream << "-- maintainers: " << seq.get_maintainers() << '\n';

    stream << "-- label: " << seq.get_label() << '\n';
    stream << "-- timeout: " << seq.get_timeout() << '\n';
    stream << "-- tags:";
    for (const Tag& tag : seq.get_tags())
        stream << ' ' << tag;
    stream << '\n';
    stream << "-- autorun: " << (seq.get_autorun() ? "true" : "false") << '\n';
    stream << "-- disabled: " << (seq.is_disabled() ? "true" : "false") << '\n';
    stream << seq;
}

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

} // anonymous namespace

SequenceManager::SequenceManager(std::filesystem::path path)
    : path_{ std::move(path) }
    , git_repo_{ path_ }
{
    if (path_.empty())
        throw Error{ "Base path name for sequences must not be empty" };
}

Sequence
SequenceManager::copy_sequence(UniqueId original_uid, const SequenceName& new_name)
{
    const auto sequences = list_sequences();
    const UniqueId new_unique_id = create_unique_id(sequences);
    auto seq = load_sequence(original_uid, sequences);
    seq.set_unique_id(new_unique_id);
    seq.set_name(new_name);

    const auto old_name = find_sequence_on_disk(original_uid, sequences).path;

    auto ok = perform_commit(gul14::cat("Copy sequence ", old_name.string(), " to "),
        [this, &seq]() {
            return this->write_sequence_to_disk(seq);
        });
    if (not ok)
        throw Error{ cat("Cannot commit sequence copy ", to_string(original_uid)) };

    return seq;
}

Sequence
SequenceManager::create_sequence(gul14::string_view label, SequenceName name)
{
    const auto sequences = list_sequences();
    const UniqueId unique_id = create_unique_id(sequences);
    auto seq = Sequence{ label, name, unique_id };

    auto ok = perform_commit("Create sequence ",
        [this, &seq]() {
            return this->write_sequence_to_disk(seq);
        });
    if (not ok)
        throw Error{ cat("Cannot commit sequence creation ", to_string(unique_id)) };

    return seq;
}

UniqueId SequenceManager::create_unique_id(const std::vector<SequenceOnDisk>& sequences)
{
    for (int i = 0; i != 10'000; ++i)
    {
        UniqueId uid;

        if (!contains_id(sequences, uid))
            return uid;
    }

    throw Error{ "Unable to find a unique ID" };
}

SequenceManager::SequenceOnDisk
SequenceManager::find_sequence_on_disk(UniqueId uid,
    const std::vector<SequenceManager::SequenceOnDisk>& sequences)
{
    const auto it = std::find_if(sequences.begin(), sequences.end(),
        [uid](const auto& seq) { return seq.unique_id == uid; });

    if (it == sequences.end())
        throw Error{ cat("Sequence not found: Unknown unique ID ", to_string(uid)) };

    return *it;
}

Sequence SequenceManager::import_sequence(const std::filesystem::path& path)
{
    auto seq = load_sequence(path);

    const UniqueId new_unique_id = create_unique_id(list_sequences());
    seq.set_unique_id(new_unique_id);

    auto ok = perform_commit(gul14::cat("Import sequence from ", path.string(), " to "),
        [this, &seq]() {
            return this->write_sequence_to_disk(seq);
        });
    if (not ok)
        throw Error{ cat("Cannot commit imported sequence ", to_string(new_unique_id)) };

    return seq;
}

std::vector<SequenceManager::SequenceOnDisk> SequenceManager::list_sequences() const
{
    std::vector<SequenceOnDisk> sequences;

    for (const auto& entry : std::filesystem::directory_iterator{ path_ })
    {
        if (not entry.is_directory())
            continue;
        if (entry.path().filename() == ".git")
            continue;

        auto seq_info = parse_folder_name(entry.path());

        // Accept only folders with a valid name and unique ID
        if (seq_info)
        {
            sequences.push_back(SequenceOnDisk{
                std::filesystem::relative(entry.path(), path_),
                seq_info->name, seq_info->unique_id });
        }
    }

    return sequences;
}

Sequence SequenceManager::load_sequence(UniqueId uid) const
{
    return load_sequence(uid, list_sequences());
}

Sequence SequenceManager::load_sequence(UniqueId uid,
    const std::vector<SequenceOnDisk>& sequences) const
{
    const auto seq_on_disk = find_sequence_on_disk(uid, sequences);
    return load_sequence(seq_on_disk);
}

Sequence SequenceManager::load_sequence(std::filesystem::path folder) const
{
    const auto seq_on_disk = parse_folder_name(folder);
    if (not seq_on_disk)
        throw Error{ cat("Invalid sequence folder name: ", folder.string()) };
    return load_sequence(*seq_on_disk);
}

Sequence SequenceManager::load_sequence(const SequenceOnDisk& seq_on_disk) const
{
    const auto path = seq_on_disk.path.is_absolute() ?
         seq_on_disk.path : path_ / seq_on_disk.path;

    if (not std::filesystem::exists(path))
        throw Error{ cat("Sequence file path does not exist: ", path.string()) };
    else if (not std::filesystem::is_directory(path))
        throw Error{ cat("Sequence file path is not a directory: ", path.string()) };

    Sequence seq{ "", seq_on_disk.name, seq_on_disk.unique_id };

    load_sequence_parameters(path, seq);

    std::vector<std::filesystem::path> steps;
    for (auto const& entry : std::filesystem::directory_iterator{ path })
    {
        if (entry.is_regular_file()
            and gul14::starts_with(entry.path().filename().string(), "step_"))
        {
            steps.push_back(entry.path());
        }
    }

    if (not steps.empty())
    {
        // load steps ...
        std::sort(std::begin(steps), std::end(steps),
            [](const auto& lhs, const auto& rhs) -> bool
            { return lhs.filename() < rhs.filename(); });

        for (const auto& entry : steps)
            seq.push_back(load_step(entry));
    }

    return seq;
}

SequenceName SequenceManager::make_sequence_name_from_label(gul14::string_view label)
{
    std::string name;

    if (label.size() > SequenceName::max_length)
        label = label.substr(0, SequenceName::max_length);

    for (const auto c : label)
    {
        if (gul14::contains(SequenceName::valid_characters, c))
            name.push_back(c);
        else
            name.push_back('_');
    }

    return SequenceName{ name };
}

gul14::optional<SequenceManager::SequenceOnDisk> SequenceManager::parse_folder_name(
    const std::filesystem::path& folder)
{
    const std::string str = unescape_filename_characters(folder.filename().string());

    auto opening_bracket = str.rfind('[');
    auto closing_bracket = str.find(']', opening_bracket);
    if (opening_bracket != std::string::npos && closing_bracket == str.size() - 1)
    {
        auto name = SequenceName::from_string(
            gul14::trim(str.substr(0, opening_bracket)));
        auto unique_id = UniqueId::from_string(
            str.substr(opening_bracket + 1, closing_bracket - opening_bracket - 1));

        if (name && unique_id)
            return SequenceOnDisk{ folder, *name, *unique_id };
    }

    // The filename has an invalid format
    return {};
}

void SequenceManager::remove_sequence(UniqueId unique_id)
{
    const auto sequences = list_sequences();
    const auto seq_on_disk = find_sequence_on_disk(unique_id, sequences);

    auto ok = perform_commit("Remove sequence ",
        [this, &seq_on_disk]() {
            std::error_code error;
            std::filesystem::remove_all(this->path_ / seq_on_disk.path, error);
            if (error)
            {
                throw Error{ cat("Cannot remove sequence folder ",
                    seq_on_disk.path.string(), ": ", error.message()) };
            }
            return seq_on_disk.path;
        });
    if (not ok)
        throw Error{ cat("Cannot commit sequence removal ", to_string(unique_id)) };
}

void SequenceManager::rename_sequence(UniqueId unique_id, const SequenceName& new_name)
{
    const auto sequences = list_sequences();
    const auto old_seq_on_disk = find_sequence_on_disk(unique_id, sequences);
    const auto new_disk_name = make_sequence_filename(new_name, unique_id);

    auto ok = perform_commit(gul14::cat("Rename ", old_seq_on_disk.path.string(), " to "),
        [this, &old_seq_on_disk, &new_disk_name]() {
            const auto old_path = this->path_ / old_seq_on_disk.path;
            const auto new_path = this->path_ / new_disk_name;
            std::error_code error;
            std::filesystem::rename(old_path, new_path, error);
            if (error)
            {
                throw Error{ gul14::cat("Cannot rename folder ", old_path.string(),
                    " to ", new_path.string(), ": ", error.message()) };
            }
            return new_disk_name;
        },
        old_seq_on_disk.path);
    if (not ok)
        throw Error{ cat("Cannot commit sequence rename ", to_string(unique_id)) };
}

void SequenceManager::rename_sequence(Sequence& sequence, const SequenceName& new_name)
{
    rename_sequence(sequence.get_unique_id(), new_name);
    sequence.set_name(new_name);
}

bool SequenceManager::store_sequence(const Sequence& seq)
{
    return perform_commit("Modify sequence ",
        [this, &seq]() {
            return this->write_sequence_to_disk(seq);
        });
}

std::string SequenceManager::write_sequence_to_disk(const Sequence& seq)
{
    const int max_digits = int( seq.size() / 10 ) + 1;
    const auto folder = seq.get_folder();
    const auto seq_path = path_ / folder;
    std::error_code error;
    std::filesystem::remove_all(seq_path, error); // remove previous storage
    if (not error)
        std::filesystem::create_directories(seq_path, error);
    if (error)
        throw Error{ cat("I/O error: ", error.message()) };

    store_sequence_parameters(seq_path / sequence_lua_filename, seq);

    unsigned int idx = 0;
    for (const auto& step: seq)
        store_step(seq_path / extract_filename_step(++idx, max_digits, step), step);

    return folder;
}

std::string SequenceManager::stage_files(const std::string& glob)
{
    git_repo_.add(glob);

    auto git_msg = ""s;
    for(const auto& elm: git_repo_.status()) {
        if (elm.handling != "staged")
            continue;
        auto filename = std::filesystem::path{ elm.path_name }.filename();
        git_msg = gul14::cat(git_msg, "\n- ", elm.changes, ": ", filename.c_str());
    }
    return git_msg;
}

namespace {

std::string escape_glob(const std::string& path)
{
    auto escaped = ""s;
    escaped.reserve(path.length());

    for (const char c : path) {
        switch (c) {
        case '*':
            escaped += "\\*";
            break;
        case '?':
            escaped += "\\?";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        case '[':
            escaped += "\\[";
            break;
        case ']':
            escaped += "\\]";
            break;
        default:
            escaped += c;
            break;
        }
    }
#   ifndef ANCIENT_LIBGIT2
        escaped += "/*";
#   endif
    return escaped;
}

} // anonymous namespace

std::string SequenceManager::stage_files_in_directory(const std::string& directory)
{
    return stage_files(escape_glob(directory));
}

} // namespace task

// vi:ts=4:sw=4:sts=4:et
