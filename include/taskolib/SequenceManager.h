/**
 * \file    SequenceManager.h
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

#ifndef TASKOLIB_SEQUENCEMANAGER_H_
#define TASKOLIB_SEQUENCEMANAGER_H_

#include <filesystem>
#include <string>
#include <vector>

#include <gul14/optional.h>
#include <gul14/string_view.h>
#include <libgit4cpp/Repository.h>

#include "taskolib/Sequence.h"
#include "taskolib/UniqueId.h"

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
        /// Path to the sequence (usually relative to SequenceManager base path, but can
        /// also be absolute)
        std::filesystem::path path;

        SequenceName name; ///< Machine-friendly name of the sequence
        UniqueId unique_id; ///< Unique ID of the sequence

        friend bool operator==(const SequenceOnDisk& a, const SequenceOnDisk& b) noexcept
        {
            return a.unique_id == b.unique_id && a.name == b.name && a.path == b.path;
        }

        friend bool operator!=(const SequenceOnDisk& a, const SequenceOnDisk& b) noexcept
        {
            return !(a == b);
        }
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
    Sequence create_sequence(gul14::string_view label = "", SequenceName name = SequenceName{});

    /**
     * Return the base path of the serialized sequences.
     *
     * \returns the base path of the serialized sequences.
     */
    std::filesystem::path get_path() const { return path_; }

    /**
     * Import a sequence from a folder, assigning a new unique ID to it.
     *
     * This function loads an existing sequence from the specified path. If the import is
     * successful, the new sequence is stored in the base folder. It keeps the name and
     * label of the original, but a random unique ID is assigned.
     *
     * \param path  Path to the original sequence folder
     *
     * \returns the copied sequence as if it had been loaded from disk.
     *
     * \exception Error is thrown if the original sequence cannot be loaded or if the new
     *            sequence folder cannot be created.
     */
    Sequence import_sequence(const std::filesystem::path& path);

    /**
     * Return an unsorted list of all valid sequences that are found in the base path.
     *
     * This function quietly ignores folders that do not follow the naming convention for
     * sequences.
     *
     * \returns a vector containing one SequenceOnDisk object for each sequence that was
     *          found. The paths in the returned objects are relative to the base path.
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
     * Load a sequence from an arbitrary folder on disk.
     *
     * This function can be used to load a sequence from a folder that is not managed by
     * the SequenceManager. The machine-friendly name and unique ID of the sequence are
     * parsed from its folder name.
     *
     * \param folder  The sequence folder to be loaded
     *
     * \exception Error is thrown if the sequence cannot be loaded.
     */
    Sequence load_sequence(std::filesystem::path folder) const;

    /**
     * Determine the name and unique ID of a sequence from a folder name, if possible.
     *
     * Taskolib stores sequences in a folder following the scheme "name[uid]". This
     * function extracts the final folder name from the given path and splits it into name
     * and unique ID. If the folder does not follow the naming scheme, nullopt is
     * returned.
     */
    static gul14::optional<SequenceOnDisk>
    parse_folder_name(const std::filesystem::path& folder);

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
     * This function uses git.
     *
     * \param sequence  the sequence to be stored
     * \returns True if the sequence has been stored or false if it was unmodified
     * \exception git::Error is thrown if git could not handle the operation
     */
    bool store_sequence(const Sequence& sequence);

