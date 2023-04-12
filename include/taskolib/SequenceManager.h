/**
 * \file   SequenceManager.h
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

#ifndef TASKOLIB_SEQUENCEMANAGER_H_
#define TASKOLIB_SEQUENCEMANAGER_H_

#include <vector>
#include <filesystem>
#include <string>
#include <gul14/string_view.h>
#include "taskolib/Sequence.h"
#include "GitRepository.h"

namespace task {

/**
 * A class to have a birds eye view on the underlying serialized sequences in the file
 * system. It allows to manage and control sequences.
 *
 * Since we have a predefined flat struture for serialize sequences we come to the
 * assumption for the following specification:
 *
 * \code
 * ./sequence_1 <- folder name that represents the first sequence
 *    step_while_01.lua <- first step of sequence 1
 *    ...
 *    step_action_<n>.lua <- n-th step of sequence 1
 * ...
 * ./sequence_<m> <- folder name that represents the m-th sequence
 *    step_if_01.lua <- first step of sequence 1
 *    ...
 *    step_action_<n>.lua <- n-th step of sequence 1
 * \endcode
 *
 * Above we have as root path the folder '.'.
 */
class SequenceManager
{
public:
    using PathList = std::vector<std::filesystem::path>;

    /**
     * Creates a new instance to manage and control of sequences that are serialized in
     * the underlying file system.
     *
     * \param path set root path to the sequence folders.
     *
     * \exception throws Error exception if path is empty.
     */
    explicit SequenceManager(std::filesystem::path path);

    /**
     * Returns the root path of the serialized sequences.
     *
     * \return root path of the serialized sequences.
     */
    std::filesystem::path get_path() const { return path_; }

    /**
     * Get sequence names without the previous path.
     *
     * \return sequences as a list of strings.
     */
    PathList get_sequence_names() const;

    /**
     * Loads sequence on the sequence file path.
     *
     * \param sequence_path path to sequence.
     *
     * \return deserialized sequence.
     *
     * \exception throws Error if the sequence path is invalid.
     */
    Sequence load_sequence(std::filesystem::path sequence_path) const;


    /**
     * Delete sequence specified in path to sequence
     *
     * \param sequence_path path to sequence.
     * \note sequence_path must be relative to path_
     *
     * \exception throws Error if removal was unsucessful
     */
    void remove_sequence(std::filesystem::path sequence_path);


    /**
     * Rename sequence on the sequence file path.
     *
     * \param sequence_path path to sequence.
     * \note sequence_path must be relative to path_
     * \param new_name new name of sequence
     *
     * \exception throws Error if renaming was unsuccessful
     */
    void rename_sequence(std::filesystem::path sequence_path, const std::string& new_name);


    /**
     * Create an empty sequence and store it at given path
     * 
     * \param name: sequence name
    */
   void create_sequence(const std::string& name);

    /**
     * Reset the repository by deleting all files from path_ and reinitialiszing the GitRepository Wrapper git_repo_
    */
    void remove_all_sequences_and_repository();

private:
    /// Root path to the sequences
    std::filesystem::path path_;
    GitRepository git_repo_;
    void check_sequence(std::filesystem::path sequence_path) const;

};

} // namespace task

#endif
