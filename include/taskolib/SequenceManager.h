/**
 * \file   SequenceManager.h
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

#ifndef TASKOLIB_SEQUENCEMANAGER_H_
#define TASKOLIB_SEQUENCEMANAGER_H_

#include <filesystem>
#include <string>
#include <vector>

#include <gul14/string_view.h>

#include "taskolib/Sequence.h"
#include "taskolib/UniqueId.h"

#include <libgit4cpp/GitRepository.h>


namespace task {

/**
 * A class for listing, loading, storing, and renaming sequences in a given file system
 * directory.
 *
 * \code
 * // Create a SequenceManager that manages sequences stored in the base folder "sequences"
 * SequenceManager manager{ "sequences" };
 *
 * // List all sequences below the base folder
 * auto sequences = manager.list_sequences();
 *
 * for (const auto& s : sequences) {
 *     // Load the sequence from disk
 *     auto seq = manager.load_sequence(s.path);
 *     std::cout << "Sequence " << seq.get_name() << " has " << seq.get_steps().size()
 *               << " steps\n";
 * }
 * \endcode
 */
class SequenceManager
{
public:
    /// A struct to represent a sequence on disk.
    struct SequenceOnDisk
    {
        std::filesystem::path path; ///< Path to the sequence, relative to SequenceManager base path
        SequenceName name; ///< Machine-friendly name of the sequence
        UniqueId unique_id; ///< Unique ID of the sequence
    };

    /**
     * Create a SequenceManager to manage sequences that are stored in a given directory.
     *
     * \param path  the base folder that contains individual folders for each sequence.
     *
     * \exception Error is thrown if the path name is empty.
     */
    explicit SequenceManager(std::filesystem::path path);

    /**
     * Create a copy of an existing sequence (from disk).
     *
     * This function loads an existing sequence (identified by its unique ID) from disk,
     * assigns a new name (new_name) and a new random unique ID to it, and stores the new
     * sequence in the base folder.
     *
     * \param original_uid  Unique ID of the original sequence
     * \param new_name      Machine-friendly name of the copy
     *
     * \returns the copied sequence as if it had been loaded from disk.
     *
     * \exception Error is thrown if the original sequence cannot be found or if the new
     *            sequence folder cannot be created.
     */
    Sequence copy_sequence(UniqueId original_uid, const SequenceName& new_name);

    /**
     * Create an empty sequence on disk.
     *
     * The new sequence contains no steps and has a randomly assigned unique ID.
     *
     * \param label  An optional human-readable label for the sequence
     * \param name   A optional machine-friendly name for the sequence
     *
     * \returns a new sequence.
     *
     * \exception Error is thrown if the sequence folder cannot be created.
     */
    Sequence create_sequence(gul14::string_view label = "",
        SequenceName name = SequenceName{});

    /**
     * Return the base path of the serialized sequences.
     *
     * \returns the base path of the serialized sequences.
     */
    std::filesystem::path get_path() const { return path_; }

    /**
     * Return an unsorted list of all valid sequences that are found inside the base path
     * and rename sequence folders that do not contain a valid unique ID.
     *
     * This function examines all folders inside the base path. If any of these folder
     * names does not contain a valid unique ID, one is randomly generated and the folder
     * is renamed accordingly.
     *
     * \returns a vector containing one SequenceOnDisk object for each sequence that was
     *          found. The paths in the returned objects are relative to the base path.
     *
     * \exception Error is thrown if one of the folders needs to be renamed but the
     *            renaming fails.
     */
    std::vector<SequenceOnDisk> list_sequences() const;

    /**
     * Load a sequence from the base folder.
     *
     * \param unique_id  unique ID of the sequence to be loaded
     *
     * \returns the loaded sequence.
     *
     * \exception Error is thrown if the sequence cannot be loaded.
     */
    Sequence load_sequence(UniqueId uid) const;

    /**
     * \copydoc load_sequence(UniqueId)
     *
     * This overload allows to pass in a list of sequences to avoid having to examine the
     * base path again:
     *
     * \param sequences  a list of sequences as obtained from list_sequences()
     */
    Sequence
    load_sequence(UniqueId uid, const std::vector<SequenceOnDisk>& sequences) const;

    /**
     * Remove a sequence from the base folder.
     *
     * The sequence to be removed is identified by its unique ID.
     *
     * \param unique_id  the unique ID of the sequence
     *
     * \exception Error is thrown if the sequence cannot be found or if the removal of
     *            the folder fails.
     */
    void remove_sequence(UniqueId unique_id);

    /**
     * Change the machine-friendly name of a sequence on disk.
     *
     * The sequence to be renamed is identified by its unique ID.
     *
     * \param unique_id  the unique ID of the sequence
     * \param new_name   the new machine-friendly name of the sequence
     *
     * \exception Error is thrown if the sequence cannot be found or if the renaming of
     *            the folder fails.
     */
    void rename_sequence(UniqueId unique_id, const SequenceName& new_name);

    /**
     * Change the machine-friendly name of a sequence, both in a Sequence object and on
     * disk.
     *
     * The sequence to be renamed is identified by the unique ID of the given Sequence
     * object. The Sequence object is updated to reflect the new name.
     *
     * \param sequence  the sequence to be renamed
     * \param new_name  the new machine-friendly name of the sequence
     *
     * \exception Error is thrown if the sequence cannot be found or if the renaming of
     *            the folder fails.
     */
    void rename_sequence(Sequence& sequence, const SequenceName& new_name);

    /**
     * Store the given sequence in a subfolder under the base directory of this object.
     *
     * This function generates a subfolder name from the sequence name and unique ID. If
     * such a subfolder exists already, it is removed. Afterwards, a subfolder is newly
     * created and the sequence is stored inside it. Inside the folder, each step is
     * stored in a separate file. The filenames start with `step` followed by a
     * consecutive number followed by the type of the step and the extension `'.lua'`.
     * The step number is zero-filled to allow alphanumerical sorting
     * (e.g. `step_01_action.lua`).
     *
     * \param sequence  the sequence to be stored
     */
    void store_sequence(const Sequence& sequence);

private:
    /// Base path to the sequences.
    std::filesystem::path path_;

    /// Git Repsoitory object
    git::GitRepository gl_;

    /**
     * Create a random unique ID that does not collide with the ID of any sequence in the
     * given sequence list.
     *
     * \exception Error is thrown if no unique ID can be found.
     */
    static UniqueId create_unique_id(const std::vector<SequenceOnDisk>& sequences);

    /**
     * Commit chosen changes of sequences to the local git repository.
     * \param commit_msg: commit message
     * \param files: list of files to be staged. path is relative to sequence directory. No regex allowed.
     * \exception an exception is thrown if not all files could be staged.
    */
   void stage_and_commit_files(std::string commit_msg, std::vector<std::filesystem::path> files);

    /**
     * Find the sequence with the given unique ID in the given list of sequences.
     * \exception Error is thrown if the sequence cannot be found.
     */
    static SequenceOnDisk find_sequence_on_disk(UniqueId uid,
        const std::vector<SequenceOnDisk>& sequences);

    /// Generate a machine-friendly sequence name from a human-readable label.
    static SequenceName make_sequence_name_from_label(gul14::string_view label);

    void _store_sequence(const Sequence& sequence) const;
};

} // namespace task

#endif