private:
    /// Base path to the sequences.
    std::filesystem::path path_;

    /// Git repository in path_ that holds the sequences
    git::Repository git_repo_;

    /**
     * Create a random unique ID that does not collide with the ID of any sequence in the
     * given sequence list.
     *
     * \exception Error is thrown if no unique ID can be found.
     */
    static UniqueId create_unique_id(const std::vector<SequenceOnDisk>& sequences);

    /// Load a sequence from the specified path, with the given name and unique ID.
    Sequence load_sequence(const SequenceOnDisk& seq_on_disk) const;

    /**
     * Perform changes and commit them to the git repository.
     *
     * This function allows to introduce some changes to the filesystem and create a
     * git commit from the changes; but if something fails the repository is rolled
     * back safely.
     *
     * Committed are just changes in the directories given in dirs and the message starts
     * with the specified message. Both dirs and message can be extended by the actual
     * actions that are performed.
     *
     * - Repository is prepared (reset)
     * - The actions are performed (closure is executed) - they should change the filesystem
     * - Git searches for changes (just in the directories given by dirs)
     * - The changes are added and committed using the given message as title
     * - The commit body contains details about the changed files
     * - If at any point an error occurs the commit is aborted and the filesystem is reset to
     *   where we started (changes are undone)
     *
     * Sometimes we do not know the actual path we want change and commit before the real
     * actions are performed. The closure can return a string; and that is a path to be
     * also considered/added to the commit. The path as string is also added at the end
     * of the title.
     *
     * The signature of the closure can be `void action()` or `std::string action()`.
     *
     * It is not possible to add a path to dirs while keeping the commit title unchanged.
     *
     * All dirs are relative to the git repository root.
     *
     * \param dirs     List of directories to add for this commit (usually just one)
     * \param message  The commit message title
     * \param action   Closure that performs the actual modifications on the filesystem
     * \returns        true if the commit was successful, false if nothing found to commit
     * \exception      task::Error, git::Error, or any other type can be thrown
     *
     */
    template <typename T>
    bool perform_commit(std::string message, T action, std::string extra_dir = "")
    {
        auto commit_body = std::string{ };
        try {
            git_repo_.reset(0);
            if constexpr (std::is_same<decltype(action()), void>::value)
                action();
            else {
                auto path = action();
                commit_body = stage_files_in_directory(path);
                message += path;
            }
            if (not extra_dir.empty())
                commit_body = stage_files_in_directory(extra_dir);
            if (commit_body.empty())
                return false;
            git_repo_.commit(gul14::cat(message, "\n", commit_body));
        }
        catch (std::exception const& e) {
            try {
                git_repo_.reset(0);
            }
            catch (...) {}
            throw e;
        }
        return true;
    }

    /**
     * Stage all changes to files in the given directory for the next git commit.
     *
     * This function is similar to "git add", but it can also stage files for removal like
     * "git rm". It recursively stages all changes to files in the given directory and in
     * its subdirectories.
     *
     * \param directory  Directory whose contents should be staged. Glob/wildcard
     *                   characters like '*' or '?' are considered a literal part of the
     *                   directory name and escaped automatically.
     * \returns a partial commit message containing information about the staged changes.
     *          The returned string starts with a linebreak.
     */
    std::string stage_files_in_directory(const std::string& directory);

    /**
     * Stage files matching the specified glob for the next git commit.
     *
     * This function is similar to "git add", but it can also stage files for removal like
     * "git rm".
     *
     * \param glob  A git glob specifying which files to stage. If it is empty, all files
     *              in the repository are considered. Examples: "", "*.txt",
     *              "foo/backup_[0-9].dat"
     * \returns a partial commit message containing information about the staged changes.
     *          The returned string starts with a linebreak.
     */
    std::string stage_files(const std::string& glob);

    /**
     * Find the sequence with the given unique ID in the given list of sequences.
     * \exception Error is thrown if the sequence cannot be found.
     */
    static SequenceOnDisk find_sequence_on_disk(UniqueId uid,
        const std::vector<SequenceOnDisk>& sequences);

    /// Generate a machine-friendly sequence name from a human-readable label.
    static SequenceName make_sequence_name_from_label(gul14::string_view label);

    /**
     * Actually write a sequence to disk (without commit in git)
     *
     * \param sequence  Sequence object to save
     * \returns         Folder of the sequence on disk (relative to path_)
    */
    std::string write_sequence_to_disk(const Sequence& sequence);
};

} // namespace task

#endif

// vi:ts=4:sw=4:sts=4:et
