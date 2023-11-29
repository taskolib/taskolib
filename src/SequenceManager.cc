/**
 * \file   SequenceManager.cc
 * \author Marcus Walla, Lars Fröhlich
 * \date   Created on July 22, 2022
 * \brief  Manage and control sequences.
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

#include <algorithm>
#include <fstream>

#include <gul14/substring_checks.h>

#include "deserialize_sequence.h"
#include "internals.h"
#include "serialize_sequence.h"
#include "taskolib/SequenceManager.h"

#include <libgit4cpp/GitRepository.h>
#include <libgit4cpp/Error.h>

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

void remove_path(const std::filesystem::path& folder)
{
    try
    {
        if (std::filesystem::exists(folder))
            std::filesystem::remove(folder); // remove previous stored step
    }
    catch (const std::exception& e)
    {
        throw Error(cat("I/O error: ", e.what()));
    }
}

void store_sequence_parameters(const std::filesystem::path& lua_file, const Sequence& seq)
{
    remove_path(lua_file);

    std::ofstream stream(lua_file);

    if (not stream.is_open())
        throw Error(gul14::cat("I/O error: unable to open file (", lua_file.string(), ")"));

    if (not seq.get_maintainers().empty())
        stream << "-- maintainers: " << seq.get_maintainers() << '\n';

    stream << "-- label: " << seq.get_label() << '\n';

    stream << "-- timeout: ";
    if (!isfinite(seq.get_timeout()))
        stream << "infinite\n";
    else
        stream << static_cast<std::chrono::milliseconds>(seq.get_timeout()).count() << '\n';

    stream << seq; // RAII closes the stream (let the destructor do the job)
}

} // anonymous namespace

SequenceManager::SequenceManager(std::filesystem::path path)
    : path_{ std::move(path) }, git_repo_{path_}
{
    if (path_.empty())
        throw Error("Base path name for sequences must not be empty");
}

Sequence
SequenceManager::copy_sequence(UniqueId original_uid, const SequenceName& new_name)
{
    const auto sequences = list_sequences();
    const UniqueId new_unique_id = create_unique_id(sequences);

    Sequence sequence = load_sequence(original_uid, sequences);

    sequence.set_unique_id(new_unique_id);
    sequence.set_name(new_name);

    store_sequence_impl(sequence);

    // commit to local repository
    const auto new_folder_name = make_sequence_filename(new_name, new_unique_id);
    const auto commit_msg = stage_files_in_directory(new_folder_name, "");
    if (commit_msg != "")
        git_repo_.commit(gul14::cat("copy sequence:", commit_msg));

    return sequence;
}

Sequence
SequenceManager::create_sequence(gul14::string_view label, SequenceName name) const
{
    const auto sequences = list_sequences();
    const UniqueId unique_id = create_unique_id(sequences);

    const auto new_folder_name = make_sequence_filename(name, unique_id);
    const auto new_path = path_ / new_folder_name;

    std::error_code error;
    std::filesystem::create_directory(new_path, error);
    if (error)
    {
        throw Error(gul14::cat("Unable to create sequence folder '", new_path.string(),
            "': ", error.message()));
    }

    return Sequence{ label, name, unique_id };
}

UniqueId SequenceManager::create_unique_id(const std::vector<SequenceOnDisk>& sequences)
{
    for (int i = 0; i != 10'000; ++i)
    {
        UniqueId uid;

        if (!contains_id(sequences, uid))
            return uid;
    }

    throw Error("Unable to find a unique ID");
}

SequenceManager::SequenceOnDisk
SequenceManager::find_sequence_on_disk(UniqueId uid,
    const std::vector<SequenceManager::SequenceOnDisk>& sequences)
{
    const auto it = std::find_if(sequences.begin(), sequences.end(),
        [uid](const auto& seq) { return seq.unique_id == uid; });

    if (it == sequences.end())
        throw Error(cat("Sequence not found: Unknown unique ID ", to_string(uid)));

    return *it;
}

std::vector<SequenceManager::SequenceOnDisk> SequenceManager::list_sequences() const
{
    std::vector<SequenceOnDisk> sequences;
    std::vector<std::filesystem::path> suspicious_folders;

    for (const auto& entry : std::filesystem::directory_iterator{ path_ })
    {
        if (not entry.is_directory())
            continue;
        if (entry.path().filename() == ".git")
            continue;

        SequenceInfo seq_info = get_sequence_info_from_filename(
            entry.path().filename().string());

        auto rel_path = std::filesystem::relative(entry.path(), path_);

        if (seq_info.name.has_value() && seq_info.unique_id.has_value())
        {
            sequences.push_back(SequenceOnDisk{
                std::move(rel_path),
                std::move(*seq_info.name),
                std::move(*seq_info.unique_id) });
        }
        else
        {
            suspicious_folders.push_back(std::move(rel_path));
        }
    }

    // Loop over all folders that did not have name and unique ID in their name.
    for (const auto& folder : suspicious_folders)
    {
        // It could be a non-sequence folder or a sequence folder from an older version.
        // We only believe it is the latter if it contains a sequence.lua file.
        if (not std::filesystem::exists(path_ / folder / "sequence.lua"))
            continue;

        // So it is a sequence folder after all. We automatically generate a unique ID
        // and rename the folder.
        UniqueId unique_id = create_unique_id(sequences);

        const auto [label, dummy1, dummy2] =
            get_sequence_info_from_filename(folder.string());

        SequenceName name = make_sequence_name_from_label(label);

        const auto new_folder_name = make_sequence_filename(name, unique_id);

        std::error_code error;
        std::filesystem::rename(path_ / folder, path_ / new_folder_name, error);
        if (error)
        {
            throw Error(gul14::cat("Sequence folder ", folder.string(),
                " does not contain a unique ID and cannot be renamed to ",
                new_folder_name, ": ", error.message()));
        }

        auto seq = load_sequence(unique_id);
        if (seq.get_label().empty()) // legacy sequences do not store the label in the lua file
        {
            seq.set_label(label);
            store_sequence_impl(seq);
        }

        sequences.push_back(SequenceOnDisk{ new_folder_name, name, unique_id });
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

    const auto folder = path_ / seq_on_disk.path;

    if (not std::filesystem::exists(folder))
        throw Error(cat("Sequence file path does not exist: ", folder.string()));
    else if (not std::filesystem::is_directory(folder))
        throw Error(cat("Sequence file path is not a directory: ", folder.string()));

    Sequence seq{ "", seq_on_disk.name, seq_on_disk.unique_id };

    load_sequence_parameters(folder, seq);

    std::vector<std::filesystem::path> steps;
    for (auto const& entry : std::filesystem::directory_iterator{folder})
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

void SequenceManager::remove_sequence(UniqueId unique_id)
{
    const auto sequences = list_sequences();
    const auto seq_on_disk = find_sequence_on_disk(unique_id, sequences);
    const auto path = path_ / seq_on_disk.path;

    std::error_code error;
    std::filesystem::remove_all(path, error);
    if (error)
    {
        throw Error(cat("Cannot remove sequence folder ", path.string(), ": ",
            error.message()));
    }

    // commit to local repository
    const auto commit_msg = stage_files_in_directory(seq_on_disk.path, "");
    if (commit_msg != "")
        git_repo_.commit(gul14::cat("remove sequence:", commit_msg));

}

void SequenceManager::rename_sequence(UniqueId unique_id, const SequenceName& new_name)
{
    const auto sequences = list_sequences();
    const auto old_seq_on_disk = find_sequence_on_disk(unique_id, sequences);
    const auto old_path = path_ / old_seq_on_disk.path;
    const auto new_path = path_ / make_sequence_filename(new_name, unique_id);

    std::error_code error;
    std::filesystem::rename(old_path, new_path, error);
    if (error)
    {
        throw Error(gul14::cat("Cannot rename folder ", old_path.string(),
            " to ", new_path.string(), ": ", error.message()));
    }

    // commit to local repository
    auto commit_msg = stage_files_in_directory(old_seq_on_disk.path, "");
    commit_msg += stage_files_in_directory(make_sequence_filename(new_name, unique_id), "");
    if (commit_msg != "")
        git_repo_.commit(gul14::cat("Rename ", old_seq_on_disk.name.string(), " to ", new_name.string(), ":", commit_msg));
}

void SequenceManager::rename_sequence(Sequence& sequence, const SequenceName& new_name)
{
    rename_sequence(sequence.get_unique_id(), new_name);
    sequence.set_name(new_name);
}

void SequenceManager::store_sequence(const Sequence& seq)
{
    store_sequence_impl(seq);

    // detect what has changed in the sequence
    const auto dir_name = make_sequence_filename(seq.get_name(), seq.get_unique_id());
    const auto commit_msg = stage_files_in_directory(dir_name, "");

    // commit to local repository
    if (commit_msg != "")
        git_repo_.commit(gul14::cat("change sequence:", commit_msg));
}

void SequenceManager::store_sequence_impl(const Sequence& seq) const
{
    const int max_digits = int( seq.size() / 10 ) + 1;
    const auto seq_path = path_ / make_sequence_filename(seq);
    try
    {
        if (std::filesystem::exists(seq_path))
            std::filesystem::remove_all(seq_path); // remove previous storage
        std::filesystem::create_directories(seq_path);
    }
    catch (const std::exception& e)
    {
        throw Error(cat("I/O error: ", e.what()));
    }

    store_sequence_parameters(seq_path / sequence_lua_filename, seq);

    unsigned int idx = 0;
    for (const auto& step: seq)
        store_step(seq_path / extract_filename_step(++idx, max_digits, step), step);
}

std::string SequenceManager::stage_files_in_directory(std::filesystem::path dir_name, const std::string& filetype)
{
    // detect what has changed in the sequence
    std::string git_msg{""};
    auto stats = git_repo_.status();
    for(const auto& elm: stats)
    {
        
        // filter for changes in sequence
        if (gul14::starts_with(elm.path_name, dir_name.string()))
        {
            if (elm.changes == "new file" and (filetype == elm.changes or filetype == ""))
            {
                git_msg += gul14::cat("\n- create '", elm.path_name, "'");
            }
            else if (elm.changes == "modified" and (filetype == elm.changes or filetype == ""))
            {
                git_msg += gul14::cat("\n- modify '", elm.path_name, "'");
            }
            else if (elm.changes == "deleted" and (filetype == elm.changes or filetype == ""))
            {
                git_msg += gul14::cat("\n- delete '", elm.path_name, "'");
                git_repo_.remove_files({elm.path_name});
                continue;
            }
            else if (elm.changes == "renamed" and (filetype == elm.changes or filetype == ""))
            {
                git_msg += gul14::cat("\n- rename '", elm.path_name, "'");
            }
            else if (elm.changes == "typechange" and (filetype == elm.changes or filetype == ""))
            {
                git_msg += gul14::cat("\n- '", elm.path_name, "' has its type change");
            }
            else if (elm.changes == "untracked" and (filetype == elm.changes or filetype == ""))
            {
                const auto elmpath = (std::filesystem::path) elm.path_name;
                git_msg += gul14::cat("\n- Add ", elmpath.filename().string(), " from new sequence '", dir_name.string(), "'");
            }
            else continue;

            auto err_index = git_repo_.add_files({elm.path_name});
            if (err_index.size() > 0)
                throw git::Error(gul14::cat("ERROR:", elm.path_name, " was not staged."));
        }
    }
    return git_msg;
}

} // namespace task
