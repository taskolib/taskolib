/**
 * \file   SequenceManager.cc
 * \author Marcus Walla
 * \date   Created on July 22, 2022
 * \brief  Manage and control sequences.
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



#include <algorithm>
#include "taskolib/SequenceManager.h"
#include "taskolib/deserialize_sequence.h"
#include "taskolib/serialize_sequence.h"

namespace task {

SequenceManager::SequenceManager(std::filesystem::path path): git_repo_{path}
{
    if (path.empty())
        throw Error("Root sequences path must not be empty.");
    path_ = path;
}

SequenceManager::PathList SequenceManager::get_sequence_names() const
{
    PathList sequences;
    for (auto const& entry : std::filesystem::directory_iterator{path_})
        if (entry.is_directory() && entry != path_ / ".git")
            sequences.push_back(entry.path());

    std::sort(sequences.begin(), sequences.end());

    return sequences;
}

Sequence SequenceManager::load_sequence(std::filesystem::path sequence_path) const
{
    auto sequence = path_/sequence_path;
    check_sequence(sequence_path);
    return task::load_sequence(sequence);
}

void SequenceManager::check_sequence(std::filesystem::path sequence) const
{
    if (not std::filesystem::exists(sequence))
        throw Error(gul14::cat("Sequence file path does not exist: ",
            sequence.string()));
    else if (not std::filesystem::is_directory(sequence))
        throw Error(gul14::cat("File path to sequence is not a directory: ",
            sequence.string()));
}

void SequenceManager::create_sequence(const std::string& name)
{
    Sequence seq{name};
    task::store_sequence(path_/name, seq);
    git_repo_.add();
    git_repo_.commit("create sequence " + name);
}

void SequenceManager::rename_sequence(std::filesystem::path sequence_path, const std::string& new_name)
{
    auto sequence = path_/sequence_path;
    check_sequence(sequence);
    Sequence my_sequence = load_sequence(sequence);
    const std::string old_name = my_sequence.get_label();
    my_sequence.set_label(new_name);
    task::store_sequence(sequence, my_sequence);
    remove_sequence(sequence);
    
    git_repo_.add();
    git_repo_.commit("rename " + old_name + " to " + new_name);

}


void SequenceManager::remove_sequence(std::filesystem::path sequence_name)
{
    // check if relative path is an existing directory
    check_sequence(path_ / sequence_name);
    
    git_repo_.remove_directory(sequence_name);
    git_repo_.add();
    git_repo_.commit("remove " + sequence_name.string());
}

void SequenceManager::remove_all_sequences_and_repository()
{
    //TODO: Delete function?
    
    // delete all sequences and git related files
    std::filesystem::remove_all(path_);

    // reinitialize Libgit object
    //git_repo_ = GitRepository{path_};
}

} // namespace task
